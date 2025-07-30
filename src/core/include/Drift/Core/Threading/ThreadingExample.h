#pragma once

#include "ThreadingSystem.h"
#include <vector>
#include <string>

namespace Drift::Core::Threading {

/**
 * @brief Exemplo de uso do sistema de threading
 * 
 * Demonstra como usar o sistema de threading de forma eficiente
 * e profissional em diferentes cenários.
 */
class ThreadingExample {
public:
    static void RunBasicExample();
    static void RunParallelProcessingExample();
    static void RunPriorityExample();
    static void RunProfilingExample();
    static void RunPerformanceTest();
    
private:
    static void SimulateWork(int milliseconds);
    static int CalculateFibonacci(int n);
    static std::vector<int> GenerateRandomData(size_t size);
    static void ProcessDataChunk(const std::vector<int>& data, size_t start, size_t end, std::vector<int>& result);
};

} // namespace Drift::Core::Threading 