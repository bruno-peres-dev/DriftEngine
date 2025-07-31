#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/Font.h"
#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Profiler.h"
#include "Drift/Core/Assets/AssetsSystem.h"
#include <algorithm>
#include <future>
#include <filesystem>

namespace Drift::UI {

// Singleton instance
static FontManager* s_Instance = nullptr;

FontManager& FontManager::GetInstance() {
    if (!s_Instance) {
        s_Instance = new FontManager();
    }
    return *s_Instance;
}

void FontManager::Initialize(const FontSystemConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (m_Initialized) {
        DRIFT_LOG_WARNING("FontManager já foi inicializado");
        return;
    }
    
    DRIFT_LOG_INFO("Inicializando FontManager...");
    
    m_Config = config;
    m_Initialized = true;
    
    // Registrar loader no AssetsSystem
    RegisterFontLoader();
    
    DRIFT_LOG_INFO("FontManager inicializado com sucesso");
}

void FontManager::Shutdown() {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Initialized) {
        return;
    }
    
    DRIFT_LOG_INFO("Finalizando FontManager...");
    
    // Limpar cache
    ClearCache();
    
    // Desregistrar loader
    UnregisterFontLoader();
    
    m_Initialized = false;
    DRIFT_LOG_INFO("FontManager finalizado");
}

void FontManager::SetConfig(const FontSystemConfig& config) {
    m_Config = config;
    
    // Aplicar novas configurações
    if (m_Config.maxFonts < m_Fonts.size()) {
        TrimCache();
    }
}

void FontManager::SetDevice(Drift::RHI::IDevice* device) {
    m_Device = device;
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& path, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Initialized) {
        DRIFT_LOG_ERROR("FontManager não foi inicializado");
        return nullptr;
    }
    
    // Verificar se já está no cache
    FontKey key{path, config.size, config.quality, config.format};
    
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Fonts.find(key);
        if (it != m_Fonts.end()) {
            UpdateCacheStats(true);
            UpdateAccessStats(it->second);
            return it->second.font;
        }
    }
    
    UpdateCacheStats(false);
    
    // Criar nova fonte
    auto font = CreateFont(path, config);
    if (!font) {
        return nullptr;
    }
    
    // Adicionar ao cache
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        // Verificar se ainda há espaço
        if (m_Fonts.size() >= m_Config.maxFonts) {
            EvictLeastUsedFont();
        }
        
        FontCacheEntry entry;
        entry.font = font;
        entry.lastUsed = GetCurrentTime();
        entry.accessCount = 1;
        entry.loadTime = std::chrono::steady_clock::now();
        
        m_Fonts[key] = entry;
    }
    
    return font;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    DRIFT_PROFILE_FUNCTION();
    
    FontLoadConfig config;
    config.size = size;
    config.quality = quality;
    
    // Tentar encontrar no cache por nome
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (auto& pair : m_Fonts) {
            if (pair.second.font->GetName() == name && 
                pair.second.font->GetSize() == size &&
                pair.second.font->GetQuality() == quality) {
                UpdateCacheStats(true);
                UpdateAccessStats(pair.second);
                return pair.second.font;
            }
        }
    }
    
    // Tentar carregar do caminho padrão
    std::string path = m_DefaultFontPath;
    if (name != "default") {
        path = "fonts/" + name + ".ttf";
    }
    
    return LoadFont(path, config);
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size) {
    return GetFont(name, size, m_Config.defaultQuality);
}

std::shared_ptr<Font> FontManager::GetOrLoadFont(const std::string& path, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    // Primeiro tentar obter do cache
    FontKey key{path, config.size, config.quality, config.format};
    
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Fonts.find(key);
        if (it != m_Fonts.end()) {
            UpdateCacheStats(true);
            UpdateAccessStats(it->second);
            return it->second.font;
        }
    }
    
    // Se não estiver no cache, carregar
    return LoadFont(path, config);
}

std::shared_ptr<Font> FontManager::LoadFontAsset(const std::string& assetPath, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    // Usar AssetsSystem para carregar
    auto asset = Drift::Core::Assets::AssetsSystem::GetInstance().LoadAsset(assetPath);
    if (!asset) {
        DRIFT_LOG_ERROR("Falha ao carregar asset de fonte: {}", assetPath);
        return nullptr;
    }
    
    // Criar fonte a partir do asset
    return CreateFontAsset(assetPath, config);
}

std::shared_ptr<Font> FontManager::GetFontAsset(const std::string& assetPath, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    // Verificar cache primeiro
    FontKey key{assetPath, config.size, config.quality, config.format};
    
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Fonts.find(key);
        if (it != m_Fonts.end()) {
            UpdateCacheStats(true);
            UpdateAccessStats(it->second);
            return it->second.font;
        }
    }
    
    return LoadFontAsset(assetPath, config);
}

std::future<std::shared_ptr<Font>> FontManager::LoadFontAsync(const std::string& path, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Config.enableAsyncLoading) {
        // Carregamento síncrono
        auto font = LoadFont(path, config);
        return std::async(std::launch::deferred, [font]() { return font; });
    }
    
    return std::async(std::launch::async, [this, path, config]() {
        return LoadFont(path, config);
    });
}

std::future<std::shared_ptr<Font>> FontManager::LoadFontAssetAsync(const std::string& assetPath, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Config.enableAsyncLoading) {
        auto font = LoadFontAsset(assetPath, config);
        return std::async(std::launch::deferred, [font]() { return font; });
    }
    
    return std::async(std::launch::async, [this, assetPath, config]() {
        return LoadFontAsset(assetPath, config);
    });
}

void FontManager::PreloadFont(const std::string& path, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Config.enablePreloading) {
        return;
    }
    
    // Verificar se já está carregado
    FontKey key{path, config.size, config.quality, config.format};
    
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Fonts.find(key) != m_Fonts.end()) {
            return; // Já está carregado
        }
    }
    
    // Carregar em background
    std::async(std::launch::async, [this, path, config]() {
        auto font = LoadFont(path, config);
        if (font) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            FontKey key{path, config.size, config.quality, config.format};
            auto it = m_Fonts.find(key);
            if (it != m_Fonts.end()) {
                it->second.isPreloaded = true;
            }
        }
    });
}

void FontManager::PreloadFontAsset(const std::string& assetPath, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Config.enablePreloading) {
        return;
    }
    
    // Similar ao PreloadFont, mas para assets
    FontKey key{assetPath, config.size, config.quality, config.format};
    
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Fonts.find(key) != m_Fonts.end()) {
            return;
        }
    }
    
    std::async(std::launch::async, [this, assetPath, config]() {
        auto font = LoadFontAsset(assetPath, config);
        if (font) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            FontKey key{assetPath, config.size, config.quality, config.format};
            auto it = m_Fonts.find(key);
            if (it != m_Fonts.end()) {
                it->second.isPreloaded = true;
            }
        }
    });
}

void FontManager::PreloadCommonSizes(const std::string& path, const std::vector<float>& sizes) {
    DRIFT_PROFILE_FUNCTION();
    
    for (float size : sizes) {
        FontLoadConfig config;
        config.size = size;
        config.quality = m_Config.defaultQuality;
        PreloadFont(path, config);
    }
}

void FontManager::PreloadCharSet(const std::string& path, const std::vector<uint32_t>& chars) {
    DRIFT_PROFILE_FUNCTION();
    
    FontLoadConfig config;
    config.preloadChars = chars;
    PreloadFont(path, config);
}

void FontManager::RegisterFallbackFont(const std::string& path, const std::string& name) {
    DRIFT_PROFILE_FUNCTION();
    
    FontLoadConfig config;
    config.size = 16.0f; // Tamanho padrão para fallback
    config.quality = FontQuality::Medium;
    
    auto font = LoadFont(path, config);
    if (font) {
        m_FallbackFonts[name] = font;
        DRIFT_LOG_INFO("Fonte de fallback registrada: {} -> {}", name, path);
    }
}

void FontManager::SetSystemFallbackFonts(const std::vector<std::string>& fonts) {
    DRIFT_PROFILE_FUNCTION();
    
    for (const auto& fontPath : fonts) {
        RegisterFallbackFont(fontPath, "system");
    }
}

std::shared_ptr<Font> FontManager::GetFallbackFont(uint32_t codepoint) {
    DRIFT_PROFILE_FUNCTION();
    
    m_FallbackUsage++;
    
    // Tentar encontrar a melhor fonte de fallback
    auto font = FindBestFallbackFont(codepoint);
    if (font) {
        return font;
    }
    
    // Usar primeira fonte de fallback disponível
    if (!m_FallbackFonts.empty()) {
        return m_FallbackFonts.begin()->second;
    }
    
    return nullptr;
}

std::shared_ptr<FontAtlas> FontManager::GetAtlas(const FontAtlasConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    return FontAtlasManager::GetInstance().GetAtlas(config);
}

void FontManager::OptimizeAtlas() {
    DRIFT_PROFILE_FUNCTION();
    
    FontAtlasManager::GetInstance().OptimizeAllAtlas();
}

void FontManager::ClearUnusedAtlas() {
    DRIFT_PROFILE_FUNCTION();
    
    FontAtlasManager::GetInstance().ClearUnusedAtlas();
}

void FontManager::ClearCache() {
    DRIFT_PROFILE_FUNCTION();
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Fonts.clear();
    m_FallbackFonts.clear();
    
    DRIFT_LOG_INFO("Cache de fontes limpo");
}

void FontManager::TrimCache() {
    DRIFT_PROFILE_FUNCTION();
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (m_Fonts.size() <= m_Config.maxFonts) {
        return;
    }
    
    // Converter para vector para ordenar
    std::vector<std::pair<FontKey, FontCacheEntry>> sortedFonts(m_Fonts.begin(), m_Fonts.end());
    
    // Ordenar por último uso e contagem de acesso
    std::sort(sortedFonts.begin(), sortedFonts.end(), 
              [](const auto& a, const auto& b) {
                  if (a.second.lastUsed != b.second.lastUsed) {
                      return a.second.lastUsed < b.second.lastUsed;
                  }
                  return a.second.accessCount < b.second.accessCount;
              });
    
    // Remover fontes menos usadas
    size_t toRemove = m_Fonts.size() - m_Config.maxFonts;
    for (size_t i = 0; i < toRemove; ++i) {
        m_Fonts.erase(sortedFonts[i].first);
    }
    
    DRIFT_LOG_INFO("Cache de fontes reduzido: {} fontes removidas", toRemove);
}

void FontManager::UnloadUnusedFonts() {
    DRIFT_PROFILE_FUNCTION();
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t currentTime = GetCurrentTime();
    size_t unloadedCount = 0;
    
    for (auto it = m_Fonts.begin(); it != m_Fonts.end();) {
        // Descarregar fontes não usadas há muito tempo
        if (currentTime - it->second.lastUsed > 300) { // 5 minutos
            it = m_Fonts.erase(it);
            unloadedCount++;
        } else {
            ++it;
        }
    }
    
    if (unloadedCount > 0) {
        DRIFT_LOG_INFO("{} fontes não utilizadas foram descarregadas", unloadedCount);
    }
}

size_t FontManager::GetCacheSize() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Fonts.size();
}

FontManager::FontStats FontManager::GetStats() const {
    DRIFT_PROFILE_FUNCTION();
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    FontStats stats;
    stats.totalFonts = m_Fonts.size();
    stats.cacheHits = m_CacheHits;
    stats.cacheMisses = m_CacheMisses;
    stats.fallbackUsage = m_FallbackUsage;
    
    // Calcular estatísticas por qualidade e formato
    for (const auto& pair : m_Fonts) {
        const auto& font = pair.second.font;
        stats.fontsByQuality[font->GetQuality()]++;
        stats.fontsByFormat[font->GetFormat()]++;
        
        if (font->IsLoaded()) {
            stats.loadedFonts++;
            stats.totalMemoryUsage += font->GetMemoryUsage();
        } else if (font->GetStatus() == Drift::Core::Assets::AssetStatus::Loading) {
            stats.loadingFonts++;
        } else if (font->GetStatus() == Drift::Core::Assets::AssetStatus::Failed) {
            stats.failedFonts++;
        }
    }
    
    // Calcular tempo médio de carregamento
    if (m_LoadCount > 0) {
        stats.averageLoadTime = m_TotalLoadTime / m_LoadCount;
    }
    
    // Estatísticas de atlas
    stats.totalAtlas = FontAtlasManager::GetInstance().GetAtlasCount();
    
    return stats;
}

void FontManager::LogStats() const {
    DRIFT_PROFILE_FUNCTION();
    
    auto stats = GetStats();
    
    DRIFT_LOG_INFO("=== Estatísticas do FontManager ===");
    DRIFT_LOG_INFO("Total de fontes: {}", stats.totalFonts);
    DRIFT_LOG_INFO("Fontes carregadas: {}", stats.loadedFonts);
    DRIFT_LOG_INFO("Fontes carregando: {}", stats.loadingFonts);
    DRIFT_LOG_INFO("Fontes com falha: {}", stats.failedFonts);
    DRIFT_LOG_INFO("Total de atlas: {}", stats.totalAtlas);
    DRIFT_LOG_INFO("Uso de memória: {} bytes", stats.totalMemoryUsage);
    DRIFT_LOG_INFO("Cache hits: {}", stats.cacheHits);
    DRIFT_LOG_INFO("Cache misses: {}", stats.cacheMisses);
    DRIFT_LOG_INFO("Uso de fallback: {}", stats.fallbackUsage);
    DRIFT_LOG_INFO("Tempo médio de carregamento: {:.2f}ms", stats.averageLoadTime);
    
    DRIFT_LOG_INFO("Fontes por qualidade:");
    for (const auto& pair : stats.fontsByQuality) {
        DRIFT_LOG_INFO("  {}: {}", static_cast<int>(pair.first), pair.second);
    }
    
    DRIFT_LOG_INFO("Fontes por formato:");
    for (const auto& pair : stats.fontsByFormat) {
        DRIFT_LOG_INFO("  {}: {}", static_cast<int>(pair.first), pair.second);
    }
}

void FontManager::ResetStats() {
    DRIFT_PROFILE_FUNCTION();
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CacheHits = 0;
    m_CacheMisses = 0;
    m_FallbackUsage = 0;
    m_LoadCount = 0;
    m_TotalLoadTime = 0.0;
}

// Métodos privados
std::shared_ptr<Font> FontManager::CreateFont(const std::string& path, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    auto font = std::make_shared<Font>(GetFontNameFromPath(path), config);
    
    if (font->LoadFromFile(path)) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_LoadCount++;
        m_TotalLoadTime += duration.count() / 1000.0; // Converter para ms
        
        return font;
    }
    
    return nullptr;
}

std::shared_ptr<Font> FontManager::CreateFontAsset(const std::string& assetPath, const FontLoadConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    auto font = std::make_shared<Font>(GetFontNameFromPath(assetPath), config);
    
    if (font->LoadFromAsset(assetPath)) {
        return font;
    }
    
    return nullptr;
}

void FontManager::UpdateCacheStats(bool hit) {
    if (hit) {
        m_CacheHits++;
    } else {
        m_CacheMisses++;
    }
}

void FontManager::UpdateAccessStats(FontCacheEntry& entry) {
    entry.lastUsed = GetCurrentTime();
    entry.accessCount++;
}

size_t FontManager::GetCurrentTime() const {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

bool FontManager::EvictLeastUsedFont() {
    DRIFT_PROFILE_FUNCTION();
    
    if (m_Fonts.empty()) {
        return false;
    }
    
    // Encontrar fonte menos usada
    auto leastUsed = m_Fonts.begin();
    for (auto it = m_Fonts.begin(); it != m_Fonts.end(); ++it) {
        if (it->second.lastUsed < leastUsed->second.lastUsed) {
            leastUsed = it;
        }
    }
    
    m_Fonts.erase(leastUsed);
    return true;
}

void FontManager::RegisterFontLoader() {
    DRIFT_PROFILE_FUNCTION();
    
    auto& assetsSystem = Drift::Core::Assets::AssetsSystem::GetInstance();
    auto loader = std::make_shared<FontLoader>(m_Device);
    assetsSystem.RegisterLoader<Font>(loader);
}

void FontManager::UnregisterFontLoader() {
    DRIFT_PROFILE_FUNCTION();
    
    auto& assetsSystem = Drift::Core::Assets::AssetsSystem::GetInstance();
    assetsSystem.UnregisterLoader<Font>();
}

std::shared_ptr<Font> FontManager::FindBestFallbackFont(uint32_t codepoint) {
    DRIFT_PROFILE_FUNCTION();
    
    // Tentar encontrar fonte que suporte o codepoint
    for (const auto& pair : m_FallbackFonts) {
        if (pair.second->HasGlyph(codepoint)) {
            return pair.second;
        }
    }
    
    return nullptr;
}

bool FontManager::IsSystemFontAvailable(const std::string& name) const {
    // TODO: Implementar verificação de fontes do sistema
    return false;
}

FontLoadConfig FontManager::CreateDefaultConfig(float size, FontQuality quality) const {
    FontLoadConfig config;
    config.size = size;
    config.quality = quality;
    config.dpi = m_Config.defaultDPI;
    config.enableHinting = m_Config.enableHinting;
    config.enableKerning = m_Config.enableKerning;
    config.enableLigatures = m_Config.enableLigatures;
    return config;
}

std::string FontManager::GetFontNameFromPath(const std::string& path) const {
    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of('.');
    
    if (lastSlash == std::string::npos) {
        lastSlash = 0;
    } else {
        lastSlash++;
    }
    
    if (lastDot == std::string::npos) {
        lastDot = path.length();
    }
    
    return path.substr(lastSlash, lastDot - lastSlash);
}

bool FontManager::IsValidFontPath(const std::string& path) const {
    return std::filesystem::exists(path) && 
           (path.ends_with(".ttf") || path.ends_with(".otf") || 
            path.ends_with(".woff") || path.ends_with(".woff2"));
}

} // namespace Drift::UI 