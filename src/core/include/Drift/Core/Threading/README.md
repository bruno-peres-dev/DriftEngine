# Sistema de Threading - DriftEngine

## Visão Geral

O sistema de threading do DriftEngine foi completamente refeito para ser **simples**, **otimizado**, **profissional** e **utilizável em qualquer subsistema**. É um sistema unificado que combina as melhores práticas de threading moderno.

## Características Principais

### ✅ **Simplicidade**
- Interface intuitiva e fácil de usar
- Macros para facilitar o uso comum
- Auto-configuração baseada no hardware
- Menos complexidade, mais produtividade

### ✅ **Otimização**
- Work stealing entre threads
- CPU affinity para melhor performance
- Filas locais e globais otimizadas
- Balanceamento de carga inteligente

### ✅ **Profissionalismo**
- Código limpo e bem documentado
- Tratamento de exceções robusto
- Estatísticas detalhadas
- Profiling integrado

### ✅ **Utilizável em Qualquer Subsistema**
- Interface unificada
- Configuração flexível
- Integração com todos os módulos
- Sem dependências complexas

## Uso Básico

### Inicialização

```cpp
#include "Drift/Core/Threading/ThreadingSystem.h"

// Inicializa o sistema (auto-detect de threads)
auto& threading = Drift::Core::Threading::ThreadingSystem::GetInstance();
threading.Initialize();

// Ou com configuração customizada
Drift::Core::Threading::ThreadingConfig config;
config.threadCount = 8;
config.enableProfiling = true;
threading.Initialize(config);
```

### Tarefas Simples

```cpp
// Tarefa básica
auto future = DRIFT_ASYNC([]() {
    return 42;
});

int result = future.Get();
```

### Tarefas com Prioridade

```cpp
// Tarefa de alta prioridade
auto highPriorityFuture = DRIFT_ASYNC_PRIORITY([]() {
    return "Urgente!";
}, Drift::Core::Threading::TaskPriority::High);

// Tarefa crítica
auto criticalFuture = DRIFT_ASYNC_PRIORITY([]() {
    return "Crítica!";
}, Drift::Core::Threading::TaskPriority::Critical);
```

### Tarefas Nomeadas (para Profiling)

```cpp
// Método 1: Usando a macro (para tarefas simples)
auto namedFuture = DRIFT_ASYNC_NAMED([]() {
    // Trabalho pesado aqui
    return "Resultado";
}, "ProcessamentoPesado");

// Método 2: Usando SubmitWithInfo diretamente (recomendado para lambdas complexas)
auto info = Drift::Core::Threading::TaskInfo{};
info.name = "ProcessamentoPesado";
auto future = threadingSystem.SubmitWithInfo(info, []() {
    // Trabalho pesado aqui
    return "Resultado";
});
```

### Sincronização

```cpp
// Aguarda todas as tarefas terminarem
DRIFT_WAIT_FOR_ALL();

// Ou aguarda uma tarefa específica
auto future = DRIFT_ASYNC([]() { /* trabalho */ });
future.Get(); // Bloqueia até terminar
```

## Exemplos Práticos

### Processamento Paralelo de Dados

```cpp
std::vector<int> data = GenerateLargeDataset();
std::vector<int> result(data.size());

const size_t chunkSize = data.size() / 8;
std::vector<TaskFuture<void>> futures;

// Divide o trabalho em chunks
for (size_t i = 0; i < 8; ++i) {
    size_t start = i * chunkSize;
    size_t end = (i == 7) ? data.size() : (i + 1) * chunkSize;
    
    // Cria info da tarefa
    auto info = Drift::Core::Threading::TaskInfo{};
    info.name = "Chunk_" + std::to_string(i);
    
    // Submete a tarefa
    auto future = threadingSystem.SubmitWithInfo(info, [&data, &result, start, end]() {
        ProcessChunk(data, result, start, end);
    });
    
    futures.push_back(std::move(future));
}

// Aguarda todos terminarem
DRIFT_WAIT_FOR_ALL();
```

### Carregamento de Assets

```cpp
// Carrega múltiplos assets em paralelo
std::vector<std::string> assetPaths = {"texture1.png", "texture2.png", "model.obj"};

for (const auto& path : assetPaths) {
    auto info = Drift::Core::Threading::TaskInfo{};
    info.name = "LoadAsset_" + path;
    threadingSystem.SubmitWithInfo(info, [path]() {
        LoadAsset(path);
    });
}

DRIFT_WAIT_FOR_ALL();
```

### Renderização

```cpp
// Renderiza diferentes passos em paralelo
auto geometryFuture = DRIFT_ASYNC_PRIORITY([]() {
    RenderGeometry();
}, TaskPriority::High);

auto lightingFuture = DRIFT_ASYNC_PRIORITY([]() {
    RenderLighting();
}, TaskPriority::Normal);

auto postProcessFuture = DRIFT_ASYNC_PRIORITY([]() {
    ApplyPostProcessing();
}, TaskPriority::Low);

DRIFT_WAIT_FOR_ALL();
```

## Configuração

### ThreadingConfig

```cpp
struct ThreadingConfig {
    size_t threadCount = 0;                    // 0 = auto-detect
    size_t maxQueueSize = 10000;               // Tamanho máximo da fila
    bool enableWorkStealing = true;            // Work stealing
    bool enableAffinity = true;                // CPU affinity
    std::string threadNamePrefix = "Drift";    // Prefixo dos nomes
    size_t spinCount = 1000;                   // Spins antes de dormir
    bool enableProfiling = false;              // Profiling
};
```

### Configurações Recomendadas por Cenário

```cpp
// Para jogos (performance)
ThreadingConfig gameConfig;
gameConfig.threadCount = std::thread::hardware_concurrency() - 1;
gameConfig.enableWorkStealing = true;
gameConfig.enableAffinity = true;
gameConfig.enableProfiling = false;

// Para desenvolvimento (debug)
ThreadingConfig devConfig;
devConfig.threadCount = 4;
devConfig.enableProfiling = true;
devConfig.threadNamePrefix = "DriftDev";
```

## Estatísticas e Profiling

### Estatísticas do Sistema

```cpp
auto stats = threading.GetStats();
Core::Log("Threads ativas: " + std::to_string(stats.cpuUtilization) + "%");
Core::Log("Tarefas completadas: " + std::to_string(stats.totalTasksCompleted));

// Log completo das estatísticas
threading.LogStats();
```

### Profiling de Tarefas

```cpp
// Habilita profiling
threading.EnableProfiling(true);

// Tarefas com profiling automático
auto future = DRIFT_ASYNC_NAMED([]() {
    // Trabalho aqui
}, "TarefaImportante");

// Logs de profiling aparecerão automaticamente
```

### Macros de Profiling

```cpp
DRIFT_PROFILE_THREAD_SCOPE("MeuBlocoDeCodigo") {
    // Código a ser medido
    ProcessarDados();
}
```

## Integração com Subsistemas

### Renderer

```cpp
class RenderManager {
    void RenderFrame() {
        // Renderiza em paralelo
        auto geometryInfo = Drift::Core::Threading::TaskInfo{};
        geometryInfo.name = "GeometryPass";
        auto geometryFuture = threadingSystem.SubmitWithInfo(geometryInfo, [this]() {
            RenderGeometry();
        });
        
        auto lightingInfo = Drift::Core::Threading::TaskInfo{};
        lightingInfo.name = "LightingPass";
        auto lightingFuture = threadingSystem.SubmitWithInfo(lightingInfo, [this]() {
            RenderLighting();
        });
        
        DRIFT_WAIT_FOR_ALL();
    }
};
```

### Asset Manager

```cpp
class AssetManager {
    void LoadAssets(const std::vector<std::string>& paths) {
        for (const auto& path : paths) {
            auto info = Drift::Core::Threading::TaskInfo{};
            info.name = "LoadAsset_" + path;
            threadingSystem.SubmitWithInfo(info, [this, path]() {
                LoadAsset(path);
            });
        }
        DRIFT_WAIT_FOR_ALL();
    }
};
```

### Physics

```cpp
class PhysicsSystem {
    void UpdatePhysics() {
        // Atualiza corpos em paralelo
        for (auto& body : m_Bodies) {
            auto info = Drift::Core::Threading::TaskInfo{};
            info.name = "PhysicsBody_" + body.GetId();
            threadingSystem.SubmitWithInfo(info, [&body]() {
                body.Update();
            });
        }
        DRIFT_WAIT_FOR_ALL();
    }
};
```

## Performance e Otimizações

### Work Stealing
- Threads roubam trabalho de outras threads ociosas
- Balanceamento automático de carga
- Reduz idle time das threads

### CPU Affinity
- Threads são fixadas em cores específicos
- Reduz cache misses
- Melhora performance em sistemas NUMA

### Filas Otimizadas
- Filas locais por thread (lock-free)
- Fila global para balanceamento
- Reduz contenção de locks

### Prioridades
- 4 níveis de prioridade
- Escalonamento inteligente
- Tarefas críticas executam primeiro

## Boas Práticas

### ✅ **Faça**
- Use `SubmitWithInfo` diretamente para lambdas complexas com captura de variáveis
- Use `DRIFT_ASYNC_NAMED` apenas para tarefas simples sem captura
- Configure profiling durante desenvolvimento
- Monitore estatísticas em produção
- Use prioridades apropriadas
- Divida trabalho em chunks razoáveis

### ❌ **Não Faça**
- Não bloqueie threads com I/O síncrono
- Não crie tarefas muito pequenas
- Não ignore exceções nas tarefas
- Não use o sistema sem inicializar
- Não misture diferentes sistemas de threading

## Troubleshooting

### Problema: Erro de compilação com DRIFT_ASYNC_NAMED
```cpp
// ❌ Erro: Lambda com captura de variáveis
auto future = DRIFT_ASYNC_NAMED([&data, &result]() {
    ProcessData(data, result);
}, "ProcessData");

// ✅ Correto: Use SubmitWithInfo diretamente
auto info = Drift::Core::Threading::TaskInfo{};
info.name = "ProcessData";
auto future = threadingSystem.SubmitWithInfo(info, [&data, &result]() {
    ProcessData(data, result);
});
```

### Problema: Sistema não inicializa
```cpp
// Verifique se não está inicializado duas vezes
if (!threading.IsInitialized()) {
    threading.Initialize();
}
```

### Problema: Tarefas não executam
```cpp
// Verifique se o sistema está rodando
if (threading.IsRunning()) {
    // Submeta tarefas
}
```

### Problema: Performance ruim
```cpp
// Habilite profiling para identificar gargalos
threading.EnableProfiling(true);

// Monitore estatísticas
threading.LogStats();
```

## Migração do Sistema Anterior

### Antes (Sistema Complexo)
```cpp
auto& pool = DRIFT_THREAD_POOL();
auto& jobSystem = DRIFT_JOB_SYSTEM();

auto job = jobSystem.CreateJob(func, config);
jobSystem.ExecuteJobAsync(job);
job->Wait();
```

### Depois (Sistema Simples)
```cpp
auto future = DRIFT_ASYNC(func);
future.Get();
```

## Conclusão

O novo sistema de threading do DriftEngine oferece:

- **Simplicidade**: Interface fácil de usar
- **Performance**: Otimizações avançadas
- **Profissionalismo**: Código limpo e robusto
- **Flexibilidade**: Usável em qualquer contexto

Com este sistema, você pode focar no que realmente importa: criar código eficiente e produtivo, sem se preocupar com a complexidade do threading. 