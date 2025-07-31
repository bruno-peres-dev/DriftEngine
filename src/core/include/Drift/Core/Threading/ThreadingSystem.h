#pragma once

#include "Drift/Core/Log.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>

namespace Drift::Core::Threading {

/**
 * @brief Prioridade de tarefas
 */
enum class TaskPriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

/**
 * @brief Configuração do sistema de threading
 */
struct ThreadingConfig {
    size_t threadCount = 0;                    // 0 = auto-detect (cores - 1)
    size_t maxQueueSize = 10000;               // Tamanho máximo da fila de tarefas
    bool enableWorkStealing = true;            // Habilita work stealing entre threads
    bool enableAffinity = true;                // Habilita CPU affinity
    std::string threadNamePrefix = "Drift";    // Prefixo para nomes das threads
    size_t spinCount = 1000;                   // Número de spins antes de dormir
    bool enableProfiling = false;              // Habilita profiling de tarefas
};

/**
 * @brief Informações sobre uma tarefa
 */
struct TaskInfo {
    std::string name;
    TaskPriority priority = TaskPriority::Normal;
    size_t estimatedWork = 1;  // Estimativa de trabalho (para balanceamento)
    bool isBlocking = false;   // Se a tarefa pode bloquear
    std::chrono::steady_clock::time_point submitTime;
};

/**
 * @brief Futuro com informações adicionais
 */
template<typename T>
class TaskFuture {
public:
    TaskFuture(std::future<T>&& future, const TaskInfo& info) 
        : m_Future(std::move(future)), m_Info(info) {}
    
    // Aguarda conclusão e retorna resultado
    T Get() { return m_Future.get(); }
    
    // Verifica se está pronto
    bool IsReady() const { 
        return m_Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready; 
    }
    
    // Aguarda com timeout
    template<typename Rep, typename Period>
    bool WaitFor(const std::chrono::duration<Rep, Period>& timeout) {
        return m_Future.wait_for(timeout) == std::future_status::ready;
    }
    
    // Informações da tarefa
    const TaskInfo& GetTaskInfo() const { return m_Info; }
    
    // Tempo de execução
    std::chrono::microseconds GetExecutionTime() const {
        if (m_Info.submitTime.time_since_epoch().count() == 0) {
            return std::chrono::microseconds(0);
        }
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now - m_Info.submitTime);
    }

private:
    std::future<T> m_Future;
    TaskInfo m_Info;
};

/**
 * @brief Sistema de threading unificado e otimizado
 * 
 * Características:
 * - ThreadPool com work stealing
 * - Prioridades de tarefas
 * - CPU affinity
 * - Profiling integrado
 * - Estatísticas detalhadas
 * - Interface simples e intuitiva
 * - Otimizado para performance
 */
class ThreadingSystem {
public:
    static ThreadingSystem& GetInstance();
    
    // Inicialização e configuração
    void Initialize(const ThreadingConfig& config = {});
    void Shutdown();
    
    // Submissão de tarefas
    template<typename F, typename... Args>
    auto Submit(F&& f, Args&&... args) -> TaskFuture<std::invoke_result_t<F, Args...>>;
    
    template<typename F, typename... Args>
    auto SubmitWithPriority(TaskPriority priority, F&& f, Args&&... args) 
        -> TaskFuture<std::invoke_result_t<F, Args...>>;
    
    template<typename F, typename... Args>
    auto SubmitWithInfo(const TaskInfo& info, F&& f, Args&&... args) 
        -> TaskFuture<std::invoke_result_t<F, Args...>>;
    
    // Controle do sistema
    void Start();
    void Stop();
    void Pause();
    void Resume();
    
    // Status e estatísticas
    bool IsInitialized() const { return m_Initialized; }
    bool IsRunning() const { return m_Running.load(); }
    bool IsPaused() const { return m_Paused.load(); }
    size_t GetThreadCount() const { return m_Threads.size(); }
    size_t GetQueueSize() const;
    size_t GetActiveThreadCount() const;
    
    // Estatísticas detalhadas
    struct ThreadStats {
        size_t tasksExecuted = 0;
        size_t totalWorkTime = 0;
        size_t idleTime = 0;
        size_t workSteals = 0;
        size_t workStealsReceived = 0;
        std::string threadName;
    };
    
    struct SystemStats {
        size_t totalTasksSubmitted = 0;
        size_t totalTasksCompleted = 0;
        size_t totalTasksCancelled = 0;
        size_t averageQueueSize = 0;
        size_t peakQueueSize = 0;
        double averageTaskTime = 0.0;
        double cpuUtilization = 0.0;
        std::vector<ThreadStats> threadStats;
    };
    
    SystemStats GetStats() const;
    void ResetStats();
    void LogStats() const;
    
    // Utilitários
    void WaitForAll();
    void CancelAll();
    
    // Profiling
    void EnableProfiling(bool enable);
    bool IsProfilingEnabled() const { return m_Config.enableProfiling; }
    
    // Configuração
    const ThreadingConfig& GetConfig() const { return m_Config; }
    void SetConfig(const ThreadingConfig& config);

private:
    ThreadingSystem() = default;
    ~ThreadingSystem() = default;
    
    struct Task {
        std::function<void()> func;
        TaskInfo info;
        size_t submitThreadId;
    };
    
    struct ThreadData {
        std::thread thread;
        std::queue<Task> localQueue;
        std::mutex queueMutex;
        std::condition_variable condition;
        ThreadStats stats;
        size_t threadId;
        bool shouldStop = false;
        std::chrono::steady_clock::time_point lastWorkTime;
    };
    
    // Métodos internos
    void WorkerThread(size_t threadId);
    void ProcessTask(Task& task, ThreadData& threadData);
    bool TryGetTask(Task& task, ThreadData& threadData);
    bool TryStealWork(size_t threadId);
    void SetThreadAffinity(std::thread& thread, size_t cpuId);
    void SetThreadName(std::thread& thread, const std::string& name);
    
    // Configuração e estado
    ThreadingConfig m_Config;
    std::atomic<bool> m_Initialized{false};
    std::atomic<bool> m_Running{false};
    std::atomic<bool> m_Paused{false};
    std::atomic<bool> m_ShouldStop{false};
    
    // Threads e filas
    std::vector<std::unique_ptr<ThreadData>> m_Threads;
    std::queue<Task> m_GlobalQueue;
    std::mutex m_GlobalQueueMutex;
    std::condition_variable m_GlobalCondition;
    
    // Estatísticas
    mutable std::mutex m_StatsMutex;
    SystemStats m_Stats;
    std::atomic<size_t> m_ActiveThreadCount{0};
    std::atomic<size_t> m_CurrentQueueSize{0};
    std::atomic<size_t> m_PeakQueueSize{0};
    
    // Work stealing
    std::atomic<size_t> m_NextStealTarget{0};
};

// Implementação dos templates
template<typename F, typename... Args>
auto ThreadingSystem::Submit(F&& f, Args&&... args) -> TaskFuture<std::invoke_result_t<F, Args...>> {
    return SubmitWithInfo(TaskInfo{}, std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename F, typename... Args>
auto ThreadingSystem::SubmitWithPriority(TaskPriority priority, F&& f, Args&&... args) 
    -> TaskFuture<std::invoke_result_t<F, Args...>> {
    TaskInfo info;
    info.priority = priority;
    return SubmitWithInfo(info, std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename F, typename... Args>
auto ThreadingSystem::SubmitWithInfo(const TaskInfo& info, F&& f, Args&&... args) 
    -> TaskFuture<std::invoke_result_t<F, Args...>> {
    
    using ReturnType = std::invoke_result_t<F, Args...>;
    
    // Cria um packaged_task para capturar o resultado
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    // Cria o future
    auto future = task->get_future();
    
    // Cria a tarefa para o sistema
    Task systemTask;
    systemTask.func = [task]() { (*task)(); };
    systemTask.info = info;
    systemTask.info.submitTime = std::chrono::steady_clock::now();
    systemTask.submitThreadId = std::hash<std::thread::id>{}(std::this_thread::get_id()) % m_Threads.size();
    
    // Adiciona à fila global
    {
        std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
        m_GlobalQueue.push(std::move(systemTask));
        m_CurrentQueueSize++;
        m_PeakQueueSize = std::max(m_PeakQueueSize.load(), m_CurrentQueueSize.load());
        m_Stats.totalTasksSubmitted++;
    }
    
    // Notifica uma thread
    m_GlobalCondition.notify_one();
    
    return TaskFuture<ReturnType>(std::move(future), info);
}

// Macros para facilitar o uso
#define DRIFT_THREADING() Drift::Core::Threading::ThreadingSystem::GetInstance()

// Macros para tarefas assíncronas
#define DRIFT_ASYNC(func) \
    DRIFT_THREADING().Submit(func)

#define DRIFT_ASYNC_PRIORITY(func, priority) \
    DRIFT_THREADING().SubmitWithPriority(priority, func)

// Macro simplificada para tarefas nomeadas (use SubmitWithInfo diretamente para melhor compatibilidade)
#define DRIFT_ASYNC_NAMED(func, name) \
    DRIFT_THREADING().SubmitWithInfo(Drift::Core::Threading::TaskInfo{name}, func)

// Macros para profiling
#define DRIFT_PROFILE_THREAD_SCOPE(name) \
    auto startTime = std::chrono::steady_clock::now(); \
    auto endTime = startTime; \
    auto profiler = [&]() { \
        endTime = std::chrono::steady_clock::now(); \
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime); \
        if (Drift::Core::Threading::ThreadingSystem::GetInstance().IsProfilingEnabled()) { \
            LOG_INFO("[ThreadProfiler] {}: {}μs", std::string(name), duration.count()); \
        } \
    }; \
    std::unique_ptr<void, decltype(profiler)> profilerGuard(nullptr, profiler)

// Macros para sincronização
#define DRIFT_WAIT_FOR_ALL() Drift::Core::Threading::ThreadingSystem::GetInstance().WaitForAll()

} // namespace Drift::Core::Threading
