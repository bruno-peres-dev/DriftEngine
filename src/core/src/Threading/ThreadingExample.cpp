#include "Drift/Core/Threading/ThreadingExample.h"
#include "Drift/Core/Log.h"
#include <random>
#include <algorithm>
#include <chrono>

namespace Drift::Core::Threading {

void ThreadingExample::RunBasicExample() {
    Core::Log("[ThreadingExample] Iniciando exemplo básico...");
    
    // Inicializa o sistema
    auto& threadingSystem = ThreadingSystem::GetInstance();
    threadingSystem.Initialize();
    
    // Exemplo 1: Tarefas simples
    std::vector<TaskFuture<int>> futures;
    
    // Submete várias tarefas
    for (int i = 0; i < 10; ++i) {
        auto future = DRIFT_ASYNC([i]() {
            SimulateWork(100);
            return i * i;
        });
        futures.push_back(std::move(future));
    }
    
    // Aguarda resultados
    for (size_t i = 0; i < futures.size(); ++i) {
        int result = futures[i].Get();
        Core::Log("[ThreadingExample] Tarefa " + std::to_string(i) + " = " + std::to_string(result));
    }
    
    // Exemplo 2: Tarefas com prioridade
    auto highPriorityFuture = DRIFT_ASYNC_PRIORITY([]() {
        SimulateWork(50);
        return std::string("Tarefa de alta prioridade");
    }, TaskPriority::High);
    
    auto lowPriorityFuture = DRIFT_ASYNC_PRIORITY([]() {
        SimulateWork(200);
        return std::string("Tarefa de baixa prioridade");
    }, TaskPriority::Low);
    
    // Aguarda todas as tarefas
    DRIFT_WAIT_FOR_ALL();
    
    Core::Log("[ThreadingExample] Resultado alta prioridade: " + highPriorityFuture.Get());
    Core::Log("[ThreadingExample] Resultado baixa prioridade: " + lowPriorityFuture.Get());
    
    // Estatísticas
    threadingSystem.LogStats();
    
    Core::Log("[ThreadingExample] Exemplo básico concluído!");
}

void ThreadingExample::RunParallelProcessingExample() {
    Core::Log("[ThreadingExample] Iniciando exemplo de processamento paralelo...");
    
    auto& threadingSystem = ThreadingSystem::GetInstance();
    threadingSystem.Initialize();
    
    // Gera dados para processar
    const size_t dataSize = 1000000;
    auto data = GenerateRandomData(dataSize);
    std::vector<int> result(dataSize);
    
    const size_t chunkSize = dataSize / 8; // Divide em 8 chunks
    std::vector<TaskFuture<void>> futures;
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Processa chunks em paralelo
    for (size_t i = 0; i < 8; ++i) {
        size_t start = i * chunkSize;
        size_t end = (i == 7) ? dataSize : (i + 1) * chunkSize;
        
        // Cria a info da tarefa
        auto info = Drift::Core::Threading::TaskInfo{};
        info.name = "ProcessChunk_" + std::to_string(i);
        
        // Submete a tarefa
        auto future = threadingSystem.SubmitWithInfo(info, [&data, &result, start, end]() {
            ProcessDataChunk(data, start, end, result);
        });
        
        futures.push_back(std::move(future));
    }
    
    // Aguarda todos os chunks serem processados
    for (auto& future : futures) {
        future.Get();
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    Core::Log("[ThreadingExample] Processamento paralelo concluído em " + 
              std::to_string(duration.count()) + "ms");
    
    // Verifica resultado
    int sum = 0;
    for (int value : result) {
        sum += value;
    }
    Core::Log("[ThreadingExample] Soma total: " + std::to_string(sum));
    
    threadingSystem.LogStats();
    Core::Log("[ThreadingExample] Exemplo de processamento paralelo concluído!");
}

void ThreadingExample::RunPriorityExample() {
    Core::Log("[ThreadingExample] Iniciando exemplo de prioridades...");
    
    auto& threadingSystem = ThreadingSystem::GetInstance();
    threadingSystem.Initialize();
    
    // Tarefas com diferentes prioridades
    auto criticalFuture = DRIFT_ASYNC_PRIORITY([]() {
        SimulateWork(100);
        return std::string("CRÍTICA");
    }, TaskPriority::Critical);
    
    auto highFuture = DRIFT_ASYNC_PRIORITY([]() {
        SimulateWork(100);
        return std::string("ALTA");
    }, TaskPriority::High);
    
    auto normalFuture = DRIFT_ASYNC_PRIORITY([]() {
        SimulateWork(100);
        return std::string("NORMAL");
    }, TaskPriority::Normal);
    
    auto lowFuture = DRIFT_ASYNC_PRIORITY([]() {
        SimulateWork(100);
        return std::string("BAIXA");
    }, TaskPriority::Low);
    
    // Aguarda todas
    DRIFT_WAIT_FOR_ALL();
    
    Core::Log("[ThreadingExample] Resultados por prioridade:");
    Core::Log("[ThreadingExample] - Crítica: " + criticalFuture.Get());
    Core::Log("[ThreadingExample] - Alta: " + highFuture.Get());
    Core::Log("[ThreadingExample] - Normal: " + normalFuture.Get());
    Core::Log("[ThreadingExample] - Baixa: " + lowFuture.Get());
    
    threadingSystem.LogStats();
    Core::Log("[ThreadingExample] Exemplo de prioridades concluído!");
}

void ThreadingExample::RunProfilingExample() {
    Core::Log("[ThreadingExample] Iniciando exemplo de profiling...");
    
    auto& threadingSystem = ThreadingSystem::GetInstance();
    threadingSystem.Initialize();
    threadingSystem.EnableProfiling(true);
    
    // Tarefas com profiling
    auto info1 = Drift::Core::Threading::TaskInfo{};
    info1.name = "Fibonacci_30";
    auto future1 = threadingSystem.SubmitWithInfo(info1, []() {
        SimulateWork(50);
        return CalculateFibonacci(30);
    });
    
    auto info2 = Drift::Core::Threading::TaskInfo{};
    info2.name = "Fibonacci_35";
    auto future2 = threadingSystem.SubmitWithInfo(info2, []() {
        SimulateWork(100);
        return CalculateFibonacci(35);
    });
    
    auto info3 = Drift::Core::Threading::TaskInfo{};
    info3.name = "Fibonacci_40";
    auto future3 = threadingSystem.SubmitWithInfo(info3, []() {
        SimulateWork(200);
        return CalculateFibonacci(40);
    });
    
    // Aguarda todas
    DRIFT_WAIT_FOR_ALL();
    
    Core::Log("[ThreadingExample] Resultados:");
    Core::Log("[ThreadingExample] - Fib(30): " + std::to_string(future1.Get()));
    Core::Log("[ThreadingExample] - Fib(35): " + std::to_string(future2.Get()));
    Core::Log("[ThreadingExample] - Fib(40): " + std::to_string(future3.Get()));
    
    threadingSystem.LogStats();
    Core::Log("[ThreadingExample] Exemplo de profiling concluído!");
}

void ThreadingExample::RunPerformanceTest() {
    Core::Log("[ThreadingExample] Iniciando teste de performance...");
    
    auto& threadingSystem = ThreadingSystem::GetInstance();
    threadingSystem.Initialize();
    
    const int numTasks = 10000;
    std::vector<TaskFuture<int>> futures;
    futures.reserve(numTasks);
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Submete muitas tarefas pequenas
    for (int i = 0; i < numTasks; ++i) {
        auto future = DRIFT_ASYNC([i]() {
            // Simula trabalho computacional
            int result = 0;
            for (int j = 0; j < 100; ++j) {
                result += i * j;
            }
            return result;
        });
        futures.push_back(std::move(future));
    }
    
    // Aguarda todos
    for (auto& future : futures) {
        future.Get();
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    Core::Log("[ThreadingExample] Performance: " + std::to_string(numTasks) + 
              " tarefas em " + std::to_string(duration.count()) + "ms");
    Core::Log("[ThreadingExample] Taxa: " + std::to_string(numTasks * 1000 / duration.count()) + 
              " tarefas/segundo");
    
    threadingSystem.LogStats();
    Core::Log("[ThreadingExample] Teste de performance concluído!");
}

void ThreadingExample::SimulateWork(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

int ThreadingExample::CalculateFibonacci(int n) {
    if (n <= 1) return n;
    
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

std::vector<int> ThreadingExample::GenerateRandomData(size_t size) {
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    for (size_t i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    
    return data;
}

void ThreadingExample::ProcessDataChunk(const std::vector<int>& data, size_t start, size_t end, std::vector<int>& result) {
    for (size_t i = start; i < end; ++i) {
        // Simula processamento complexo
        result[i] = data[i] * data[i] + data[i];
        
        // Pequena pausa para simular trabalho real
        if (i % 1000 == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
}

} // namespace Drift::Core::Threading 