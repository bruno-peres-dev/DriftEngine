#pragma once

#include "Font.h"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string_view>
#include "Drift/RHI/Device.h"

namespace Drift::UI {

/**
 * @brief Configuração de cache de fontes
 */
struct FontCacheConfig {
    size_t maxFonts = 100;           ///< Número máximo de fontes em cache
    size_t maxTTFData = 50;          ///< Número máximo de arquivos TTF em cache
    bool enableLazyLoading = true;   ///< Habilita carregamento sob demanda
    bool enablePreloading = true;    ///< Habilita pré-carregamento
};

/**
 * @brief Gerenciador de fontes com cache otimizado
 * 
 * Esta classe implementa um sistema de cache inteligente para fontes TTF,
 * incluindo compartilhamento de dados TTF, lazy loading e limpeza automática.
 */
class FontManager {
public:
    /**
     * @brief Obtém a instância singleton do gerenciador
     * @return Referência para a instância única
     */
    static FontManager& GetInstance();

    // Configuração do sistema
    void SetDevice(Drift::RHI::IDevice* device) { m_Device = device; }
    void SetDefaultFontName(const std::string& name) { m_DefaultFontName = name; }
    void SetDefaultFontPath(const std::string& path) { m_DefaultFontPath = path; }
    void SetDefaultSize(float size) { m_DefaultSize = size; }
    void SetDefaultQuality(FontQuality q) { m_DefaultQuality = q; }
    void SetCacheConfig(const FontCacheConfig& config) { m_CacheConfig = config; }
    const std::string& GetDefaultFontName() const;
    const std::string& GetDefaultFontPath() const;

    // Carregamento e obtenção de fontes
    std::shared_ptr<Font> LoadFont(const std::string& name, const std::string& path, float size, FontQuality quality);
    std::shared_ptr<Font> GetFont(const std::string& name, float size, FontQuality quality);
    std::shared_ptr<Font> GetFont(const std::string& name, float size);
    
    // Otimizações de cache
    void PreloadFontFile(const std::string& path);
    void ClearCache();
    size_t GetCacheSize() const;
    void TrimCache();  // Remove fontes menos usadas
    
    // Lazy loading otimizado
    std::shared_ptr<Font> GetOrLoadFont(const std::string& name, const std::string& path, float size, FontQuality quality = FontQuality::High);
    void PreloadCommonSizes(const std::string& name, const std::string& path, const std::vector<float>& sizes);

    // Estatísticas e debug
    struct CacheStats {
        size_t fontCount = 0;
        size_t ttfDataCount = 0;
        size_t totalMemoryUsage = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
    };
    CacheStats GetCacheStats() const;

private:
    FontManager() = default;
    
    /**
     * @brief Chave para identificação única de fontes no cache
     */
    struct FontKey {
        std::string name;
        float size;
        FontQuality quality;
        
        bool operator==(const FontKey& other) const {
            return name == other.name && size == other.size && quality == other.quality;
        }
    };
    
    /**
     * @brief Hash function otimizada para FontKey
     */
    struct FontKeyHash {
        size_t operator()(const FontKey& k) const noexcept {
            return std::hash<std::string>{}(k.name) ^ 
                   (std::hash<int>{}(static_cast<int>(k.size * 10)) << 1) ^ 
                   (std::hash<int>{}(static_cast<int>(k.quality)) << 2);
        }
    };

    /**
     * @brief Dados TTF compartilhados entre fontes
     */
    struct TTFData {
        std::vector<unsigned char> data;
        std::string path;
        size_t refCount = 0;
        size_t lastUsed = 0;  // Timestamp para LRU
    };
    
    /**
     * @brief Entrada de fonte no cache com estatísticas
     */
    struct FontCacheEntry {
        std::shared_ptr<Font> font;
        size_t lastUsed = 0;
        size_t accessCount = 0;
    };
    
    // Cache de dados TTF compartilhados
    std::unordered_map<std::string, std::shared_ptr<TTFData>> m_TTFDataCache;
    std::unordered_map<FontKey, FontCacheEntry, FontKeyHash> m_Fonts;
    
    mutable std::mutex m_Mutex;
    Drift::RHI::IDevice* m_Device{nullptr};
    std::string m_DefaultFontName{"default"};
    std::string m_DefaultFontPath{"fonts/Arial-Regular.ttf"};
    float m_DefaultSize{16.0f};
    FontQuality m_DefaultQuality{FontQuality::High};
    FontCacheConfig m_CacheConfig;
    
    // Estatísticas
    mutable size_t m_CacheHits{0};
    mutable size_t m_CacheMisses{0};
    mutable size_t m_LastTrimTime{0};
    
    // Métodos auxiliares
    std::shared_ptr<TTFData> LoadTTFData(const std::string& path);
    void ReleaseTTFData(const std::string& path);
    void UpdateCacheStats(bool hit);
    size_t GetCurrentTime() const;
};

} // namespace Drift::UI
