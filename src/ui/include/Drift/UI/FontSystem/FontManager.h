#pragma once

#include "Font.h"
#include "FontAtlas.h"
#include "FontMetrics.h"
#include "Drift/Core/Assets/AssetsSystem.h"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

namespace Drift::UI {

/**
 * @brief Configuração do sistema de fontes
 */
struct FontSystemConfig {
    // Configurações de cache
    size_t maxFonts = 100;                    // Número máximo de fontes em cache
    size_t maxAtlasCount = 20;                // Número máximo de atlas
    bool enableLazyLoading = true;            // Habilita carregamento sob demanda
    bool enablePreloading = true;             // Habilita pré-carregamento
    bool enableAsyncLoading = true;           // Habilita carregamento assíncrono
    
    // Configurações de qualidade
    FontQuality defaultQuality = FontQuality::High;
    float defaultDPI = 96.0f;
    bool enableHinting = true;
    bool enableKerning = true;
    bool enableLigatures = true;
    
    // Configurações de fallback
    std::vector<std::string> fallbackFonts;   // Fontes de fallback
    bool enableUnicodeFallback = true;        // Fallback para caracteres Unicode
    bool enableSystemFontFallback = true;     // Fallback para fontes do sistema
    
    // Configurações de performance
    size_t maxConcurrentLoads = 4;            // Máximo de carregamentos simultâneos
    size_t preloadCharSetSize = 256;          // Tamanho do conjunto de caracteres para pré-carregar
    bool enableAtlasSharing = true;           // Compartilhamento de atlas entre fontes
};

/**
 * @brief Gerenciador de Fontes Profissional Integrado ao Sistema de Assets
 * 
 * Esta classe implementa um sistema de gerenciamento de fontes completo e profissional,
 * integrado ao sistema de assets do Drift Engine. Características principais:
 * 
 * - Integração completa com AssetsSystem
 * - Cache inteligente com LRU
 * - Sistema de fallback de fontes
 * - Carregamento assíncrono
 * - Pré-carregamento otimizado
 * - Suporte a múltiplos formatos
 * - Gerenciamento de atlas compartilhados
 * - Estatísticas detalhadas
 */
class FontManager {
public:
    /**
     * @brief Obtém a instância singleton do gerenciador
     * @return Referência para a instância única
     */
    static FontManager& GetInstance();

    // Inicialização e configuração
    void Initialize(const FontSystemConfig& config = {});
    void Shutdown();
    
    // Configuração
    void SetConfig(const FontSystemConfig& config);
    const FontSystemConfig& GetConfig() const { return m_Config; }
    void SetDevice(Drift::RHI::IDevice* device);
    void SetDefaultFontPath(const std::string& path) { m_DefaultFontPath = path; }

    // Carregamento e obtenção de fontes
    std::shared_ptr<Font> LoadFont(const std::string& path, const FontLoadConfig& config = {});
    std::shared_ptr<Font> GetFont(const std::string& name, float size, FontQuality quality = FontQuality::High);
    std::shared_ptr<Font> GetFont(const std::string& name, float size);
    std::shared_ptr<Font> GetOrLoadFont(const std::string& path, const FontLoadConfig& config = {});
    
    // Carregamento via AssetsSystem
    std::shared_ptr<Font> LoadFontAsset(const std::string& assetPath, const FontLoadConfig& config = {});
    std::shared_ptr<Font> GetFontAsset(const std::string& assetPath, const FontLoadConfig& config = {});
    
    // Carregamento assíncrono
    std::future<std::shared_ptr<Font>> LoadFontAsync(const std::string& path, const FontLoadConfig& config = {});
    std::future<std::shared_ptr<Font>> LoadFontAssetAsync(const std::string& assetPath, const FontLoadConfig& config = {});
    
    // Pré-carregamento
    void PreloadFont(const std::string& path, const FontLoadConfig& config = {});
    void PreloadFontAsset(const std::string& assetPath, const FontLoadConfig& config = {});
    void PreloadCommonSizes(const std::string& path, const std::vector<float>& sizes);
    void PreloadCharSet(const std::string& path, const std::vector<uint32_t>& chars);
    
    // Sistema de fallback
    void RegisterFallbackFont(const std::string& path, const std::string& name);
    void SetSystemFallbackFonts(const std::vector<std::string>& fonts);
    std::shared_ptr<Font> GetFallbackFont(uint32_t codepoint);
    
    // Gerenciamento de atlas
    std::shared_ptr<FontAtlas> GetAtlas(const FontAtlasConfig& config);
    void OptimizeAtlas();
    void ClearUnusedAtlas();
    
    // Gerenciamento de cache
    void ClearCache();
    void TrimCache();
    void UnloadUnusedFonts();
    size_t GetCacheSize() const;
    
    // Estatísticas e debug
    struct FontStats {
        size_t totalFonts = 0;
        size_t loadedFonts = 0;
        size_t loadingFonts = 0;
        size_t failedFonts = 0;
        size_t totalAtlas = 0;
        size_t totalMemoryUsage = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
        size_t fallbackUsage = 0;
        double averageLoadTime = 0.0;
        
        // Estatísticas por qualidade
        std::unordered_map<FontQuality, size_t> fontsByQuality;
        std::unordered_map<FontFormat, size_t> fontsByFormat;
    };
    
    FontStats GetStats() const;
    void LogStats() const;
    void ResetStats();

private:
    FontManager() = default;
    ~FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    /**
     * @brief Chave para identificação única de fontes no cache
     */
    struct FontKey {
        std::string path;
        float size;
        FontQuality quality;
        FontFormat format;
        
        bool operator==(const FontKey& other) const {
            return path == other.path && 
                   size == other.size && 
                   quality == other.quality &&
                   format == other.format;
        }
    };
    
    /**
     * @brief Hash function otimizada para FontKey
     */
    struct FontKeyHash {
        size_t operator()(const FontKey& k) const noexcept {
            return std::hash<std::string>{}(k.path) ^ 
                   (std::hash<int>{}(static_cast<int>(k.size * 10)) << 1) ^ 
                   (std::hash<int>{}(static_cast<int>(k.quality)) << 2) ^
                   (std::hash<int>{}(static_cast<int>(k.format)) << 3);
        }
    };

    /**
     * @brief Entrada de fonte no cache com estatísticas
     */
    struct FontCacheEntry {
        std::shared_ptr<Font> font;
        size_t lastUsed = 0;
        size_t accessCount = 0;
        bool isPreloaded = false;
        std::chrono::steady_clock::time_point loadTime;
    };
    
    // Cache de fontes
    std::unordered_map<FontKey, FontCacheEntry, FontKeyHash> m_Fonts;
    std::unordered_map<std::string, std::shared_ptr<Font>> m_FallbackFonts;
    
    // Configuração e estado
    FontSystemConfig m_Config;
    Drift::RHI::IDevice* m_Device{nullptr};
    std::string m_DefaultFontPath{"fonts/Arial-Regular.ttf"};
    bool m_Initialized{false};
    
    // Estatísticas
    mutable size_t m_CacheHits{0};
    mutable size_t m_CacheMisses{0};
    mutable size_t m_FallbackUsage{0};
    mutable size_t m_LoadCount{0};
    mutable double m_TotalLoadTime{0.0};
    
    // Threading
    mutable std::mutex m_Mutex;
    
    // Métodos auxiliares
    std::shared_ptr<Font> CreateFont(const std::string& path, const FontLoadConfig& config);
    std::shared_ptr<Font> CreateFontAsset(const std::string& assetPath, const FontLoadConfig& config);
    
    void UpdateCacheStats(bool hit);
    void UpdateAccessStats(FontCacheEntry& entry);
    size_t GetCurrentTime() const;
    
    bool EvictLeastUsedFont();
    void RegisterFontLoader();
    void UnregisterFontLoader();
    
    // Fallback
    std::shared_ptr<Font> FindBestFallbackFont(uint32_t codepoint);
    bool IsSystemFontAvailable(const std::string& name) const;
    
    // Utilitários
    FontLoadConfig CreateDefaultConfig(float size, FontQuality quality) const;
    std::string GetFontNameFromPath(const std::string& path) const;
    bool IsValidFontPath(const std::string& path) const;
};

// Macros para facilitar o uso
#define DRIFT_FONTS() Drift::UI::FontManager::GetInstance()

#define DRIFT_LOAD_FONT(path, size, quality) \
    DRIFT_FONTS().LoadFont(path, {size, quality})

#define DRIFT_GET_FONT(name, size) \
    DRIFT_FONTS().GetFont(name, size)

#define DRIFT_LOAD_FONT_ASSET(assetPath, config) \
    DRIFT_FONTS().LoadFontAsset(assetPath, config)

#define DRIFT_PRELOAD_FONT(path, config) \
    DRIFT_FONTS().PreloadFont(path, config)

} // namespace Drift::UI
