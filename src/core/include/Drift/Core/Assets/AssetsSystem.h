#pragma once

#include "Drift/Core/Threading/ThreadingSystem.h"
#include "Drift/Core/Log.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <functional>
#include <typeindex>
#include <vector>
#include <chrono>
#include <iomanip>
#include <any>
#include <future>
#include <queue>

namespace Drift::Core::Assets {

/**
 * @brief Prioridade de carregamento de assets
 */
enum class AssetPriority {
    Low = 0,        // Carregamento em background
    Normal = 1,     // Carregamento padrão
    High = 2,       // Carregamento prioritário
    Critical = 3    // Carregamento imediato (bloqueante)
};

/**
 * @brief Status de um asset
 */
enum class AssetStatus {
    NotLoaded,      // Asset não carregado
    Loading,        // Asset sendo carregado
    Loaded,         // Asset carregado com sucesso
    Failed,         // Falha no carregamento
    Unloading       // Asset sendo descarregado
};

/**
 * @brief Configuração do sistema de assets
 */
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

/**
 * @brief Interface base para assets
 */
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

/**
 * @brief Interface para loaders de assets
 */
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

/**
 * @brief Chave única para identificação de assets
 */
struct AssetKey {
    std::string path;
    std::type_index type;
    std::string variant;
    
    AssetKey(const std::string& p, std::type_index t, const std::string& v = "")
        : path(p), type(t), variant(v) {}
    
    bool operator==(const AssetKey& other) const {
        return path == other.path && type == other.type && variant == other.variant;
    }
    
    std::string ToString() const {
        return path + ":" + std::string(type.name()) + ":" + variant;
    }
};

/**
 * @brief Hash function para AssetKey
 */
struct AssetKeyHash {
    size_t operator()(const AssetKey& k) const noexcept {
        return std::hash<std::string>{}(k.path) ^ 
               (std::hash<std::type_index>{}(k.type) << 1) ^
               (std::hash<std::string>{}(k.variant) << 2);
    }
};

/**
 * @brief Entrada de asset no cache
 */
struct AssetCacheEntry {
    std::shared_ptr<IAsset> asset;
    AssetStatus status = AssetStatus::NotLoaded;
    size_t lastAccess = 0;
    size_t accessCount = 0;
    size_t memoryUsage = 0;
    std::chrono::steady_clock::time_point loadTime;
    bool isPreloaded = false;
    AssetPriority priority = AssetPriority::Normal;
    std::string errorMessage;
    
    // Para carregamento assíncrono
    bool isAsyncLoading = false;
};

/**
 * @brief Estatísticas do sistema de assets
 */
struct AssetsStats {
    size_t totalAssets = 0;
    size_t loadedAssets = 0;
    size_t loadingAssets = 0;
    size_t failedAssets = 0;
    size_t memoryUsage = 0;
    size_t maxMemoryUsage = 0;
    size_t cacheHits = 0;
    size_t cacheMisses = 0;
    size_t loadCount = 0;
    size_t unloadCount = 0;
    size_t asyncLoadCount = 0;
    double averageLoadTime = 0.0;
    
    // Estatísticas por tipo
    std::unordered_map<std::type_index, size_t> assetsByType;
    std::unordered_map<std::type_index, size_t> memoryByType;
    std::unordered_map<std::type_index, size_t> loadCountByType;
};

/**
 * @brief Sistema de Assets unificado e profissional
 * 
 * Características:
 * - Carregamento assíncrono com threading
 * - Cache inteligente com LRU
 * - Prioridades de carregamento
 * - Pré-carregamento automático
 * - Estatísticas detalhadas
 * - Interface simples e intuitiva
 * - Integração com qualquer subsistema
 */
class AssetsSystem {
public:
    static AssetsSystem& GetInstance();
    
    // Inicialização e configuração
    void Initialize(const AssetsConfig& config = {});
    void Shutdown();
    
    // Configuração
    void SetConfig(const AssetsConfig& config);
    const AssetsConfig& GetConfig() const { return m_Config; }
    
    // Registro de loaders
    template<typename T>
    void RegisterLoader(std::unique_ptr<IAssetLoader<T>> loader);
    
    template<typename T>
    void UnregisterLoader();
    
    // Carregamento de assets
    template<typename T>
    std::shared_ptr<T> LoadAsset(const std::string& path, const std::string& variant = "", 
                                const std::any& params = {}, AssetPriority priority = AssetPriority::Normal);
    
    template<typename T>
    std::shared_ptr<T> LoadAssetSync(const std::string& path, const std::string& variant = "", 
                                    const std::any& params = {});
    
    template<typename T>
    std::shared_ptr<T> GetAsset(const std::string& path, const std::string& variant = "");
    
    template<typename T>
    std::shared_ptr<T> GetOrLoadAsset(const std::string& path, const std::string& variant = "", 
                                     const std::any& params = {}, AssetPriority priority = AssetPriority::Normal);
    
    // Carregamento assíncrono
    template<typename T>
    std::future<std::shared_ptr<T>> LoadAssetAsync(const std::string& path, const std::string& variant = "", 
                                                   const std::any& params = {}, AssetPriority priority = AssetPriority::Normal);
    
    // Pré-carregamento
    template<typename T>
    void PreloadAsset(const std::string& path, const std::string& variant = "", 
                     const std::any& params = {}, AssetPriority priority = AssetPriority::Low);
    
    void PreloadAssets(const std::vector<std::string>& paths);
    
    // Gerenciamento de cache
    void UnloadAsset(const std::string& path, std::type_index type, const std::string& variant = "");
    void UnloadAssets(std::type_index type);
    void UnloadUnusedAssets();
    void ClearCache();
    void TrimCache();
    
    // Status e verificação
    bool IsAssetLoaded(const std::string& path, std::type_index type, const std::string& variant = "") const;
    bool IsAssetLoading(const std::string& path, std::type_index type, const std::string& variant = "") const;
    AssetStatus GetAssetStatus(const std::string& path, std::type_index type, const std::string& variant = "") const;
    bool CanLoadAsset(const std::string& path, std::type_index type) const;
    std::vector<std::string> GetSupportedExtensions(std::type_index type) const;
    
    // Estatísticas e debug
    AssetsStats GetStats() const;
    void LogStats() const;
    void ResetStats();
    
    // Eventos e callbacks
    using AssetLoadedCallback = std::function<void(const std::string& path, std::type_index type)>;
    using AssetUnloadedCallback = std::function<void(const std::string& path, std::type_index type)>;
    using AssetFailedCallback = std::function<void(const std::string& path, std::type_index type, const std::string& error)>;
    
    void SetAssetLoadedCallback(AssetLoadedCallback callback) { m_AssetLoadedCallback = callback; }
    void SetAssetUnloadedCallback(AssetUnloadedCallback callback) { m_AssetUnloadedCallback = callback; }
    void SetAssetFailedCallback(AssetFailedCallback callback) { m_AssetFailedCallback = callback; }
    
    // Utilitários
    void WaitForAllLoads();
    void CancelAllLoads();
    size_t GetLoadingCount() const;
    size_t GetQueuedCount() const;

private:
    AssetsSystem() = default;
    ~AssetsSystem() = default;
    AssetsSystem(const AssetsSystem&) = delete;
    AssetsSystem& operator=(const AssetsSystem&) = delete;
    
    // Cache de assets
    std::unordered_map<AssetKey, AssetCacheEntry, AssetKeyHash> m_Assets;
    
    // Loaders registrados
    std::unordered_map<std::type_index, std::any> m_Loaders;
    
    // Configuração e estado
    AssetsConfig m_Config;
    mutable std::mutex m_Mutex;
    size_t m_AccessCounter = 0;
    bool m_Initialized = false;
    
    // Estatísticas
    mutable size_t m_CacheHits = 0;
    mutable size_t m_CacheMisses = 0;
    mutable size_t m_LoadCount = 0;
    mutable size_t m_UnloadCount = 0;
    mutable size_t m_AsyncLoadCount = 0;
    mutable double m_TotalLoadTime = 0.0;
    
    // Callbacks
    AssetLoadedCallback m_AssetLoadedCallback;
    AssetUnloadedCallback m_AssetUnloadedCallback;
    AssetFailedCallback m_AssetFailedCallback;
    
    // Métodos auxiliares
    template<typename T>
    IAssetLoader<T>* GetLoader() const;
    
    bool EvictLeastUsedAsset();
    void UpdateAccessStats(AssetCacheEntry& entry);
    size_t CalculateCurrentMemoryUsage() const;
    void TriggerAssetLoadedCallback(const std::string& path, std::type_index type);
    void TriggerAssetUnloadedCallback(const std::string& path, std::type_index type);
    void TriggerAssetFailedCallback(const std::string& path, std::type_index type, const std::string& error);
    
    // Carregamento assíncrono
    template<typename T>
    void LoadAssetAsyncInternal(const AssetKey& key, const std::any& params, AssetPriority priority);
    
    void ProcessAsyncLoads();
    void CleanupCompletedLoads();
};

// Implementação dos templates
template<typename T>
void AssetsSystem::RegisterLoader(std::unique_ptr<IAssetLoader<T>> loader) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Loaders[std::type_index(typeid(T))] = std::any{std::shared_ptr<void>(std::move(loader))};
    DRIFT_LOG_INFO("[AssetsSystem] Loader registrado: ", std::string(typeid(T).name()));
}

template<typename T>
void AssetsSystem::UnregisterLoader() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Loaders.erase(std::type_index(typeid(T)));
    DRIFT_LOG_INFO("[AssetsSystem] Loader removido: ", std::string(typeid(T).name()));
}

template<typename T>
IAssetLoader<T>* AssetsSystem::GetLoader() const {
    auto it = m_Loaders.find(std::type_index(typeid(T)));
    if (it != m_Loaders.end()) {
        auto shared_ptr = std::any_cast<std::shared_ptr<void>>(it->second);
        return static_cast<IAssetLoader<T>*>(shared_ptr.get());
    }
    return nullptr;
}

template<typename T>
std::shared_ptr<T> AssetsSystem::LoadAsset(const std::string& path, const std::string& variant, 
                                          const std::any& params, AssetPriority priority) {
    if (priority == AssetPriority::Critical) {
        // Carregamento síncrono para prioridade crítica
        return LoadAssetSync<T>(path, variant, params);
    } else if (m_Config.enableAsyncLoading) {
        // Carregamento assíncrono
        auto future = LoadAssetAsync<T>(path, variant, params, priority);
        return future.get(); // Aguarda conclusão
    } else {
        // Carregamento síncrono
        return LoadAssetSync<T>(path, variant, params);
    }
}



template<typename T>
std::shared_ptr<T> AssetsSystem::LoadAssetSync(const std::string& path, const std::string& variant, 
                                              const std::any& params) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, std::type_index(typeid(T)), variant);
    auto it = m_Assets.find(key);
    
    if (it != m_Assets.end()) {
        // Asset já existe no cache
        UpdateAccessStats(it->second);
        m_CacheHits++;
        
        if (it->second.status == AssetStatus::Loaded) {
            return std::static_pointer_cast<T>(it->second.asset);
        } else if (it->second.status == AssetStatus::Loading) {
            // Aguarda carregamento assíncrono
            while (it->second.status == AssetStatus::Loading) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            if (it->second.status == AssetStatus::Loaded) {
                return std::static_pointer_cast<T>(it->second.asset);
            }
        }
    }
    
    // Asset não existe ou falhou - carrega
    m_CacheMisses++;
    
    IAssetLoader<T>* loader = GetLoader<T>();
    if (!loader) {
        DRIFT_LOG_ERROR("[AssetsSystem] Loader não encontrado para tipo: ", std::string(typeid(T).name()));
        return nullptr;
    }
    
    auto startTime = std::chrono::steady_clock::now();
    auto asset = loader->Load(path, params);
    auto endTime = std::chrono::steady_clock::now();
    
    if (!asset) {
        DRIFT_LOG_ERROR("[AssetsSystem] Falha ao carregar asset: ", path);
        return nullptr;
    }
    
    // Calcula tempo de carregamento
    auto loadTime = std::chrono::duration<double>(endTime - startTime).count();
    m_TotalLoadTime += loadTime;
    m_LoadCount++;
    
    // Verifica limites de memória
    size_t assetMemory = asset->GetMemoryUsage();
    if (CalculateCurrentMemoryUsage() + assetMemory > m_Config.maxMemoryUsage) {
        while (CalculateCurrentMemoryUsage() + assetMemory > m_Config.maxMemoryUsage) {
            if (!EvictLeastUsedAsset()) {
                break;
            }
        }
    }
    
    // Adiciona ao cache
    AssetCacheEntry entry;
    entry.asset = asset;
    entry.status = AssetStatus::Loaded;
    entry.lastAccess = ++m_AccessCounter;
    entry.accessCount = 1;
    entry.memoryUsage = assetMemory;
    entry.loadTime = endTime;
    entry.isPreloaded = false;
    entry.priority = AssetPriority::Normal;
    
    m_Assets[key] = entry;
    
    // Verifica limite de quantidade
    if (m_Assets.size() > m_Config.maxAssets) {
        EvictLeastUsedAsset();
    }
    
    TriggerAssetLoadedCallback(path, std::type_index(typeid(T)));
    
            DRIFT_LOG_INFO("[AssetsSystem] Asset carregado: ", path, " (", std::fixed, std::setprecision(2), loadTime * 1000.0, "ms)");
    
    return asset;
}

template<typename T>
std::shared_ptr<T> AssetsSystem::GetAsset(const std::string& path, const std::string& variant) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, std::type_index(typeid(T)), variant);
    auto it = m_Assets.find(key);
    
    if (it != m_Assets.end() && it->second.status == AssetStatus::Loaded) {
        UpdateAccessStats(it->second);
        m_CacheHits++;
        return std::static_pointer_cast<T>(it->second.asset);
    }
    
    m_CacheMisses++;
    return nullptr;
}

template<typename T>
std::shared_ptr<T> AssetsSystem::GetOrLoadAsset(const std::string& path, const std::string& variant, 
                                                const std::any& params, AssetPriority priority) {
    auto asset = GetAsset<T>(path, variant);
    if (asset) {
        return asset;
    }
    
    return LoadAsset<T>(path, variant, params, priority);
}

template<typename T>
std::future<std::shared_ptr<T>> AssetsSystem::LoadAssetAsync(const std::string& path, const std::string& variant, 
                                                            const std::any& params, AssetPriority priority) {
    AssetKey key(path, std::type_index(typeid(T)), variant);
    
    // Verifica se já está carregado
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Assets.find(key);
        if (it != m_Assets.end() && it->second.status == AssetStatus::Loaded) {
            UpdateAccessStats(it->second);
            m_CacheHits++;
            auto asset = std::static_pointer_cast<T>(it->second.asset);
            return std::async(std::launch::deferred, [asset]() { return asset; });
        }
    }
    
    // Inicia carregamento assíncrono
    LoadAssetAsyncInternal<T>(key, params, priority);
    
    // Retorna future que aguarda o carregamento
    return std::async(std::launch::async, [this, key]() {
        while (true) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            auto it = m_Assets.find(key);
            if (it != m_Assets.end()) {
                if (it->second.status == AssetStatus::Loaded) {
                    return std::static_pointer_cast<T>(it->second.asset);
                } else if (it->second.status == AssetStatus::Failed) {
                    return std::shared_ptr<T>(nullptr);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
}

template<typename T>
void AssetsSystem::PreloadAsset(const std::string& path, const std::string& variant, 
                               const std::any& params, AssetPriority priority) {
    if (!m_Config.enablePreloading) {
        return;
    }
    
    LoadAssetAsync<T>(path, variant, params, priority);
}

template<typename T>
void AssetsSystem::LoadAssetAsyncInternal(const AssetKey& key, const std::any& params, AssetPriority priority) {
    // Marca como carregando
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto& entry = m_Assets[key];
        entry.status = AssetStatus::Loading;
        entry.isAsyncLoading = true;
        entry.priority = priority;
    }
    
    // Cria a tarefa assíncrona
    auto task = [this, key, params]() {
        try {
            IAssetLoader<T>* loader = GetLoader<T>();
            if (!loader) {
                throw std::runtime_error("Loader não encontrado");
            }
            
            auto asset = loader->Load(key.path, params);
            if (!asset) {
                throw std::runtime_error("Falha ao carregar asset");
            }
            
            // Atualiza o cache
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                auto it = m_Assets.find(key);
                if (it != m_Assets.end()) {
                    it->second.asset = asset;
                    it->second.status = AssetStatus::Loaded;
                    it->second.isAsyncLoading = false;
                    it->second.memoryUsage = asset->GetMemoryUsage();
                    it->second.loadTime = std::chrono::steady_clock::now();
                    
                    m_AsyncLoadCount++;
                    TriggerAssetLoadedCallback(key.path, key.type);
                    
                    DRIFT_LOG_INFO("[AssetsSystem] Asset carregado assincronamente: ", key.path);
                }
            }
            
        } catch (const std::exception& e) {
            // Marca como falhou
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                auto it = m_Assets.find(key);
                if (it != m_Assets.end()) {
                    it->second.status = AssetStatus::Failed;
                    it->second.isAsyncLoading = false;
                    it->second.errorMessage = e.what();
                    
                    TriggerAssetFailedCallback(key.path, key.type, e.what());
                    
                    DRIFT_LOG_ERROR("[AssetsSystem] Falha ao carregar asset: ", key.path, " - ", e.what());
                }
            }
        }
    };
    
    // Submete a tarefa ao sistema de threading
    auto priority_enum = static_cast<Drift::Core::Threading::TaskPriority>(priority);
    auto info = Drift::Core::Threading::TaskInfo{};
    info.name = "LoadAsset_" + key.path;
    info.priority = priority_enum;
    
    Drift::Core::Threading::ThreadingSystem::GetInstance().SubmitWithInfo(info, task);
}

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

} // namespace Drift::Core::Assets

