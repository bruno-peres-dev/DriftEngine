#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace Drift::UI;

FontManager& FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

const std::string& FontManager::GetDefaultFontName() const {
    return m_DefaultFontName;
}

size_t FontManager::GetCurrentTime() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

void FontManager::UpdateCacheStats(bool hit) {
    if (hit) {
        m_CacheHits++;
    } else {
        m_CacheMisses++;
    }
}

std::shared_ptr<FontManager::TTFData> FontManager::LoadTTFData(const std::string& path) {
    auto it = m_TTFDataCache.find(path);
    if (it != m_TTFDataCache.end()) {
        it->second->refCount++;
        it->second->lastUsed = GetCurrentTime();
        return it->second;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        Drift::Core::LogError("[FontManager] Não foi possível abrir arquivo TTF: " + path);
        return nullptr;
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    auto ttfData = std::make_shared<TTFData>();
    ttfData->data.resize(fileSize);
    ttfData->path = path;
    ttfData->refCount = 1;
    ttfData->lastUsed = GetCurrentTime();
    
    file.read(reinterpret_cast<char*>(ttfData->data.data()), fileSize);
    file.close();
    
    m_TTFDataCache[path] = ttfData;
    
    return ttfData;
}

void FontManager::ReleaseTTFData(const std::string& path) {
    auto it = m_TTFDataCache.find(path);
    if (it != m_TTFDataCache.end()) {
        it->second->refCount--;
        if (it->second->refCount == 0) {
            m_TTFDataCache.erase(it);
        }
    }
}

void FontManager::PreloadFontFile(const std::string& path) {
    if (std::filesystem::exists(path)) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        LoadTTFData(path);
        Drift::Core::LogRHI("[FontManager] Fonte pré-carregada: " + path);
    } else {
        Drift::Core::LogWarning("[FontManager] Arquivo de fonte não encontrado: " + path);
    }
}

void FontManager::ClearCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Fonts.clear();
    m_TTFDataCache.clear();
    m_CacheHits = 0;
    m_CacheMisses = 0;
    Drift::Core::LogRHI("[FontManager] Cache limpo");
}

size_t FontManager::GetCacheSize() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Fonts.size();
}

void FontManager::TrimCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    size_t currentTime = GetCurrentTime();
    
    // Limpar cache de fontes se exceder o limite
    if (m_Fonts.size() > m_CacheConfig.maxFonts) {
        std::vector<std::pair<FontKey, FontCacheEntry>> entries;
        entries.reserve(m_Fonts.size());
        
        for (auto& [key, entry] : m_Fonts) {
            entries.emplace_back(key, entry);
        }
        
        // Ordenar por último uso (LRU)
        std::sort(entries.begin(), entries.end(), 
            [](const auto& a, const auto& b) {
                return a.second.lastUsed < b.second.lastUsed;
            });
        
        // Remover as entradas mais antigas
        size_t toRemove = m_Fonts.size() - m_CacheConfig.maxFonts;
        for (size_t i = 0; i < toRemove; ++i) {
            m_Fonts.erase(entries[i].first);
        }
    }
    
    // Limpar cache TTF se exceder o limite
    if (m_TTFDataCache.size() > m_CacheConfig.maxTTFData) {
        std::vector<std::pair<std::string, std::shared_ptr<TTFData>>> ttfEntries;
        ttfEntries.reserve(m_TTFDataCache.size());
        
        for (auto& [path, ttfData] : m_TTFDataCache) {
            ttfEntries.emplace_back(path, ttfData);
        }
        
        // Ordenar por último uso (LRU)
        std::sort(ttfEntries.begin(), ttfEntries.end(), 
            [](const auto& a, const auto& b) {
                return a.second->lastUsed < b.second->lastUsed;
            });
        
        // Remover as entradas mais antigas
        size_t toRemove = m_TTFDataCache.size() - m_CacheConfig.maxTTFData;
        for (size_t i = 0; i < toRemove; ++i) {
            m_TTFDataCache.erase(ttfEntries[i].first);
        }
    }
    
    m_LastTrimTime = currentTime;
}

FontManager::CacheStats FontManager::GetCacheStats() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    CacheStats stats;
    stats.fontCount = m_Fonts.size();
    stats.ttfDataCount = m_TTFDataCache.size();
    stats.cacheHits = m_CacheHits;
    stats.cacheMisses = m_CacheMisses;
    
    // Calcular uso de memória aproximado
    for (const auto& [key, entry] : m_Fonts) {
        if (entry.font) {
            stats.totalMemoryUsage += sizeof(Font);
            // Adicionar tamanho aproximado dos glyphs
            stats.totalMemoryUsage += 96 * sizeof(GlyphInfo); // ASCII glyphs
        }
    }
    
    for (const auto& [path, ttfData] : m_TTFDataCache) {
        stats.totalMemoryUsage += ttfData->data.size();
    }
    
    return stats;
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& name, const std::string& path, float size, FontQuality quality) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (!m_Device) {
        Drift::Core::LogError("[FontManager] Device não configurado");
        return nullptr;
    }

    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        UpdateCacheStats(true);
        it->second.lastUsed = GetCurrentTime();
        it->second.accessCount++;
        return it->second.font;
    }

    UpdateCacheStats(false);
    
    auto ttfData = LoadTTFData(path);
    if (!ttfData) {
        Drift::Core::LogError("[FontManager] Falha ao carregar dados TTF: " + path);
        return nullptr;
    }

    auto font = std::make_shared<Font>(name, size, quality);
    
    try {
        if (font->LoadFromMemory(ttfData->data.data(), ttfData->data.size(), m_Device)) {
            FontCacheEntry entry;
            entry.font = font;
            entry.lastUsed = GetCurrentTime();
            entry.accessCount = 1;
            
            m_Fonts[key] = entry;
            
            // Verificar se precisa fazer trim do cache
            if (m_Fonts.size() > m_CacheConfig.maxFonts || 
                m_TTFDataCache.size() > m_CacheConfig.maxTTFData) {
                TrimCache();
            }
            
            return font;
        } else {
            Drift::Core::LogError("[FontManager] Falha ao carregar fonte: " + name);
            ReleaseTTFData(path);
            return nullptr;
        }
    } catch (const std::exception& e) {
        Drift::Core::LogException("[FontManager] Exceção ao carregar fonte", e);
        ReleaseTTFData(path);
        return nullptr;
    }
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        UpdateCacheStats(true);
        it->second.lastUsed = GetCurrentTime();
        it->second.accessCount++;
        return it->second.font;
    }
    UpdateCacheStats(false);
    return nullptr;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size) {
    return GetFont(name, size, m_DefaultQuality);
}

std::shared_ptr<Font> FontManager::GetOrLoadFont(const std::string& name, const std::string& path, float size, FontQuality quality) {
    auto font = GetFont(name, size, quality);
    if (font) {
        return font;
    }
    
    if (m_CacheConfig.enableLazyLoading) {
        return LoadFont(name, path, size, quality);
    }
    
    return nullptr;
}

void FontManager::PreloadCommonSizes(const std::string& name, const std::string& path, const std::vector<float>& sizes) {
    if (!m_CacheConfig.enablePreloading) {
        return;
    }
    
    Drift::Core::LogRHI("[FontManager] Pré-carregando " + std::to_string(sizes.size()) + " tamanhos para fonte: " + name);
    
    for (float size : sizes) {
        try {
            auto font = LoadFont(name, path, size, m_DefaultQuality);
            if (!font) {
                Drift::Core::LogError("[FontManager] Erro ao carregar tamanho " + std::to_string(size));
            }
        } catch (const std::exception& e) {
            Drift::Core::LogError("[FontManager] Exceção ao carregar tamanho " + std::to_string(size) + ": " + e.what());
        }
    }
}

