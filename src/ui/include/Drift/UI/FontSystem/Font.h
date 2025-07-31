#pragma once

#include "Drift/Core/Assets/AssetsSystem.h"
#include "FontAtlas.h"
#include "FontMetrics.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
// Forward declaration para evitar múltiplas inclusões
struct stbtt_fontinfo;

namespace Drift::UI {

/**
 * @brief Qualidade de renderização da fonte
 */
enum class FontQuality { 
    Low,    ///< Qualidade baixa (atlas 256x256)
    Medium, ///< Qualidade média (atlas 512x512)
    High,   ///< Qualidade alta (atlas 1024x1024)
    Ultra   ///< Qualidade ultra (atlas 2048x2048)
};

/**
 * @brief Formato de fonte suportado
 */
enum class FontFormat {
    TTF,    // TrueType
    OTF,    // OpenType
    WOFF,   // Web Open Font Format
    WOFF2,  // Web Open Font Format 2.0
    BMF     // Bitmap Font (futuro)
};

/**
 * @brief Configuração de carregamento de fonte
 */
struct FontLoadConfig {
    float size = 16.0f;                    // Tamanho da fonte em pixels
    FontQuality quality = FontQuality::High; // Qualidade de renderização
    FontFormat format = FontFormat::TTF;   // Formato da fonte
    float dpi = 96.0f;                     // DPI para renderização
    bool enableHinting = true;             // Habilita hinting
    bool enableKerning = true;             // Habilita kerning
    bool enableLigatures = true;           // Habilita ligaduras
    std::vector<uint32_t> preloadChars;    // Caracteres para pré-carregar
    bool enableFallback = true;            // Habilita fallback de fontes
};

/**
 * @brief Fonte Profissional Integrada ao Sistema de Assets
 * 
 * Esta classe representa uma fonte carregada com suporte completo a:
 * - Integração com AssetsSystem
 * - Múltiplos formatos de fonte
 * - Cache inteligente de glyphs
 * - Sistema de fallback
 * - Métricas tipográficas completas
 * - Renderização de alta qualidade
 */
class Font : public Drift::Core::Assets::IAsset {
public:
    /**
     * @brief Construtor
     * @param name Nome da fonte
     * @param config Configuração de carregamento
     */
    Font(const std::string& name, const FontLoadConfig& config = {});
    
    /**
     * @brief Destrutor
     */
    ~Font();

    // Implementação de IAsset
    const std::string& GetPath() const override { return m_Path; }
    const std::string& GetName() const override { return m_Name; }
    size_t GetMemoryUsage() const override;
    Drift::Core::Assets::AssetStatus GetStatus() const override { return m_Status; }
    bool Load() override;
    void Unload() override;
    bool IsLoaded() const override { return m_Status == Drift::Core::Assets::AssetStatus::Loaded; }
    std::chrono::steady_clock::time_point GetLoadTime() const override { return m_LoadTime; }
    size_t GetAccessCount() const override { return m_AccessCount; }
    void UpdateAccess() override { m_AccessCount++; }

    // Carregamento de fontes
    bool LoadFromFile(const std::string& path);
    bool LoadFromMemory(const unsigned char* data, size_t size);
    bool LoadFromAsset(const std::string& assetPath);

    // Acesso a glyphs
    const GlyphInfo* GetGlyph(uint32_t codepoint) const;
    bool HasGlyph(uint32_t codepoint) const;
    bool LoadGlyph(uint32_t codepoint);
    
    // Kerning
    float GetKerning(uint32_t left, uint32_t right) const;
    
    // Métricas
    const FontMetrics& GetMetrics() const { return m_Metrics; }
    float GetSize() const { return m_Config.size; }
    FontQuality GetQuality() const { return m_Config.quality; }
    FontFormat GetFormat() const { return m_Config.format; }
    
    // Atlas
    std::shared_ptr<FontAtlas> GetAtlas() const { return m_Atlas; }
    std::shared_ptr<Drift::RHI::ITexture> GetAtlasTexture() const;
    
    // Fallback
    void SetFallbackFont(std::shared_ptr<Font> fallback) { m_FallbackFont = fallback; }
    std::shared_ptr<Font> GetFallbackFont() const { return m_FallbackFont; }
    
    // Utilitários
    bool IsValid() const { return m_IsValid; }
    std::string GetFamilyName() const { return m_FamilyName; }
    std::string GetStyleName() const { return m_StyleName; }
    bool IsBold() const { return m_IsBold; }
    bool IsItalic() const { return m_IsItalic; }
    bool IsMonospace() const { return m_IsMonospace; }

private:
    // Dados da fonte
    std::string m_Name;
    std::string m_Path;
    FontLoadConfig m_Config;
    Drift::Core::Assets::AssetStatus m_Status{Drift::Core::Assets::AssetStatus::NotLoaded};
    std::chrono::steady_clock::time_point m_LoadTime;
    size_t m_AccessCount{0};
    
    // Dados TTF/OTF
    std::vector<unsigned char> m_FontData;
    std::unique_ptr<stbtt_fontinfo> m_FontInfo;
    bool m_IsValid{false};
    
    // Informações da fonte
    std::string m_FamilyName;
    std::string m_StyleName;
    bool m_IsBold{false};
    bool m_IsItalic{false};
    bool m_IsMonospace{false};
    
    // Métricas
    FontMetrics m_Metrics;
    
    // Atlas e glyphs
    std::shared_ptr<FontAtlas> m_Atlas;
    std::unordered_map<uint32_t, GlyphInfo> m_Glyphs;
    
    // Fallback
    std::shared_ptr<Font> m_FallbackFont;
    
    // Cache de kerning
    mutable std::unordered_map<uint64_t, float> m_KerningCache;
    
    // Métodos auxiliares
    bool InitializeFontInfo();
    bool LoadFontMetrics();
    bool LoadFontInfo();
    bool CreateAtlas();
    bool LoadGlyphInternal(uint32_t codepoint);
    uint32_t GetFallbackCodepoint(uint32_t codepoint) const;
    
    // Utilitários
    int GetAtlasSize() const;
    float GetScale() const;
    uint64_t MakeKerningKey(uint32_t left, uint32_t right) const;
    
    // Constantes
    static constexpr size_t MAX_GLYPHS = 65536;
    static constexpr size_t KERNING_CACHE_SIZE = 10000;
};

/**
 * @brief Loader de fontes para o sistema de assets
 */
class FontLoader : public Drift::Core::Assets::IAssetLoader<Font> {
public:
    /**
     * @brief Construtor
     * @param device Dispositivo RHI
     */
    FontLoader(Drift::RHI::IDevice* device);
    
    /**
     * @brief Destrutor
     */
    ~FontLoader();

    // Implementação de IAssetLoader
    std::shared_ptr<Font> Load(const std::string& path, const std::any& params = {}) override;
    bool CanLoad(const std::string& path) const override;
    std::vector<std::string> GetSupportedExtensions() const override;
    std::string GetLoaderName() const override { return "FontLoader"; }
    size_t EstimateMemoryUsage(const std::string& path) const override;

private:
    Drift::RHI::IDevice* m_Device{nullptr};
    
    // Extensões suportadas
    static const std::vector<std::string> s_SupportedExtensions;
    
    // Utilitários
    FontLoadConfig ParseLoadParams(const std::any& params) const;
    bool IsValidFontFile(const std::string& path) const;
};

} // namespace Drift::UI
