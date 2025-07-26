#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <glm/glm.hpp>
#include "stb_truetype.h"
#include "Drift/UI/FontSystem/FontAtlas.h"

namespace Drift::UI {

// Estrutura para representar um glyph individual otimizada
struct Glyph {
    uint32_t codepoint;
    glm::vec2 position;      // Posição no atlas
    glm::vec2 size;          // Tamanho do glyph
    glm::vec2 offset;        // Offset para posicionamento
    float advance;           // Avanço horizontal
    glm::vec2 uvMin;         // Coordenadas UV mínimas
    glm::vec2 uvMax;         // Coordenadas UV máximas
    bool isValid;
    uint32_t atlasId;        // ID do atlas que contém este glyph
    
    Glyph() : codepoint(0), position(0), size(0), offset(0), advance(0), 
              uvMin(0), uvMax(0), isValid(false), atlasId(0) {}
};

// Configurações de qualidade de fonte otimizadas
enum class FontQuality {
    Low = 0,        // 8x MSDF, sem suavização adicional
    Medium = 1,     // 16x MSDF, suavização básica
    High = 2,       // 32x MSDF, suavização avançada
    Ultra = 3       // 64x MSDF, suavização máxima + subpixel
};

// Configurações de renderização de texto avançadas
struct TextRenderSettings {
    FontQuality quality = FontQuality::High;
    bool enableSubpixel = true;
    bool enableLigatures = true;
    bool enableKerning = true;
    bool enableHinting = true;
    float gamma = 2.2f;
    float contrast = 0.1f;
    float smoothing = 0.1f;
    float outlineWidth = 0.0f;
    glm::vec4 outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    bool operator==(const TextRenderSettings& other) const {
        return quality == other.quality &&
               enableSubpixel == other.enableSubpixel &&
               enableLigatures == other.enableLigatures &&
               enableKerning == other.enableKerning &&
               enableHinting == other.enableHinting &&
               gamma == other.gamma &&
               contrast == other.contrast &&
               smoothing == other.smoothing &&
               outlineWidth == other.outlineWidth &&
               outlineColor == other.outlineColor;
    }
};

// Métricas de fonte otimizadas
struct FontMetrics {
    float ascender;
    float descender;
    float lineHeight;
    float xHeight;
    float capHeight;
    float underlinePosition;
    float underlineThickness;
    float maxAdvance;
    glm::vec2 boundingBox;
    
    FontMetrics() : ascender(0), descender(0), lineHeight(0), xHeight(0),
                   capHeight(0), underlinePosition(0), underlineThickness(0),
                   maxAdvance(0), boundingBox(0) {}
};

// Configurações de cache de fonte
struct FontCacheConfig {
    size_t maxFonts = 64;           // Máximo de fontes em cache
    size_t maxGlyphsPerFont = 4096; // Máximo de glyphs por fonte
    size_t maxAtlasSize = 4096;     // Tamanho máximo do atlas
    bool enablePreloading = true;   // Habilitar pré-carregamento
    bool enableLazyLoading = true;  // Habilitar carregamento lazy
    float memoryBudgetMB = 256.0f;  // Orçamento de memória em MB
};

// Estatísticas de uso de fonte
struct FontStats {
    size_t totalFonts = 0;
    size_t totalGlyphs = 0;
    size_t totalAtlases = 0;
    size_t memoryUsageBytes = 0;
    size_t cacheHits = 0;
    size_t cacheMisses = 0;
    float cacheHitRate = 0.0f;
    
    void Reset() {
        totalFonts = 0;
        totalGlyphs = 0;
        totalAtlases = 0;
        memoryUsageBytes = 0;
        cacheHits = 0;
        cacheMisses = 0;
        cacheHitRate = 0.0f;
    }
};

// Classe para representar uma fonte carregada otimizada
class Font {
public:
    Font(const std::string& name, const std::string& filePath, float size, FontQuality quality);
    ~Font();

    bool Load();
    void Unload();
    bool IsLoaded() const;

    // Métodos de acesso a glyphs
    const Glyph* GetGlyph(uint32_t character) const;
    bool HasGlyph(uint32_t character) const;
    void PreloadGlyphs(const std::vector<uint32_t>& characters);
    
    // Métricas e medidas
    float GetKerning(uint32_t left, uint32_t right) const;
    glm::vec2 MeasureText(const std::string& text) const;
    glm::vec2 MeasureText(const std::wstring& text) const;
    std::vector<glm::vec2> GetGlyphPositions(const std::string& text, float x, float y) const;
    
    // Propriedades da fonte
    float GetLineHeight() const;
    float GetAscender() const;
    float GetDescender() const;
    const FontMetrics& GetMetrics() const { return m_Metrics; }
    const std::string& GetName() const;
    float GetSize() const;
    FontQuality GetQuality() const;
    const std::string& GetFilePath() const;
    
    // Atlas e texturas
    const std::unique_ptr<class FontAtlas>& GetAtlas() const;
    uint32_t GetAtlasTextureId() const;
    
    // Cache e otimizações
    void Touch(); // Marca como usado recentemente
    size_t GetLastUsed() const { return m_LastUsed; }
    size_t GetMemoryUsage() const;

private:
    std::string m_Name;
    std::string m_FilePath;
    float m_Size;
    FontQuality m_Quality;
    std::atomic<bool> m_IsLoaded{false};
    
    FontMetrics m_Metrics;
    std::unordered_map<uint32_t, Glyph> m_Glyphs;
    std::unordered_map<uint64_t, float> m_Kerning;
    std::unique_ptr<class FontAtlas> m_Atlas;
    
    // Dados da fonte TTF
    std::vector<unsigned char> m_TTFBuffer;
    stbtt_fontinfo m_FontInfo{};
    float m_Scale{1.0f};
    
    // Cache e otimizações
    size_t m_LastUsed{0};
    mutable std::mutex m_GlyphMutex;
    
    void LoadBasicGlyphs();
    void LoadGlyph(uint32_t character);
    void CalculateMetrics();
    
    // Permitir que FontManager acesse membros privados
    friend class FontManager;
};

// Gerenciador principal de fontes otimizado e profissional
class FontManager {
public:
    static FontManager& GetInstance();
    
    // === Gerenciamento de fontes ===
    std::shared_ptr<Font> LoadFont(const std::string& name, const std::string& filePath, 
                                  float size, FontQuality quality = FontQuality::High);
    std::shared_ptr<Font> GetFont(const std::string& name, float size, 
                                 FontQuality quality = FontQuality::High);
    void UnloadFont(const std::string& name, float size, FontQuality quality = FontQuality::High);
    void UnloadAllFonts();
    
    // === Configurações ===
    void SetDefaultQuality(FontQuality quality);
    void SetDefaultSize(float size);
    void SetDefaultFontName(const std::string& name);
    void SetCacheConfig(const FontCacheConfig& config);
    FontCacheConfig GetCacheConfig() const { return m_CacheConfig; }
    
    // === Pré-carregamento e otimizações ===
    void PreloadFont(const std::string& name, const std::string& filePath, 
                    const std::vector<float>& sizes, FontQuality quality = FontQuality::High);
    void PreloadCharacters(const std::string& fontName, const std::vector<uint32_t>& characters,
                          float size = 16.0f, FontQuality quality = FontQuality::High);
    
    // === Fonte padrão embutida ===
    std::shared_ptr<Font> CreateEmbeddedDefaultFont(float size, FontQuality quality = FontQuality::High);
    
    // === Ciclo de renderização ===
    void BeginTextRendering();
    void EndTextRendering();
    
    // === Cache e memória ===
    void UpdateCache();
    void ClearCache();
    void TrimCache();
    FontStats GetStats() const;
    void ResetStats();
    
    // === Métodos utilitários ===
    size_t GetLoadedFontCount() const;
    std::vector<std::string> GetLoadedFontNames() const;
    bool IsFontLoaded(const std::string& name, float size, FontQuality quality = FontQuality::High) const;
    
    // === Otimizações avançadas ===
    void SetMemoryBudget(float budgetMB);
    void EnableAsyncLoading(bool enabled);
    void SetWorkerThreadCount(size_t count);

private:
    FontManager();
    ~FontManager();
    
    struct FontKey {
        std::string name;
        float size;
        FontQuality quality;
        
        bool operator==(const FontKey& other) const {
            return name == other.name && size == other.size && quality == other.quality;
        }
    };
    
    struct FontKeyHash {
        size_t operator()(const FontKey& key) const {
            return std::hash<std::string>{}(key.name) ^ 
                   std::hash<float>{}(key.size) ^ 
                   std::hash<int>{}(static_cast<int>(key.quality));
        }
    };
    
    // Cache de fontes com LRU
    std::unordered_map<FontKey, std::shared_ptr<Font>, FontKeyHash> m_Fonts;
    std::queue<FontKey> m_FontUsageQueue;
    
    // Configurações
    FontQuality m_DefaultQuality;
    float m_DefaultSize;
    std::string m_DefaultFontName;
    FontCacheConfig m_CacheConfig;
    FontStats m_Stats;
    
    // Estado de renderização
    bool m_IsRendering;
    size_t m_FrameCounter;
    
    // Threading e sincronização
    mutable std::mutex m_FontMutex;
    std::atomic<bool> m_AsyncLoadingEnabled{false};
    size_t m_WorkerThreadCount{4};
    
    // Métodos internos
    void UpdateFontUsage(const FontKey& key);
    void TrimFontCache();
    size_t CalculateFontMemoryUsage(const std::shared_ptr<Font>& font) const;
    void UpdateStats();
};

// Utilitários para renderização de texto otimizados
namespace TextUtils {
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName = "default", 
                         float size = 16.0f, FontQuality quality = FontQuality::High);
    std::vector<std::string> WordWrap(const std::string& text, float maxWidth, 
                                     const std::string& fontName = "default", 
                                     float size = 16.0f, FontQuality quality = FontQuality::High);
    std::string TruncateText(const std::string& text, float maxWidth, 
                            const std::string& fontName = "default", 
                            float size = 16.0f, FontQuality quality = FontQuality::High);
    std::vector<uint32_t> StringToCodepoints(const std::string& text);
    std::string CodepointsToString(const std::vector<uint32_t>& codepoints);
}

} // namespace Drift::UI 