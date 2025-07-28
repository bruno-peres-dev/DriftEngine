#pragma once

#include "Drift/Core/AssetsManager.h"
#include "Drift/UI/FontSystem/Font.h"
#include "Drift/UI/FontSystem/FontManager.h"

namespace Drift::Core::Assets {

/**
 * @brief Parâmetros para carregamento de fontes
 */
struct FontLoadParams {
    float size = 16.0f;
    UI::FontQuality quality = UI::FontQuality::High;
    std::string name = ""; // Nome da fonte (se vazio, usa o nome do arquivo)
};

/**
 * @brief Asset wrapper para fontes
 */
class FontAsset : public IAsset {
public:
    FontAsset(const std::string& path, std::shared_ptr<UI::Font> font = nullptr);
    virtual ~FontAsset() = default;

    // IAsset implementation
    size_t GetMemoryUsage() const override;
    const std::string& GetPath() const override { return m_Path; }
    bool IsLoaded() const override { return m_Font != nullptr; }
    bool Load() override;
    void Unload() override;

    // Font-specific methods
    std::shared_ptr<UI::Font> GetFont() const { return m_Font; }
    float GetSize() const { return m_Size; }
    UI::FontQuality GetQuality() const { return m_Quality; }
    const std::string& GetFontName() const { return m_FontName; }
    
    void SetFont(std::shared_ptr<UI::Font> font) { m_Font = font; }
    void SetLoadParams(const FontLoadParams& params);

private:
    std::string m_Path;
    std::shared_ptr<UI::Font> m_Font;
    std::string m_FontName;
    float m_Size = 16.0f;
    UI::FontQuality m_Quality = UI::FontQuality::High;
    size_t m_EstimatedMemoryUsage = 0;
};

/**
 * @brief Loader para fontes
 */
class FontLoader : public IAssetLoader<FontAsset> {
public:
    FontLoader() = default;

    std::shared_ptr<FontAsset> Load(const std::string& path, const std::any& params = {}) override;
    bool CanLoad(const std::string& path) const override;
    std::vector<std::string> GetSupportedExtensions() const override;

    // Métodos auxiliares públicos
    size_t EstimateFontMemoryUsage(float size, UI::FontQuality quality) const;

private:
    // Métodos auxiliares privados
    FontLoadParams ExtractParams(const std::any& params) const;
    std::string ExtractFontName(const std::string& path, const std::string& requestedName) const;
};

} // namespace Drift::Core::Assets