#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "stb_truetype.h"
#include "Drift/UI/FontSystem/FontAtlas.h"

namespace Drift::UI {

// Estrutura para representar um glyph individual
struct Glyph {
    uint32_t codepoint;
    glm::vec2 position;      // Posição no atlas
    glm::vec2 size;          // Tamanho do glyph
    glm::vec2 offset;        // Offset para posicionamento
    float advance;           // Avanço horizontal
    glm::vec2 uvMin;         // Coordenadas UV mínimas
    glm::vec2 uvMax;         // Coordenadas UV máximas
    bool isValid;
};

// Configurações de qualidade de fonte
enum class FontQuality {
    Low,        // 8x MSDF, sem suavização adicional
    Medium,     // 16x MSDF, suavização básica
    High,       // 32x MSDF, suavização avançada
    Ultra       // 64x MSDF, suavização máxima + subpixel
};

// Configurações de renderização de texto
struct TextRenderSettings {
    FontQuality quality = FontQuality::High;
    bool enableSubpixel = true;
    bool enableLigatures = true;
    bool enableKerning = true;
    float gamma = 2.2f;
    float contrast = 0.1f;
    float smoothing = 0.1f;
};

// Classe para representar uma fonte carregada
class Font {
public:
    Font(const std::string& name, const std::string& filePath, float size, FontQuality quality);
    ~Font();

    bool Load();
    void Unload();

    const Glyph* GetGlyph(uint32_t character) const;
    float GetKerning(uint32_t left, uint32_t right) const;
    glm::vec2 MeasureText(const std::string& text) const;
    float GetLineHeight() const;
    float GetAscender() const;
    float GetDescender() const;
    const std::string& GetName() const;
    float GetSize() const;
    FontQuality GetQuality() const;
    bool IsLoaded() const;
    
    // Métodos adicionais
    const std::string& GetFilePath() const;
    const std::unique_ptr<class FontAtlas>& GetAtlas() const;

private:
    std::string m_Name;
    std::string m_FilePath;
    float m_Size;
    FontQuality m_Quality;
    bool m_IsLoaded;
    float m_LineHeight;
    float m_Ascender;
    float m_Descender;
    std::unordered_map<uint32_t, Glyph> m_Glyphs;
    std::unordered_map<uint64_t, float> m_Kerning;
    std::unique_ptr<class FontAtlas> m_Atlas;
    std::vector<unsigned char> m_TTFBuffer;
    std::vector<unsigned char> m_Bitmap;
    int m_BitmapWidth{0};
    int m_BitmapHeight{0};
    float m_Scale{1.0f};
    stbtt_fontinfo m_FontInfo{};

    void LoadBasicGlyphs();
    
    // Permitir que FontManager acesse membros privados
    friend class FontManager;
}; 

// Gerenciador principal de fontes
class FontManager {
public:
    static FontManager& GetInstance();
    std::shared_ptr<Font> LoadFont(const std::string& name, const std::string& filePath, float size, FontQuality quality = FontQuality::High);
    std::shared_ptr<Font> GetFont(const std::string& name, float size, FontQuality quality = FontQuality::High);
    void UnloadFont(const std::string& name, float size, FontQuality quality = FontQuality::High);
    void UnloadAllFonts();
    void SetDefaultQuality(FontQuality quality);
    void SetDefaultSize(float size);
    void SetDefaultFontName(const std::string& name);
    void PreloadFont(const std::string& name, const std::string& filePath, const std::vector<float>& sizes, FontQuality quality = FontQuality::High);
    
    // Método para criar fonte padrão embutida
    std::shared_ptr<Font> CreateEmbeddedDefaultFont(float size, FontQuality quality = FontQuality::High);
    void BeginTextRendering();
    void EndTextRendering();
    
    // Métodos utilitários
    size_t GetLoadedFontCount() const;
    std::vector<std::string> GetLoadedFontNames() const;

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
            return std::hash<std::string>{}(key.name) ^ std::hash<float>{}(key.size) ^ std::hash<int>{}(static_cast<int>(key.quality));
        }
    };
    std::unordered_map<FontKey, std::shared_ptr<Font>, FontKeyHash> m_Fonts;
    FontQuality m_DefaultQuality;
    float m_DefaultSize;
    std::string m_DefaultFontName;
    bool m_IsRendering;
};

// Utilitários para renderização de texto
namespace TextUtils {
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName = "default", float size = 16.0f);
    std::vector<std::string> WordWrap(const std::string& text, float maxWidth, const std::string& fontName = "default", float size = 16.0f);
}

} // namespace Drift::UI 