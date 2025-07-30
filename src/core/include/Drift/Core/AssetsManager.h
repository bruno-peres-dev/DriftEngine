#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <functional>
#include <typeindex>
#include <vector>
#include <chrono>
#include <any>

namespace Drift::Core {

/**
 * @brief Configuração do cache de assets
 */
struct AssetCacheConfig {
    size_t maxAssets = 1000;                    ///< Número máximo de assets em cache
    size_t maxMemoryUsage = 1024 * 1024 * 1024; ///< Uso máximo de memória (1GB default)
    bool enableLazyLoading = true;              ///< Habilita carregamento sob demanda
    bool enablePreloading = false;              ///< Habilita pré-carregamento
    bool enableAsyncLoading = true;             ///< Habilita carregamento assíncrono
    float trimThreshold = 0.8f;                 ///< Threshold para limpeza automática (80%)
};

/**
 * @brief Interface base para assets
 */
class IAsset {
public:
    virtual ~IAsset() = default;
    virtual size_t GetMemoryUsage() const = 0;
    virtual const std::string& GetPath() const = 0;
    virtual bool IsLoaded() const = 0;
    virtual bool Load() = 0;
    virtual void Unload() = 0;
};

/**
 * @brief Interface para loaders de assets
 */
template<typename T>
class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;
    virtual std::shared_ptr<T> Load(const std::string& path, const std::any& params = {}) = 0;
    virtual bool CanLoad(const std::string& path) const = 0;
    virtual std::vector<std::string> GetSupportedExtensions() const = 0;
};

/**
 * @brief Chave única para identificação de assets
 */
struct AssetKey {
    std::string path;
    std::type_index type;
    std::string variant; // Para diferentes variações (ex: tamanho de fonte, qualidade de textura)
    
    AssetKey(const std::string& p, std::type_index t, const std::string& v = "")
        : path(p), type(t), variant(v) {}
    
    bool operator==(const AssetKey& other) const {
        return path == other.path && type == other.type && variant == other.variant;
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
    size_t lastAccess = 0;
    size_t accessCount = 0;
    size_t memoryUsage = 0;
    std::chrono::steady_clock::time_point loadTime;
    bool isPreloaded = false;
};

/**
 * @brief Estatísticas do cache de assets
 */
struct AssetCacheStats {
    size_t totalAssets = 0;
    size_t loadedAssets = 0;
    size_t memoryUsage = 0;
    size_t maxMemoryUsage = 0;
    size_t cacheHits = 0;
    size_t cacheMisses = 0;
    size_t loadCount = 0;
    size_t unloadCount = 0;
    double averageLoadTime = 0.0;
    
    // Estatísticas por tipo
    std::unordered_map<std::type_index, size_t> assetsByType;
    std::unordered_map<std::type_index, size_t> memoryByType;
};

/**
 * @brief Gerenciador genérico de assets
 * 
 * Esta classe implementa um sistema de cache inteligente para diferentes tipos de assets,
 * incluindo carregamento assíncrono, lazy loading, pré-carregamento e limpeza automática.
 */
class AssetsManager {
public:
    /**
     * @brief Obtém a instância singleton do gerenciador
     */
    static AssetsManager& GetInstance();

    // Configuração
    void SetCacheConfig(const AssetCacheConfig& config);
    const AssetCacheConfig& GetCacheConfig() const { return m_Config; }

    // Registro de loaders
    template<typename T>
    void RegisterLoader(std::unique_ptr<IAssetLoader<T>> loader);
    
    template<typename T>
    void UnregisterLoader();

    // Carregamento de assets
    template<typename T>
    std::shared_ptr<T> LoadAsset(const std::string& path, const std::string& variant = "", const std::any& params = {});
    
    template<typename T>
    std::shared_ptr<T> GetAsset(const std::string& path, const std::string& variant = "");
    
    template<typename T>
    std::shared_ptr<T> GetOrLoadAsset(const std::string& path, const std::string& variant = "", const std::any& params = {});

    // Pré-carregamento
    template<typename T>
    void PreloadAsset(const std::string& path, const std::string& variant = "", const std::any& params = {});
    
    void PreloadAssets(const std::vector<std::string>& paths);

    // Gerenciamento de cache
    void UnloadAsset(const std::string& path, std::type_index type, const std::string& variant = "");
    void UnloadAssets(std::type_index type);
    void UnloadUnusedAssets();
    void ClearCache();
    void TrimCache();

    // Estatísticas e debug
    AssetCacheStats GetCacheStats() const;
    void LogCacheStats() const;
    
    // Verificação de assets
    bool IsAssetLoaded(const std::string& path, std::type_index type, const std::string& variant = "") const;
    bool CanLoadAsset(const std::string& path, std::type_index type) const;
    std::vector<std::string> GetSupportedExtensions(std::type_index type) const;

    // Eventos de callback
    using AssetLoadedCallback = std::function<void(const std::string& path, std::type_index type)>;
    using AssetUnloadedCallback = std::function<void(const std::string& path, std::type_index type)>;
    
    void SetAssetLoadedCallback(AssetLoadedCallback callback) { m_AssetLoadedCallback = callback; }
    void SetAssetUnloadedCallback(AssetUnloadedCallback callback) { m_AssetUnloadedCallback = callback; }

private:
    AssetsManager() = default;
    ~AssetsManager() = default;
    AssetsManager(const AssetsManager&) = delete;
    AssetsManager& operator=(const AssetsManager&) = delete;

    // Cache de assets
    std::unordered_map<AssetKey, AssetCacheEntry, AssetKeyHash> m_Assets;
    
    // Loaders registrados
    std::unordered_map<std::type_index, std::any> m_Loaders;
    
    // Configuração e estado
    AssetCacheConfig m_Config;
    mutable std::mutex m_Mutex;
    size_t m_AccessCounter = 0;
    
    // Estatísticas
    mutable size_t m_CacheHits = 0;
    mutable size_t m_CacheMisses = 0;
    mutable size_t m_LoadCount = 0;
    mutable size_t m_UnloadCount = 0;
    mutable double m_TotalLoadTime = 0.0;
    
    // Callbacks
    AssetLoadedCallback m_AssetLoadedCallback;
    AssetUnloadedCallback m_AssetUnloadedCallback;
    
    // Métodos auxiliares
    template<typename T>
    IAssetLoader<T>* GetLoader() const;
    
    bool EvictLeastUsedAsset();
    void UpdateAccessStats(AssetCacheEntry& entry);
    size_t CalculateCurrentMemoryUsage() const;
    void TriggerAssetLoadedCallback(const std::string& path, std::type_index type);
    void TriggerAssetUnloadedCallback(const std::string& path, std::type_index type);
};

// Implementação dos templates no header para evitar problemas de linkagem

template<typename T>
void AssetsManager::RegisterLoader(std::unique_ptr<IAssetLoader<T>> loader) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Loaders[std::type_index(typeid(T))] = std::any{std::shared_ptr<void>(std::move(loader))};
}

template<typename T>
void AssetsManager::UnregisterLoader() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Loaders.erase(std::type_index(typeid(T)));
}

template<typename T>
IAssetLoader<T>* AssetsManager::GetLoader() const {
    auto it = m_Loaders.find(std::type_index(typeid(T)));
    if (it != m_Loaders.end()) {
        auto shared_ptr = std::any_cast<std::shared_ptr<void>>(it->second);
        return static_cast<IAssetLoader<T>*>(shared_ptr.get());
    }
    return nullptr;
}

template<typename T>
std::shared_ptr<T> AssetsManager::LoadAsset(const std::string& path, const std::string& variant, const std::any& params) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, std::type_index(typeid(T)), variant);
    auto it = m_Assets.find(key);
    
    if (it != m_Assets.end()) {
        // Asset já existe no cache
        UpdateAccessStats(it->second);
        m_CacheHits++;
        return std::static_pointer_cast<T>(it->second.asset);
    }
    
    // Asset não existe - carrega
    m_CacheMisses++;
    
    IAssetLoader<T>* loader = GetLoader<T>();
    if (!loader) {
        return nullptr;
    }
    
    auto startTime = std::chrono::steady_clock::now();
    auto asset = loader->Load(path, params);
    auto endTime = std::chrono::steady_clock::now();
    
    if (!asset) {
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
                break; // Não conseguiu liberar espaço suficiente
            }
        }
    }
    
    // Adiciona ao cache
    AssetCacheEntry entry;
    entry.asset = asset;
    entry.lastAccess = ++m_AccessCounter;
    entry.accessCount = 1;
    entry.memoryUsage = assetMemory;
    entry.loadTime = endTime;
    entry.isPreloaded = false;
    
    m_Assets[key] = entry;
    
    // Verifica limite de quantidade
    if (m_Assets.size() > m_Config.maxAssets) {
        EvictLeastUsedAsset();
    }
    
    TriggerAssetLoadedCallback(path, std::type_index(typeid(T)));
    
    return asset;
}

template<typename T>
std::shared_ptr<T> AssetsManager::GetAsset(const std::string& path, const std::string& variant) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, std::type_index(typeid(T)), variant);
    auto it = m_Assets.find(key);
    
    if (it != m_Assets.end()) {
        UpdateAccessStats(it->second);
        m_CacheHits++;
        return std::static_pointer_cast<T>(it->second.asset);
    }
    
    m_CacheMisses++;
    return nullptr;
}

template<typename T>
std::shared_ptr<T> AssetsManager::GetOrLoadAsset(const std::string& path, const std::string& variant, const std::any& params) {
    auto asset = GetAsset<T>(path, variant);
    if (asset) {
        return asset;
    }
    
    return LoadAsset<T>(path, variant, params);
}

template<typename T>
void AssetsManager::PreloadAsset(const std::string& path, const std::string& variant, const std::any& params) {
    if (!m_Config.enablePreloading) {
        return;
    }
    
    // TODO: Implementar carregamento assíncrono se habilitado
    auto asset = LoadAsset<T>(path, variant, params);
    if (asset) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        AssetKey key(path, std::type_index(typeid(T)), variant);
        auto it = m_Assets.find(key);
        if (it != m_Assets.end()) {
            it->second.isPreloaded = true;
        }
    }
}

} // namespace Drift::Core