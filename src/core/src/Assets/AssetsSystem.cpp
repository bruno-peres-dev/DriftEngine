#include "Drift/Core/Assets/AssetsSystem.h"
#include <algorithm>
#include <filesystem>
#include <thread>

namespace Drift::Core::Assets {

AssetsSystem& AssetsSystem::GetInstance() {
    static AssetsSystem instance;
    return instance;
}

void AssetsSystem::Initialize(const AssetsConfig& config) {
    if (m_Initialized) {
        LOG_WARNING("[AssetsSystem] Sistema já inicializado");
        return;
    }
    
    m_Config = config;
    m_Initialized = true;
    
    LOG_INFO("[AssetsSystem] Sistema inicializado");
    DRIFT_LOG_INFO("[AssetsSystem] - Max Assets: ", m_Config.maxAssets);
    DRIFT_LOG_INFO("[AssetsSystem] - Max Memory: ", m_Config.maxMemoryUsage / (1024 * 1024), " MB");
    DRIFT_LOG_INFO("[AssetsSystem] - Async Loading: ", m_Config.enableAsyncLoading ? "Enabled" : "Disabled");
    DRIFT_LOG_INFO("[AssetsSystem] - Preloading: ", m_Config.enablePreloading ? "Enabled" : "Disabled");
}

void AssetsSystem::Shutdown() {
    if (!m_Initialized) return;
    
    LOG_INFO("[AssetsSystem] Finalizando sistema...");
    
    // Aguarda todos os carregamentos terminarem
    WaitForAllLoads();
    
    // Limpa o cache
    ClearCache();
    
    // Limpa loaders
    m_Loaders.clear();
    
    m_Initialized = false;
    LOG_INFO("[AssetsSystem] Sistema finalizado");
}

void AssetsSystem::SetConfig(const AssetsConfig& config) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Config = config;
    
    // Aplica novos limites
    if (m_Assets.size() > m_Config.maxAssets) {
        while (m_Assets.size() > m_Config.maxAssets) {
            if (!EvictLeastUsedAsset()) {
                break;
            }
        }
    }
    
    if (CalculateCurrentMemoryUsage() > m_Config.maxMemoryUsage) {
        while (CalculateCurrentMemoryUsage() > m_Config.maxMemoryUsage) {
            if (!EvictLeastUsedAsset()) {
                break;
            }
        }
    }
    
    LOG_INFO("[AssetsSystem] Configuração atualizada");
}

void AssetsSystem::PreloadAssets(const std::vector<std::string>& paths) {
    if (!m_Config.enablePreloading) {
        return;
    }
    
    DRIFT_LOG_INFO("[AssetsSystem] Pré-carregando ", paths.size(), " assets...");
    
    for (const auto& path : paths) {
        // Tenta determinar o tipo do asset pela extensão
        std::filesystem::path fsPath(path);
        std::string extension = fsPath.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // Por enquanto, logamos que o asset seria pré-carregado
        DRIFT_LOG_INFO("[AssetsSystem] Asset para pré-carregamento: ", path, " (extensão: ", extension, ")");
    }
}

void AssetsSystem::UnloadAsset(const std::string& path, std::type_index type, const std::string& variant) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, type, variant);
    auto it = m_Assets.find(key);
    
    if (it != m_Assets.end()) {
        if (it->second.isAsyncLoading) {
            while (it->second.status == AssetStatus::Loading) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        
        it->second.asset->Unload();
        m_Assets.erase(it);
        m_UnloadCount++;
        
        TriggerAssetUnloadedCallback(path, type);
        DRIFT_LOG_INFO("[AssetsSystem] Asset descarregado: ", path);
    }
}

void AssetsSystem::UnloadAssets(std::type_index type) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Assets.begin();
    size_t unloadedCount = 0;
    
    while (it != m_Assets.end()) {
        if (it->first.type == type) {
            if (it->second.isAsyncLoading) {
                while (it->second.status == AssetStatus::Loading) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            
            it->second.asset->Unload();
            TriggerAssetUnloadedCallback(it->first.path, type);
            it = m_Assets.erase(it);
            unloadedCount++;
            m_UnloadCount++;
        } else {
            ++it;
        }
    }
    
    DRIFT_LOG_INFO("[AssetsSystem] ", unloadedCount, " assets do tipo descarregados");
}

void AssetsSystem::UnloadUnusedAssets() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Assets.begin();
    size_t unloadedCount = 0;
    
    while (it != m_Assets.end()) {
        // Asset é considerado não usado se só tem 1 referência (a do cache)
        if (it->second.asset.use_count() == 1) {
            if (it->second.isAsyncLoading) {
                while (it->second.status == AssetStatus::Loading) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            
            it->second.asset->Unload();
            TriggerAssetUnloadedCallback(it->first.path, it->first.type);
            it = m_Assets.erase(it);
            unloadedCount++;
            m_UnloadCount++;
        } else {
            ++it;
        }
    }
    
    DRIFT_LOG_INFO("[AssetsSystem] ", unloadedCount, " assets não utilizados descarregados");
}

void AssetsSystem::ClearCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t totalAssets = m_Assets.size();
    
    for (auto& [key, entry] : m_Assets) {
        if (entry.isAsyncLoading) {
            while (entry.status == AssetStatus::Loading) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        
        entry.asset->Unload();
        TriggerAssetUnloadedCallback(key.path, key.type);
    }
    
    m_Assets.clear();
    m_UnloadCount += totalAssets;
    
    DRIFT_LOG_INFO("[AssetsSystem] Cache limpo - ", totalAssets, " assets descarregados");
}

void AssetsSystem::TrimCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t currentMemory = CalculateCurrentMemoryUsage();
    size_t targetMemory = static_cast<size_t>(m_Config.maxMemoryUsage * m_Config.trimThreshold);
    
    if (currentMemory <= targetMemory) {
        return; // Não precisa fazer trim
    }
    
    size_t initialCount = m_Assets.size();
    
    // Remove assets menos usados até atingir o threshold
    while (currentMemory > targetMemory && !m_Assets.empty()) {
        if (!EvictLeastUsedAsset()) {
            break;
        }
        currentMemory = CalculateCurrentMemoryUsage();
    }
    
    size_t removedCount = initialCount - m_Assets.size();
    DRIFT_LOG_INFO("[AssetsSystem] Cache trimmed - ", removedCount, " assets removidos");
}

AssetsStats AssetsSystem::GetStats() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetsStats stats;
    stats.totalAssets = m_Assets.size();
    stats.memoryUsage = CalculateCurrentMemoryUsage();
    stats.maxMemoryUsage = m_Config.maxMemoryUsage;
    stats.cacheHits = m_CacheHits;
    stats.cacheMisses = m_CacheMisses;
    stats.loadCount = m_LoadCount;
    stats.unloadCount = m_UnloadCount;
    stats.asyncLoadCount = m_AsyncLoadCount;
    stats.averageLoadTime = m_LoadCount > 0 ? m_TotalLoadTime / m_LoadCount : 0.0;
    
    // Calcula estatísticas por tipo e status
    for (const auto& [key, entry] : m_Assets) {
        stats.assetsByType[key.type]++;
        stats.memoryByType[key.type] += entry.memoryUsage;
        stats.loadCountByType[key.type]++;
        
        switch (entry.status) {
            case AssetStatus::Loaded:
                stats.loadedAssets++;
                break;
            case AssetStatus::Loading:
                stats.loadingAssets++;
                break;
            case AssetStatus::Failed:
                stats.failedAssets++;
                break;
            default:
                break;
        }
    }
    
    return stats;
}

void AssetsSystem::LogStats() const {
    auto stats = GetStats();
    
    LOG_INFO("[AssetsSystem] === Estatísticas do Sistema ===");
    DRIFT_LOG_INFO("[AssetsSystem] Total de Assets: ", stats.totalAssets);
    DRIFT_LOG_INFO("[AssetsSystem] Assets Carregados: ", stats.loadedAssets);
    DRIFT_LOG_INFO("[AssetsSystem] Assets Carregando: ", stats.loadingAssets);
    DRIFT_LOG_INFO("[AssetsSystem] Assets Falharam: ", stats.failedAssets);
    DRIFT_LOG_INFO("[AssetsSystem] Uso de Memória: ", stats.memoryUsage / (1024 * 1024), " MB / ", stats.maxMemoryUsage / (1024 * 1024), " MB");
    DRIFT_LOG_INFO("[AssetsSystem] Cache Hits: ", stats.cacheHits);
    DRIFT_LOG_INFO("[AssetsSystem] Cache Misses: ", stats.cacheMisses);
    DRIFT_LOG_INFO("[AssetsSystem] Carregamentos: ", stats.loadCount);
    DRIFT_LOG_INFO("[AssetsSystem] Carregamentos Assíncronos: ", stats.asyncLoadCount);
    DRIFT_LOG_INFO("[AssetsSystem] Descarregamentos: ", stats.unloadCount);
    DRIFT_LOG_INFO("[AssetsSystem] Tempo Médio de Carregamento: ", std::fixed, std::setprecision(2), stats.averageLoadTime * 1000.0, " ms");
    
    if (!stats.assetsByType.empty()) {
        LOG_INFO("[AssetsSystem] === Assets por Tipo ===");
        for (const auto& [type, count] : stats.assetsByType) {
            size_t memory = stats.memoryByType.at(type);
            size_t loads = stats.loadCountByType.at(type);
            DRIFT_LOG_INFO("[AssetsSystem] ", std::string(type.name()), ": ", count, " assets, ", memory / (1024 * 1024), " MB, ", loads, " carregamentos");
        }
    }
    
    DRIFT_LOG_INFO("[AssetsSystem] ================================");
}

void AssetsSystem::ResetStats() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CacheHits = 0;
    m_CacheMisses = 0;
    m_LoadCount = 0;
    m_UnloadCount = 0;
    m_AsyncLoadCount = 0;
    m_TotalLoadTime = 0.0;
    LOG_INFO("[AssetsSystem] Estatísticas resetadas");
}

bool AssetsSystem::IsAssetLoaded(const std::string& path, std::type_index type, const std::string& variant) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, type, variant);
    auto it = m_Assets.find(key);
    
    return it != m_Assets.end() && it->second.status == AssetStatus::Loaded;
}

bool AssetsSystem::IsAssetLoading(const std::string& path, std::type_index type, const std::string& variant) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, type, variant);
    auto it = m_Assets.find(key);
    
    return it != m_Assets.end() && it->second.status == AssetStatus::Loading;
}

AssetStatus AssetsSystem::GetAssetStatus(const std::string& path, std::type_index type, const std::string& variant) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, type, variant);
    auto it = m_Assets.find(key);
    
    if (it != m_Assets.end()) {
        return it->second.status;
    }
    
    return AssetStatus::NotLoaded;
}

bool AssetsSystem::CanLoadAsset(const std::string& path, std::type_index type) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Loaders.find(type);
    if (it == m_Loaders.end()) {
        return false;
    }
    
    // Por enquanto, assume que pode carregar
    // Em uma implementação completa, seria necessário acessar o loader
    return true;
}

std::vector<std::string> AssetsSystem::GetSupportedExtensions(std::type_index type) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Loaders.find(type);
    if (it == m_Loaders.end()) {
        return {};
    }
    
    // Por enquanto, retorna vazio
    // Em uma implementação completa, seria necessário acessar o loader
    return {};
}

void AssetsSystem::WaitForAllLoads() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    for (auto& [key, entry] : m_Assets) {
        if (entry.isAsyncLoading) {
            while (entry.status == AssetStatus::Loading) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
    
    DRIFT_LOG_INFO("[AssetsSystem] Aguardou todos os carregamentos");
}

void AssetsSystem::CancelAllLoads() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t cancelledCount = 0;
    for (auto& [key, entry] : m_Assets) {
        if (entry.status == AssetStatus::Loading) {
            entry.status = AssetStatus::Failed;
            entry.errorMessage = "Carregamento cancelado";
            cancelledCount++;
        }
    }
    
    DRIFT_LOG_INFO("[AssetsSystem] Cancelou ", cancelledCount, " carregamentos");
}

size_t AssetsSystem::GetLoadingCount() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t count = 0;
    for (const auto& [key, entry] : m_Assets) {
        if (entry.status == AssetStatus::Loading) {
            count++;
        }
    }
    
    return count;
}

size_t AssetsSystem::GetQueuedCount() const {
    // Por enquanto, retorna 0
    // Em uma implementação completa, seria necessário rastrear a fila de carregamento
    return 0;
}

bool AssetsSystem::EvictLeastUsedAsset() {
    if (m_Assets.empty()) {
        return false;
    }
    
    // Encontra o asset menos usado (LRU)
    auto leastUsed = std::min_element(m_Assets.begin(), m_Assets.end(),
        [](const auto& a, const auto& b) {
            // Prioriza assets com menor contagem de acesso e mais antigos
            if (a.second.accessCount != b.second.accessCount) {
                return a.second.accessCount < b.second.accessCount;
            }
            return a.second.lastAccess < b.second.lastAccess;
        });
    
    if (leastUsed != m_Assets.end()) {
        if (leastUsed->second.isAsyncLoading) {
            while (leastUsed->second.status == AssetStatus::Loading) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        
        leastUsed->second.asset->Unload();
        TriggerAssetUnloadedCallback(leastUsed->first.path, leastUsed->first.type);
        m_Assets.erase(leastUsed);
        m_UnloadCount++;
        return true;
    }
    
    return false;
}

void AssetsSystem::UpdateAccessStats(AssetCacheEntry& entry) {
    entry.lastAccess = ++m_AccessCounter;
    entry.accessCount++;
}

size_t AssetsSystem::CalculateCurrentMemoryUsage() const {
    size_t totalMemory = 0;
    for (const auto& [key, entry] : m_Assets) {
        totalMemory += entry.memoryUsage;
    }
    return totalMemory;
}

void AssetsSystem::TriggerAssetLoadedCallback(const std::string& path, std::type_index type) {
    if (m_AssetLoadedCallback) {
        m_AssetLoadedCallback(path, type);
    }
}

void AssetsSystem::TriggerAssetUnloadedCallback(const std::string& path, std::type_index type) {
    if (m_AssetUnloadedCallback) {
        m_AssetUnloadedCallback(path, type);
    }
}

void AssetsSystem::TriggerAssetFailedCallback(const std::string& path, std::type_index type, const std::string& error) {
    if (m_AssetFailedCallback) {
        m_AssetFailedCallback(path, type, error);
    }
}

void AssetsSystem::ProcessAsyncLoads() {
    // Esta função seria chamada periodicamente para processar carregamentos assíncronos
    // Por enquanto, é uma implementação vazia
}

void AssetsSystem::CleanupCompletedLoads() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    for (auto& [key, entry] : m_Assets) {
        if (entry.isAsyncLoading && entry.status != AssetStatus::Loading) {
            entry.isAsyncLoading = false;
        }
    }
}

} // namespace Drift::Core::Assets 