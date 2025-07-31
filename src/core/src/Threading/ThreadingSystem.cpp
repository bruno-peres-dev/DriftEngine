#include "Drift/Core/Threading/ThreadingSystem.h"
#include <algorithm>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(__linux__)
#include <pthread.h>
#include <sched.h>
#elif defined(__APPLE__)
#include <pthread.h>
#include <mach/thread_policy.h>
#endif

namespace Drift::Core::Threading {

// Usar namespace std explicitamente para evitar conflitos
using namespace std;

ThreadingSystem& ThreadingSystem::GetInstance() {
    static ThreadingSystem instance;
    return instance;
}

void ThreadingSystem::Initialize(const ThreadingConfig& config) {
    if (m_Initialized.load()) {
        DRIFT_LOG_WARNING("[ThreadingSystem] Sistema já inicializado");
        return;
    }
    
    m_Config = config;
    
    // Auto-detect thread count se não especificado
    if (m_Config.threadCount == 0) {
        unsigned int hw_concurrency = std::thread::hardware_concurrency();
        m_Config.threadCount = (hw_concurrency > 1) ? (hw_concurrency - 1) : 1;
    }
    
    DRIFT_LOG_INFO("[ThreadingSystem] Inicializando com ", m_Config.threadCount, " threads");
    
    m_Initialized = true;
    Start();
}

void ThreadingSystem::Shutdown() {
    if (!m_Initialized.load()) return;
    
    DRIFT_LOG_INFO("[ThreadingSystem] Finalizando sistema...");
    Stop();
    m_Initialized = false;
}

void ThreadingSystem::SetConfig(const ThreadingConfig& config) {
    if (m_Running.load()) {
        DRIFT_LOG_WARNING("[ThreadingSystem] Tentativa de alterar configuração com sistema em execução");
        return;
    }
    
    m_Config = config;
    if (m_Config.threadCount == 0) {
        unsigned int hw_concurrency = std::thread::hardware_concurrency();
        m_Config.threadCount = (hw_concurrency > 1) ? (hw_concurrency - 1) : 1;
    }
}

void ThreadingSystem::Start() {
    if (m_Running.load()) {
        DRIFT_LOG_WARNING("[ThreadingSystem] Sistema já está em execução");
        return;
    }
    
    m_ShouldStop = false;
    m_Running = true;
    m_Paused = false;
    
    // Cria as threads
    m_Threads.clear();
    m_Threads.reserve(m_Config.threadCount);
    
    for (size_t i = 0; i < m_Config.threadCount; ++i) {
        auto threadData = std::make_unique<ThreadData>();
        threadData->threadId = i;
        threadData->shouldStop = false;
        threadData->lastWorkTime = std::chrono::steady_clock::now();
        threadData->stats.threadName = m_Config.threadNamePrefix + "-" + std::to_string(i);
        
        // Cria a thread
        threadData->thread = std::thread(&ThreadingSystem::WorkerThread, this, i);
        
        // Configura affinity se habilitado
        if (m_Config.enableAffinity) {
            SetThreadAffinity(threadData->thread, i);
        }
        
        // Configura nome da thread
        SetThreadName(threadData->thread, threadData->stats.threadName);
        
        m_Threads.push_back(std::move(threadData));
    }
    
    DRIFT_LOG_INFO("[ThreadingSystem] Sistema iniciado com ", m_Config.threadCount, " threads");
}

void ThreadingSystem::Stop() {
    if (!m_Running.load()) return;
    
    DRIFT_LOG_INFO("[ThreadingSystem] Parando sistema...");
    
    m_ShouldStop = true;
    m_Running = false;
    
    // Notifica todas as threads
    m_GlobalCondition.notify_all();
    for (auto& threadData : m_Threads) {
        threadData->condition.notify_all();
    }
    
    // Aguarda todas as threads terminarem
    for (auto& threadData : m_Threads) {
        if (threadData->thread.joinable()) {
            threadData->thread.join();
        }
    }
    
    m_Threads.clear();
    DRIFT_LOG_INFO("[ThreadingSystem] Sistema parado");
}

void ThreadingSystem::Pause() {
    m_Paused = true;
    DRIFT_LOG_INFO("[ThreadingSystem] Sistema pausado");
}

void ThreadingSystem::Resume() {
    m_Paused = false;
    m_GlobalCondition.notify_all();
    for (auto& threadData : m_Threads) {
        threadData->condition.notify_all();
    }
    DRIFT_LOG_INFO("[ThreadingSystem] Sistema resumido");
}

size_t ThreadingSystem::GetQueueSize() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_GlobalQueueMutex));
    return m_GlobalQueue.size();
}

size_t ThreadingSystem::GetActiveThreadCount() const {
    return m_ActiveThreadCount.load();
}

ThreadingSystem::SystemStats ThreadingSystem::GetStats() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_StatsMutex));
    SystemStats stats = m_Stats;
    
    // Adiciona estatísticas das threads
    stats.threadStats.clear();
    stats.threadStats.reserve(m_Threads.size());
    
    for (const auto& threadData : m_Threads) {
        stats.threadStats.push_back(threadData->stats);
    }
    
    // Calcula utilização de CPU
    if (m_Config.threadCount > 0) {
        stats.cpuUtilization = static_cast<double>(m_ActiveThreadCount.load()) / m_Config.threadCount * 100.0;
    }
    
    return stats;
}

void ThreadingSystem::ResetStats() {
    std::lock_guard<std::mutex> lock(m_StatsMutex);
    m_Stats = SystemStats{};
    
    for (auto& threadData : m_Threads) {
        threadData->stats = ThreadStats{};
        threadData->stats.threadName = threadData->stats.threadName; // Preserva o nome
    }
}

void ThreadingSystem::LogStats() const {
    auto stats = GetStats();
    
    DRIFT_LOG_INFO("=== ThreadingSystem Stats ===");
        DRIFT_LOG_INFO("Threads: ", m_Threads.size(), " | Ativas: ", m_ActiveThreadCount.load(), " | Fila: ", m_CurrentQueueSize.load(), " | CPU: ", static_cast<int>(stats.cpuUtilization), "%");
    
        DRIFT_LOG_INFO("Tarefas: ", stats.totalTasksSubmitted, " | Completadas: ", stats.totalTasksCompleted, " | Canceladas: ", stats.totalTasksCancelled);
    
    if (stats.totalTasksCompleted > 0) {
        DRIFT_LOG_INFO("Tempo médio: ", stats.averageTaskTime, "ms");
    }
    
    DRIFT_LOG_INFO("Pico da fila: ", stats.peakQueueSize);
    
    // Estatísticas por thread
    for (size_t i = 0; i < stats.threadStats.size(); ++i) {
        const auto& threadStat = stats.threadStats[i];
                DRIFT_LOG_INFO("Thread ", i, " (", threadStat.threadName, "): ", threadStat.tasksExecuted, " tarefas, ", threadStat.workSteals, " steals, ", threadStat.workStealsReceived, " stolen");
    }
    
    DRIFT_LOG_INFO("=============================");
}

void ThreadingSystem::WaitForAll() {
    while (m_CurrentQueueSize.load() > 0 || m_ActiveThreadCount.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ThreadingSystem::CancelAll() {
    std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
    while (!m_GlobalQueue.empty()) {
        m_GlobalQueue.pop();
    }
    m_CurrentQueueSize = 0;
    m_Stats.totalTasksCancelled += m_Stats.totalTasksSubmitted - m_Stats.totalTasksCompleted;
}

void ThreadingSystem::EnableProfiling(bool enable) {
    m_Config.enableProfiling = enable;
    DRIFT_LOG_INFO("[ThreadingSystem] Profiling ", enable ? "habilitado" : "desabilitado");
}

bool ThreadingSystem::TryStealWork(size_t threadId) {
    if (!m_Config.enableWorkStealing) return false;
    
    // Tenta roubar de outras threads
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        if (i == threadId) continue;
        
        auto& targetThread = m_Threads[i];
        std::lock_guard<std::mutex> lock(targetThread->queueMutex);
        
        if (!targetThread->localQueue.empty()) {
            auto& stolenTask = targetThread->localQueue.front();
            targetThread->localQueue.pop();
            
            // Adiciona à fila global
            {
                std::lock_guard<std::mutex> globalLock(m_GlobalQueueMutex);
                m_GlobalQueue.push(std::move(stolenTask));
            }
            
            targetThread->stats.workStealsReceived++;
            m_Threads[threadId]->stats.workSteals++;
            return true;
        }
    }
    
    return false;
}

void ThreadingSystem::WorkerThread(size_t threadId) {
    auto& threadData = *m_Threads[threadId];
    
    DRIFT_LOG_INFO("[ThreadingSystem] Thread ", threadId, " iniciada");
    
    while (!threadData.shouldStop) {
        Task task;
        bool gotTask = false;
        
        // Tenta pegar tarefa da fila local primeiro
        if (!m_Paused.load()) {
            gotTask = TryGetTask(task, threadData);
        }
        
        // Se não conseguiu, tenta da fila global
        if (!gotTask && !m_Paused.load()) {
            std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
            if (!m_GlobalQueue.empty()) {
                task = std::move(m_GlobalQueue.front());
                m_GlobalQueue.pop();
                m_CurrentQueueSize--;
                gotTask = true;
            }
        }
        
        // Se ainda não conseguiu, tenta work stealing
        if (!gotTask && !m_Paused.load()) {
            gotTask = TryStealWork(threadId);
            if (gotTask) {
                // Recupera a tarefa roubada da fila global
                std::unique_lock<std::mutex> lock(m_GlobalQueueMutex);
                if (!m_GlobalQueue.empty()) {
                    task = std::move(m_GlobalQueue.front());
                    m_GlobalQueue.pop();
                    m_CurrentQueueSize--;
                } else {
                    gotTask = false;
                }
            }
        }
        
        if (gotTask) {
            // Processa a tarefa
            m_ActiveThreadCount++;
            ProcessTask(task, threadData);
            m_ActiveThreadCount--;
        } else {
            // Aguarda por trabalho
            std::unique_lock<std::mutex> lock(threadData.queueMutex);
            threadData.condition.wait_for(lock, std::chrono::milliseconds(10));
        }
    }
    
    DRIFT_LOG_INFO("[ThreadingSystem] Thread ", threadId, " finalizada");
}

void ThreadingSystem::ProcessTask(Task& task, ThreadData& threadData) {
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        // Executa a tarefa
        task.func();
        
        // Atualiza estatísticas
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        threadData.stats.tasksExecuted++;
        threadData.stats.totalWorkTime += duration.count();
        threadData.lastWorkTime = endTime;
        
        std::lock_guard<std::mutex> lock(m_StatsMutex);
        m_Stats.totalTasksCompleted++;
        
        // Atualiza tempo médio
        if (m_Stats.totalTasksCompleted > 0) {
            double totalTime = m_Stats.averageTaskTime * (m_Stats.totalTasksCompleted - 1);
            totalTime += duration.count() / 1000.0; // Converte para ms
            m_Stats.averageTaskTime = totalTime / m_Stats.totalTasksCompleted;
        }
        
        // Log de profiling se habilitado
        if (m_Config.enableProfiling && !task.info.name.empty()) {
            DRIFT_LOG_INFO("[ThreadProfiler] ", task.info.name, ": ", duration.count(), "μs");
        }
        
    } catch (const std::exception& e) {
        DRIFT_LOG_ERROR("[ThreadingSystem] Exceção na thread {}: {}", threadData.threadId, e.what());
    }
}

bool ThreadingSystem::TryGetTask(Task& task, ThreadData& threadData) {
    std::lock_guard<std::mutex> lock(threadData.queueMutex);
    if (!threadData.localQueue.empty()) {
        task = std::move(threadData.localQueue.front());
        threadData.localQueue.pop();
        return true;
    }
    return false;
}

void ThreadingSystem::SetThreadAffinity(std::thread& thread, size_t cpuId) {
#ifdef _WIN32
    SetThreadAffinityMask(thread.native_handle(), (1ULL << cpuId));
#elif defined(__linux__)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuId, &cpuset);
    pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
    // macOS não suporta affinity direto
    (void)thread;
    (void)cpuId;
#endif
}

void ThreadingSystem::SetThreadName(std::thread& thread, const std::string& name) {
#ifdef _WIN32
    SetThreadDescription(thread.native_handle(), std::wstring(name.begin(), name.end()).c_str());
#elif defined(__linux__)
    pthread_setname_np(thread.native_handle(), name.c_str());
#elif defined(__APPLE__)
    // macOS não suporta naming direto
    (void)thread;
    (void)name;
#endif
}

} // namespace Drift::Core::Threading 