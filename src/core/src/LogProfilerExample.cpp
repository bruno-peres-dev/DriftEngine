#include "Drift/Core/Log.h"
#include "Drift/Core/Profiler.h"
#include <thread>
#include <chrono>
#include <vector>

using namespace Drift::Core;

void DemonstrateLogging() {
    DRIFT_LOG_INFO("=== Demonstração do Sistema de Log Profissional ===");
    
    // Configuração do sistema de log
    LogConfig logConfig;
    logConfig.minLevel = LogLevel::Debug;
    logConfig.enableTimestamps = true;
    logConfig.enableThreadInfo = true;
    logConfig.enableFileInfo = true;
    logConfig.outputFile = "drift_engine.log";
    
    g_LogSystem.Configure(logConfig);
    
    // Demonstração de diferentes níveis de log
    DRIFT_LOG_TRACE("Mensagem de trace - muito detalhada");
    DRIFT_LOG_DEBUG("Mensagem de debug - para desenvolvimento");
    DRIFT_LOG_INFO("Mensagem de informação - status normal");
    DRIFT_LOG_WARNING("Mensagem de aviso - algo pode estar errado");
    DRIFT_LOG_ERROR("Mensagem de erro - algo deu errado");
    DRIFT_LOG_FATAL("Mensagem fatal - erro crítico");
    
    // Demonstração de formatação
    int valor = 42;
    float pi = 3.14159f;
    std::string texto = "exemplo";
    
    DRIFT_LOG_INFO("Valor inteiro: {}, Float: {:.2f}, Texto: {}", valor, pi, texto);
    
    // Demonstração de logging condicional
    bool debugMode = true;
    LOG_DEBUG_IF(debugMode, "Debug mode está ativado");
    LOG_INFO_IF(!debugMode, "Debug mode está desativado");
    
    // Demonstração de logging de performance
    LOG_PERF("Iniciando operação crítica");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    LOG_PERF("Operação crítica concluída");
    
    // Demonstração de logging de memória
    LOG_MEM("Alocando 1024 bytes");
    std::vector<int> data(256);
    LOG_MEM("Vetor alocado com {} elementos", data.size());
    
    // Demonstração de logging RHI
    LogRHI("Inicializando contexto DirectX 11");
    LogRHIDebug("Shader compilado com sucesso");
    LogRHIError("Falha ao criar buffer de vértices");
    
    // Demonstração de logging de exceções
    try {
        throw std::runtime_error("Erro de exemplo");
    } catch (const std::exception& e) {
        LogException("Demonstração", e);
    }
    
    // Demonstração de logging de HRESULT
    LogHRESULT("Criação de dispositivo", S_OK);
    LogHRESULT("Criação de buffer", E_INVALIDARG);
    
    DRIFT_LOG_INFO("=== Demonstração de Log Concluída ===");
}

void DemonstrateProfiling() {
    DRIFT_LOG_INFO("=== Demonstração do Sistema de Profiler Profissional ===");
    
    // Configuração do profiler
    ProfilerConfig profilerConfig;
    profilerConfig.enableProfiling = true;
    profilerConfig.enableThreadProfiling = true;
    profilerConfig.enableMemoryProfiling = true;
    profilerConfig.maxSections = 1000;
    profilerConfig.maxDepth = 32;
    profilerConfig.outputFile = "drift_profiler.txt";
    
    Profiler::GetInstance().Configure(profilerConfig);
    
    // Demonstração básica de profiling
    {
        PROFILE_SCOPE("Operação Simples");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Demonstração de profiling de função
    {
        PROFILE_FUNCTION();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    
    // Demonstração de profiling hierárquico
    {
        PROFILE_SCOPE("Operação Principal");
        
        {
            PROFILE_SCOPE_WITH_PARENT("Sub-operação 1", "Operação Principal");
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            {
                PROFILE_SCOPE_WITH_PARENT("Sub-sub-operação", "Sub-operação 1");
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        
        {
            PROFILE_SCOPE_WITH_PARENT("Sub-operação 2", "Operação Principal");
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    }
    
    // Demonstração de profiling condicional
    bool enableDetailedProfiling = true;
    {
        PROFILE_SCOPE_IF(enableDetailedProfiling, "Profiling Detalhado");
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
    
    // Demonstração de profiling específico
    {
        PROFILE_PERF("Teste de Performance");
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    
    {
        PROFILE_RENDER("Renderização de Frame");
        std::this_thread::sleep_for(std::chrono::milliseconds(35));
    }
    
    {
        PROFILE_UPDATE("Atualização de Lógica");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    {
        PROFILE_LOAD("Carregamento de Asset");
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
    
    // Demonstração de profiling de memória
    {
        PROFILE_SCOPE("Operação com Memória");
        
        PROFILE_MEMORY_ALLOC(1024);
        std::vector<int> tempData(256);
        
        PROFILE_MEMORY_ALLOC(2048);
        std::vector<float> moreData(512);
        
        PROFILE_MEMORY_DEALLOC(1024);
        tempData.clear();
        tempData.shrink_to_fit();
    }
    
    // Demonstração de profiling em loop
    {
        PROFILE_SCOPE("Loop de Teste");
        
        for (int i = 0; i < 10; ++i) {
            PROFILE_SCOPE("Iteração do Loop");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    
    // Demonstração de profiling multi-thread
    {
        PROFILE_SCOPE("Operação Multi-thread");
        
        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([i]() {
                PROFILE_SCOPE("Thread " + std::to_string(i));
                std::this_thread::sleep_for(std::chrono::milliseconds(20 + i * 5));
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    // Gerar relatório
    Profiler::GetInstance().PrintReport();
    
    // Exportar relatório para arquivo
    Profiler::GetInstance().ExportReport("profiler_report.txt");
    
    // Obter estatísticas específicas
    auto stats = Profiler::GetInstance().GetSectionStats("Operação Principal");
    DRIFT_LOG_INFO("Estatísticas da 'Operação Principal':");
    DRIFT_LOG_INFO("  Chamadas: {}", stats.callCount);
    DRIFT_LOG_INFO("  Tempo médio: {:.3f}ms", stats.GetAverageTimeMs());
    DRIFT_LOG_INFO("  Tempo total: {:.3f}ms", stats.GetTotalTimeMs());
    DRIFT_LOG_INFO("  Tempo mínimo: {:.3f}ms", stats.GetMinTimeMs());
    DRIFT_LOG_INFO("  Tempo máximo: {:.3f}ms", stats.GetMaxTimeMs());
    DRIFT_LOG_INFO("  Desvio padrão: {:.3f}ms", stats.GetStandardDeviationMs());
    DRIFT_LOG_INFO("  Profundidade: {}", stats.depth);
    
    // Obter todas as estatísticas
    auto allStats = Profiler::GetInstance().GetAllStats();
    DRIFT_LOG_INFO("Total de seções registradas: {}", allStats.size());
    
    // Limpar dados do profiler
    Profiler::GetInstance().Clear();
    
    DRIFT_LOG_INFO("=== Demonstração de Profiler Concluída ===");
}

void DemonstrateAdvancedFeatures() {
    DRIFT_LOG_INFO("=== Demonstração de Recursos Avançados ===");
    
    // Configuração avançada do log
    LogConfig advancedLogConfig;
    advancedLogConfig.minLevel = LogLevel::Trace;
    advancedLogConfig.enableTimestamps = true;
    advancedLogConfig.enableThreadInfo = true;
    advancedLogConfig.enableFileInfo = true;
    advancedLogConfig.outputFile = "advanced_log.txt";
    
    // Output customizado
    advancedLogConfig.customOutput = [](LogLevel level, const std::string& message) {
        // Aqui você pode implementar output customizado, como:
        // - Envio para servidor remoto
        // - Interface gráfica
        // - Sistema de alertas
        DRIFT_LOG_DEBUG("[CUSTOM] {}", message);
    };
    
    g_LogSystem.Configure(advancedLogConfig);
    
    // Configuração avançada do profiler
    ProfilerConfig advancedProfilerConfig;
    advancedProfilerConfig.enableProfiling = true;
    advancedProfilerConfig.enableThreadProfiling = true;
    advancedProfilerConfig.enableMemoryProfiling = true;
    advancedProfilerConfig.enableCallStack = true;
    advancedProfilerConfig.maxSections = 5000;
    advancedProfilerConfig.maxDepth = 64;
    advancedProfilerConfig.outputFile = "advanced_profiler.txt";
    
    // Output customizado para profiler
    advancedProfilerConfig.customOutput = [](const std::string& report) {
        DRIFT_LOG_DEBUG("[PROFILER] {}", report);
    };
    
    Profiler::GetInstance().Configure(advancedProfilerConfig);
    
    // Demonstração de profiling complexo
    {
        PROFILE_SCOPE("Sistema Completo");
        
        // Simular inicialização de sistemas
        {
            PROFILE_SCOPE_WITH_PARENT("Inicialização", "Sistema Completo");
            
            {
                PROFILE_SCOPE_WITH_PARENT("RHI", "Inicialização");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                LOG_PERF("RHI inicializado");
            }
            
            {
                PROFILE_SCOPE_WITH_PARENT("Audio", "Inicialização");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                LOG_PERF("Audio inicializado");
            }
            
            {
                PROFILE_SCOPE_WITH_PARENT("Input", "Inicialização");
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                LOG_PERF("Input inicializado");
            }
        }
        
        // Simular loop principal
        {
            PROFILE_SCOPE_WITH_PARENT("Loop Principal", "Sistema Completo");
            
            for (int frame = 0; frame < 5; ++frame) {
                PROFILE_SCOPE("Frame " + std::to_string(frame));
                
                {
                    PROFILE_SCOPE_WITH_PARENT("Update", "Frame " + std::to_string(frame));
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                {
                    PROFILE_SCOPE_WITH_PARENT("Render", "Frame " + std::to_string(frame));
                    std::this_thread::sleep_for(std::chrono::milliseconds(15));
                }
                
                {
                    PROFILE_SCOPE_WITH_PARENT("Audio", "Frame " + std::to_string(frame));
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            }
        }
    }
    
    // Demonstração de profiling de memória avançado
    {
        PROFILE_SCOPE("Gerenciamento de Memória");
        
        std::vector<std::vector<int>> dataStructures;
        
        for (int i = 0; i < 10; ++i) {
            PROFILE_SCOPE("Alocação " + std::to_string(i));
            
            size_t size = (i + 1) * 1024;
            PROFILE_MEMORY_ALLOC(size);
            
            dataStructures.emplace_back(size / sizeof(int));
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        for (int i = 9; i >= 0; --i) {
            PROFILE_SCOPE("Desalocação " + std::to_string(i));
            
            size_t size = (i + 1) * 1024;
            PROFILE_MEMORY_DEALLOC(size);
            
            dataStructures.pop_back();
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    
    // Gerar relatório final
    Profiler::GetInstance().PrintReport();
    
    DRIFT_LOG_INFO("=== Demonstração de Recursos Avançados Concluída ===");
}

int main() {
    DRIFT_LOG_INFO("Iniciando demonstração do sistema de Log e Profiler profissional");
    
    DemonstrateLogging();
    DemonstrateProfiling();
    DemonstrateAdvancedFeatures();
    
    DRIFT_LOG_INFO("Demonstração concluída com sucesso!");
    
    return 0;
} 