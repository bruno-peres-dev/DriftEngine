# Sistema de Assets - DriftEngine

## 📋 Visão Geral

O Sistema de Assets do DriftEngine é um sistema unificado, profissional e otimizado para gerenciamento de recursos em tempo real. Ele oferece carregamento assíncrono, cache inteligente, prioridades de carregamento e integração perfeita com o sistema de threading.

## ✨ Características Principais

- **🔄 Carregamento Assíncrono**: Integração com ThreadingSystem para carregamento em background
- **💾 Cache Inteligente**: Sistema LRU com limites de memória configuráveis
- **⚡ Prioridades**: Diferentes níveis de prioridade (Low, Normal, High, Critical)
- **📊 Estatísticas**: Monitoramento detalhado de performance e uso de memória
- **🎯 Interface Simples**: Macros e APIs intuitivas para uso fácil
- **🔧 Extensível**: Sistema de loaders plugáveis para diferentes tipos de assets

## 🏗️ Arquitetura

### Componentes Principais

1. **AssetsSystem**: Sistema principal singleton
2. **IAsset**: Interface base para todos os assets
3. **IAssetLoader**: Interface para loaders específicos
4. **AssetCacheEntry**: Entrada no cache com metadados
5. **AssetsExample**: Exemplos de uso e implementação

### Estrutura de Arquivos

```
src/core/include/Drift/Core/Assets/
├── AssetsSystem.h          # Sistema principal
├── AssetsExample.h         # Exemplos e implementações
└── README.md              # Esta documentação

src/core/src/Assets/
├── AssetsSystem.cpp        # Implementação do sistema
└── AssetsExample.cpp       # Implementação dos exemplos
```

## 🚀 Início Rápido

### 1. Inicialização

```cpp
#include "Drift/Core/Assets/AssetsSystem.h"

// Obter instância do sistema
auto& assetsSystem = Drift::Core::Assets::AssetsSystem::GetInstance();

// Configurar
Drift::Core::Assets::AssetsConfig config;
config.maxAssets = 1000;
config.maxMemoryUsage = 512 * 1024 * 1024; // 512MB
config.enableAsyncLoading = true;
config.enablePreloading = true;

assetsSystem.Initialize(config);
```

### 2. Registrar Loader

```cpp
// Criar loader personalizado
auto loader = std::make_unique<MyAssetLoader>();
assetsSystem.RegisterLoader<MyAsset>(std::move(loader));
```

### 3. Carregar Assets

```cpp
// Carregamento síncrono
auto asset = DRIFT_LOAD_ASSET(MyAsset, "path/to/asset.asset");

// Carregamento assíncrono
auto future = DRIFT_LOAD_ASSET_ASYNC(MyAsset, "path/to/asset.asset");
auto asset = future.get();

// Pré-carregamento
DRIFT_PRELOAD_ASSET(MyAsset, "path/to/asset.asset");
```

## 📚 API de Referência

### AssetsSystem

#### Inicialização e Configuração

```cpp
// Singleton
static AssetsSystem& GetInstance();

// Inicialização
void Initialize(const AssetsConfig& config = {});

// Finalização
void Shutdown();

// Configuração
void SetConfig(const AssetsConfig& config);
const AssetsConfig& GetConfig() const;
```

#### Registro de Loaders

```cpp
template<typename T>
void RegisterLoader(std::unique_ptr<IAssetLoader<T>> loader);

template<typename T>
void UnregisterLoader();
```

#### Carregamento de Assets

```cpp
// Carregamento com prioridade
template<typename T>
std::shared_ptr<T> LoadAsset(const std::string& path, 
                            const std::string& variant = "", 
                            const std::any& params = {}, 
                            AssetPriority priority = AssetPriority::Normal);

// Carregamento síncrono
template<typename T>
std::shared_ptr<T> LoadAssetSync(const std::string& path, 
                                const std::string& variant = "", 
                                const std::any& params = {});

// Carregamento assíncrono
template<typename T>
std::future<std::shared_ptr<T>> LoadAssetAsync(const std::string& path, 
                                              const std::string& variant = "", 
                                              const std::any& params = {}, 
                                              AssetPriority priority = AssetPriority::Normal);

// Obter asset do cache
template<typename T>
std::shared_ptr<T> GetAsset(const std::string& path, const std::string& variant = "");

// Obter ou carregar
template<typename T>
std::shared_ptr<T> GetOrLoadAsset(const std::string& path, 
                                 const std::string& variant = "", 
                                 const std::any& params = {}, 
                                 AssetPriority priority = AssetPriority::Normal);
```

#### Gerenciamento de Cache

```cpp
// Descarregar assets
void UnloadAsset(const std::string& path, std::type_index type, const std::string& variant = "");
void UnloadAssets(std::type_index type);
void UnloadUnusedAssets();

// Limpeza
void ClearCache();
void TrimCache();
```

#### Estatísticas e Debug

```cpp
// Obter estatísticas
AssetsStats GetStats() const;
void LogStats() const;
void ResetStats();

// Verificação de status
bool IsAssetLoaded(const std::string& path, std::type_index type, const std::string& variant = "") const;
bool IsAssetLoading(const std::string& path, std::type_index type, const std::string& variant = "") const;
AssetStatus GetAssetStatus(const std::string& path, std::type_index type, const std::string& variant = "") const;
```

### IAsset Interface

```cpp
class IAsset {
public:
    virtual ~IAsset() = default;
    
    // Informações básicas
    virtual const std::string& GetPath() const = 0;
    virtual const std::string& GetName() const = 0;
    virtual size_t GetMemoryUsage() const = 0;
    virtual AssetStatus GetStatus() const = 0;
    
    // Controle de carregamento
    virtual bool Load() = 0;
    virtual void Unload() = 0;
    virtual bool IsLoaded() const = 0;
    
    // Metadados
    virtual std::chrono::steady_clock::time_point GetLoadTime() const = 0;
    virtual size_t GetAccessCount() const = 0;
    virtual void UpdateAccess() = 0;
};
```

### IAssetLoader Interface

```cpp
template<typename T>
class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;
    
    // Carregamento
    virtual std::shared_ptr<T> Load(const std::string& path, const std::any& params = {}) = 0;
    virtual bool CanLoad(const std::string& path) const = 0;
    virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    
    // Informações
    virtual std::string GetLoaderName() const = 0;
    virtual size_t EstimateMemoryUsage(const std::string& path) const = 0;
};
```

## 🎯 Macros Úteis

```cpp
// Macros para facilitar o uso
#define DRIFT_ASSETS() Drift::Core::Assets::AssetsSystem::GetInstance()

#define DRIFT_LOAD_ASSET(type, path) \
    DRIFT_ASSETS().LoadAsset<type>(path)

#define DRIFT_LOAD_ASSET_ASYNC(type, path) \
    DRIFT_ASSETS().LoadAssetAsync<type>(path)

#define DRIFT_GET_ASSET(type, path) \
    DRIFT_ASSETS().GetAsset<type>(path)

#define DRIFT_GET_OR_LOAD_ASSET(type, path) \
    DRIFT_ASSETS().GetOrLoadAsset<type>(path)

#define DRIFT_PRELOAD_ASSET(type, path) \
    DRIFT_ASSETS().PreloadAsset<type>(path)
```

## 📊 Configuração

### AssetsConfig

```cpp
struct AssetsConfig {
    size_t maxAssets = 1000;                       // Número máximo de assets em cache
    size_t maxMemoryUsage = 1024 * 1024 * 1024;   // Uso máximo de memória (1GB)
    bool enableAsyncLoading = true;                // Habilita carregamento assíncrono
    bool enablePreloading = true;                  // Habilita pré-carregamento
    bool enableLazyUnloading = true;               // Habilita descarregamento automático
    float trimThreshold = 0.8f;                    // Threshold para limpeza (80%)
    size_t maxConcurrentLoads = 8;                 // Máximo de carregamentos simultâneos
    std::string defaultAssetPath = "assets/";      // Caminho padrão para assets
};
```

### Prioridades de Carregamento

```cpp
enum class AssetPriority {
    Low = 0,        // Carregamento em background
    Normal = 1,     // Carregamento padrão
    High = 2,       // Carregamento prioritário
    Critical = 3    // Carregamento imediato (bloqueante)
};
```

### Status de Assets

```cpp
enum class AssetStatus {
    NotLoaded,      // Asset não carregado
    Loading,        // Asset sendo carregado
    Loaded,         // Asset carregado com sucesso
    Failed,         // Falha no carregamento
    Unloading       // Asset sendo descarregado
};
```

## 📈 Estatísticas

### AssetsStats

```cpp
struct AssetsStats {
    size_t totalAssets = 0;                    // Total de assets no cache
    size_t loadedAssets = 0;                   // Assets carregados
    size_t loadingAssets = 0;                  // Assets carregando
    size_t failedAssets = 0;                   // Assets que falharam
    size_t memoryUsage = 0;                    // Uso atual de memória
    size_t maxMemoryUsage = 0;                 // Limite de memória
    size_t cacheHits = 0;                      // Cache hits
    size_t cacheMisses = 0;                    // Cache misses
    size_t loadCount = 0;                      // Total de carregamentos
    size_t unloadCount = 0;                    // Total de descarregamentos
    size_t asyncLoadCount = 0;                 // Carregamentos assíncronos
    double averageLoadTime = 0.0;              // Tempo médio de carregamento
    
    // Estatísticas por tipo
    std::unordered_map<std::type_index, size_t> assetsByType;
    std::unordered_map<std::type_index, size_t> memoryByType;
    std::unordered_map<std::type_index, size_t> loadCountByType;
};
```

## 🔧 Exemplos de Implementação

### Asset Simples

```cpp
class SimpleAsset : public IAsset {
public:
    SimpleAsset(const std::string& path, const std::string& name);
    
    // IAsset interface
    const std::string& GetPath() const override { return m_Path; }
    const std::string& GetName() const override { return m_Name; }
    size_t GetMemoryUsage() const override { return m_MemoryUsage; }
    AssetStatus GetStatus() const override { return m_Status; }
    
    bool Load() override;
    void Unload() override;
    bool IsLoaded() const override { return m_Status == AssetStatus::Loaded; }
    
    std::chrono::steady_clock::time_point GetLoadTime() const override { return m_LoadTime; }
    size_t GetAccessCount() const override { return m_AccessCount; }
    void UpdateAccess() override { m_AccessCount++; }

private:
    std::string m_Path;
    std::string m_Name;
    size_t m_MemoryUsage = 0;
    AssetStatus m_Status = AssetStatus::NotLoaded;
    std::chrono::steady_clock::time_point m_LoadTime;
    size_t m_AccessCount = 0;
};
```

### Loader Simples

```cpp
class SimpleAssetLoader : public IAssetLoader<SimpleAsset> {
public:
    std::shared_ptr<SimpleAsset> Load(const std::string& path, const std::any& params = {}) override;
    bool CanLoad(const std::string& path) const override;
    std::vector<std::string> GetSupportedExtensions() const override;
    std::string GetLoaderName() const override { return "SimpleAssetLoader"; }
    size_t EstimateMemoryUsage(const std::string& path) const override;
};
```

## 🎮 Integração com o App

### Inicialização no main.cpp

```cpp
// Inicializar sistema de assets
auto& assetsSystem = Drift::Core::Assets::AssetsSystem::GetInstance();

AssetsConfig assetsConfig;
assetsConfig.maxAssets = 1000;
assetsConfig.maxMemoryUsage = 512 * 1024 * 1024; // 512MB
assetsConfig.enableAsyncLoading = true;
assetsConfig.enablePreloading = true;
assetsConfig.enableLazyUnloading = true;
assetsConfig.maxConcurrentLoads = 8;

assetsSystem.Initialize(assetsConfig);

// Executar exemplo
Drift::Core::Assets::AssetsExample::RunBasicExample();
```

### Teste Interativo

```cpp
// Teste do sistema de assets com A
if (input.IsKeyPressed(Engine::Input::Key::A)) {
    Core::Log("[App] Executando teste de assets...");
    
    auto& assetsSystem = Drift::Core::Assets::AssetsSystem::GetInstance();
    
    // Teste de carregamento assíncrono
    std::vector<std::future<std::shared_ptr<Drift::Core::Assets::SimpleAsset>>> futures;
    
    for (int i = 0; i < 20; ++i) {
        std::string path = "test_asset_" + std::to_string(i) + ".asset";
        auto future = DRIFT_LOAD_ASSET_ASYNC(Drift::Core::Assets::SimpleAsset, path);
        futures.push_back(std::move(future));
    }
    
    // Aguarda e mostra resultados
    int loadedCount = 0;
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            auto asset = futures[i].get();
            if (asset) {
                loadedCount++;
                Core::Log("[App] Asset " + std::to_string(i) + " carregado: " + asset->GetName());
            }
        } catch (const std::exception& e) {
            Core::LogError("[App] Falha ao carregar asset " + std::to_string(i) + ": " + e.what());
        }
    }
    
    Core::Log("[App] Carregamento concluído: " + std::to_string(loadedCount) + "/" + std::to_string(futures.size()) + " assets");
    assetsSystem.LogStats();
}
```

## 🚨 Troubleshooting

### Problemas Comuns

1. **Asset não carrega**
   - Verificar se o loader está registrado
   - Verificar se o path está correto
   - Verificar logs de erro

2. **Memória insuficiente**
   - Ajustar `maxMemoryUsage` na configuração
   - Verificar se assets estão sendo descarregados
   - Usar `TrimCache()` para limpeza manual

3. **Carregamento lento**
   - Verificar se carregamento assíncrono está habilitado
   - Ajustar `maxConcurrentLoads`
   - Verificar prioridades de carregamento

4. **Cache misses altos**
   - Verificar se assets estão sendo pré-carregados
   - Ajustar estratégia de cache
   - Verificar se assets estão sendo mantidos em memória

### Logs Úteis

```cpp
// Habilitar logs detalhados
Core::SetLogLevel(Core::LogLevel::Debug);

// Ver estatísticas
assetsSystem.LogStats();

// Verificar status de asset específico
auto status = assetsSystem.GetAssetStatus("path/to/asset", std::type_index(typeid(MyAsset)));
```

## 📝 Boas Práticas

1. **Sempre usar macros** para operações comuns
2. **Configurar limites** de memória apropriados
3. **Usar carregamento assíncrono** para assets não críticos
4. **Implementar estimativa de memória** nos loaders
5. **Monitorar estatísticas** regularmente
6. **Usar callbacks** para eventos importantes
7. **Limpar cache** periodicamente
8. **Testar performance** com diferentes configurações

## 🔄 Migração do Sistema Antigo

### Arquivos Removidos

- `AssetsManager.h/cpp` - Substituído por `AssetsSystem.h/cpp`
- `AssetsManagerExample.h/cpp` - Substituído por `AssetsExample.h/cpp`
- `TextureAsset.h/cpp` - Implementar conforme necessário
- `FontAsset.h/cpp` - Implementar conforme necessário
- `Integration.h/cpp` - Integração direta no sistema principal

### Mudanças na API

```cpp
// Antes
auto& manager = Drift::Core::AssetsManager::GetInstance();
auto asset = manager.LoadAsset<MyAsset>("path");

// Depois
auto& system = Drift::Core::Assets::AssetsSystem::GetInstance();
auto asset = DRIFT_LOAD_ASSET(MyAsset, "path");
```

## 📚 Referências

- [ThreadingSystem](../Threading/README.md) - Sistema de threading integrado
- [Log System](../../Log.h) - Sistema de logging
- [CMakeLists.txt](../../../../CMakeLists.txt) - Configuração de build

---

**Sistema de Assets - DriftEngine**  
*Profissional, Otimizado e Fácil de Usar* 🎯 