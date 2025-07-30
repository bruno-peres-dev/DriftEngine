#include "Drift/Core/AssetsManager.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <filesystem>

namespace Drift::Core {

AssetsManager& AssetsManager::GetInstance() {
    static AssetsManager instance;
    return instance;
}

void AssetsManager::SetCacheConfig(const AssetCacheConfig& config) {
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
}

void AssetsManager::PreloadAssets(const std::vector<std::string>& paths) {
    if (!m_Config.enablePreloading) {
        return;
    }
    
    Log("[AssetsManager] Pré-carregando " + std::to_string(paths.size()) + " assets...");
    
    // TODO: Implementar carregamento assíncrono se habilitado
    for (const auto& path : paths) {
        // Tentar determinar o tipo do asset pela extensão
        std::filesystem::path fsPath(path);
        std::string extension = fsPath.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // Por enquanto, logamos que o asset seria pré-carregado
        // Em uma implementação completa, seria necessário ter um registry de extensões -> tipos
        Log("[AssetsManager] Asset para pré-carregamento: " + path + " (extensão: " + extension + ")");
    }
}

void AssetsManager::UnloadAsset(const std::string& path, std::type_index type, const std::string& variant) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, type, variant);
    auto it = m_Assets.find(key);
    
    if (it != m_Assets.end()) {
        it->second.asset->Unload();
        m_Assets.erase(it);
        m_UnloadCount++;
        
        TriggerAssetUnloadedCallback(path, type);
        Log("[AssetsManager] Asset descarregado: " + path);
    }
}

void AssetsManager::UnloadAssets(std::type_index type) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Assets.begin();
    size_t unloadedCount = 0;
    
    while (it != m_Assets.end()) {
        if (it->first.type == type) {
            it->second.asset->Unload();
            TriggerAssetUnloadedCallback(it->first.path, type);
            it = m_Assets.erase(it);
            unloadedCount++;
            m_UnloadCount++;
        } else {
            ++it;
        }
    }
    
    Log("[AssetsManager] " + std::to_string(unloadedCount) + " assets do tipo descarregados");
}

void AssetsManager::UnloadUnusedAssets() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Assets.begin();
    size_t unloadedCount = 0;
    
    while (it != m_Assets.end()) {
        // Asset é considerado não usado se só tem 1 referência (a do cache)
        if (it->second.asset.use_count() == 1) {
            it->second.asset->Unload();
            TriggerAssetUnloadedCallback(it->first.path, it->first.type);
            it = m_Assets.erase(it);
            unloadedCount++;
            m_UnloadCount++;
        } else {
            ++it;
        }
    }
    
    Log("[AssetsManager] " + std::to_string(unloadedCount) + " assets não utilizados descarregados");
}

void AssetsManager::ClearCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t totalAssets = m_Assets.size();
    
    for (auto& [key, entry] : m_Assets) {
        entry.asset->Unload();
        TriggerAssetUnloadedCallback(key.path, key.type);
    }
    
    m_Assets.clear();
    m_UnloadCount += totalAssets;
    
    Log("[AssetsManager] Cache limpo - " + std::to_string(totalAssets) + " assets descarregados");
}

void AssetsManager::TrimCache() {
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
    Log("[AssetsManager] Cache trimmed - " + std::to_string(removedCount) + " assets removidos");
}

AssetCacheStats AssetsManager::GetCacheStats() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetCacheStats stats;
    stats.totalAssets = m_Assets.size();
    stats.memoryUsage = CalculateCurrentMemoryUsage();
    stats.maxMemoryUsage = m_Config.maxMemoryUsage;
    stats.cacheHits = m_CacheHits;
    stats.cacheMisses = m_CacheMisses;
    stats.loadCount = m_LoadCount;
    stats.unloadCount = m_UnloadCount;
    stats.averageLoadTime = m_LoadCount > 0 ? m_TotalLoadTime / m_LoadCount : 0.0;
    
    // Calcula estatísticas por tipo
    for (const auto& [key, entry] : m_Assets) {
        stats.assetsByType[key.type]++;
        stats.memoryByType[key.type] += entry.memoryUsage;
        
        if (entry.asset->IsLoaded()) {
            stats.loadedAssets++;
        }
    }
    
    return stats;
}

void AssetsManager::LogCacheStats() const {
    auto stats = GetCacheStats();
    
    Log("[AssetsManager] === Estatísticas do Cache ===");
    Log("[AssetsManager] Total de Assets: " + std::to_string(stats.totalAssets));
    Log("[AssetsManager] Assets Carregados: " + std::to_string(stats.loadedAssets));
    Log("[AssetsManager] Uso de Memória: " + std::to_string(stats.memoryUsage / (1024 * 1024)) + " MB / " + 
        std::to_string(stats.maxMemoryUsage / (1024 * 1024)) + " MB");
    Log("[AssetsManager] Cache Hits: " + std::to_string(stats.cacheHits));
    Log("[AssetsManager] Cache Misses: " + std::to_string(stats.cacheMisses));
    Log("[AssetsManager] Carregamentos: " + std::to_string(stats.loadCount));
    Log("[AssetsManager] Descarregamentos: " + std::to_string(stats.unloadCount));
    Log("[AssetsManager] Tempo Médio de Carregamento: " + std::to_string(stats.averageLoadTime * 1000.0) + " ms");
    
    if (!stats.assetsByType.empty()) {
        Log("[AssetsManager] === Assets por Tipo ===");
        for (const auto& [type, count] : stats.assetsByType) {
            size_t memory = stats.memoryByType.at(type);
            Log("[AssetsManager] " + std::string(type.name()) + ": " + 
                std::to_string(count) + " assets, " + 
                std::to_string(memory / (1024 * 1024)) + " MB");
        }
    }
}

bool AssetsManager::IsAssetLoaded(const std::string& path, std::type_index type, const std::string& variant) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AssetKey key(path, type, variant);
    auto it = m_Assets.find(key);
    
    return it != m_Assets.end() && it->second.asset->IsLoaded();
}

bool AssetsManager::CanLoadAsset(const std::string& path, std::type_index type) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Loaders.find(type);
    if (it == m_Loaders.end()) {
        return false;
    }
    
    // Não podemos chamar CanLoad diretamente aqui devido ao std::any
    // Seria necessário uma abordagem diferente para isso
    return true; // Por enquanto, assume que pode carregar
}

std::vector<std::string> AssetsManager::GetSupportedExtensions(std::type_index type) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Loaders.find(type);
    if (it == m_Loaders.end()) {
        return {};
    }
    
    // Similar ao CanLoadAsset, seria necessário uma abordagem diferente
    // para acessar os métodos do loader através do std::any
    return {}; // Por enquanto, retorna vazio
}

bool AssetsManager::EvictLeastUsedAsset() {
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
        leastUsed->second.asset->Unload();
        TriggerAssetUnloadedCallback(leastUsed->first.path, leastUsed->first.type);
        m_Assets.erase(leastUsed);
        m_UnloadCount++;
        return true;
    }
    
    return false;
}

void AssetsManager::UpdateAccessStats(AssetCacheEntry& entry) {
    entry.lastAccess = ++m_AccessCounter;
    entry.accessCount++;
}

size_t AssetsManager::CalculateCurrentMemoryUsage() const {
    size_t totalMemory = 0;
    for (const auto& [key, entry] : m_Assets) {
        totalMemory += entry.memoryUsage;
    }
    return totalMemory;
}

void AssetsManager::TriggerAssetLoadedCallback(const std::string& path, std::type_index type) {
    if (m_AssetLoadedCallback) {
        m_AssetLoadedCallback(path, type);
    }
}

void AssetsManager::TriggerAssetUnloadedCallback(const std::string& path, std::type_index type) {
    if (m_AssetUnloadedCallback) {
        m_AssetUnloadedCallback(path, type);
    }
}

} // namespace Drift::Core