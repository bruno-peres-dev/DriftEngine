#pragma once

#include "Drift/UI/UIElement.h"
#include <string>
#include <glm/vec4.hpp>

namespace Drift::UI {

class Image : public UIElement {
public:
    explicit Image(UIContext* context);
    ~Image() override = default;

    // Propriedades da imagem
    void SetImagePath(const std::string& path) { m_ImagePath = path; MarkDirty(); }
    const std::string& GetImagePath() const { return m_ImagePath; }

    // UV coordinates para sprites
    void SetUV(const glm::vec4& uv) { m_UV = uv; }
    glm::vec4 GetUV() const { return m_UV; }

    // Tamanho da imagem
    void SetImageSize(const glm::vec2& size) { m_ImageSize = size; MarkDirty(); }
    glm::vec2 GetImageSize() const { return m_ImageSize; }

    // Modo de escala
    enum class ScaleMode {
        Stretch,    // Estica para caber no widget
        Fit,        // Mantém proporção, ajusta para caber
        Fill,       // Mantém proporção, corta se necessário
        Tile        // Repete a textura
    };

    void SetScaleMode(ScaleMode mode) { m_ScaleMode = mode; MarkDirty(); }
    ScaleMode GetScaleMode() const { return m_ScaleMode; }

    // Tint color
    void SetTintColor(unsigned color) { m_TintColor = color; }
    unsigned GetTintColor() const { return m_TintColor; }

    // Overrides
    void Update(float deltaSeconds) override;
    void Render(Drift::RHI::IUIBatcher& batch) override;

    // Carregamento de textura
    bool LoadTexture(const std::string& path);

private:
    std::string m_ImagePath;
    glm::vec4 m_UV{0.0f, 0.0f, 1.0f, 1.0f}; // UV coordinates (x, y, width, height)
    glm::vec2 m_ImageSize{100.0f, 100.0f};   // Tamanho original da imagem
    ScaleMode m_ScaleMode{ScaleMode::Stretch};
    unsigned m_TintColor{0xFFFFFFFF}; // Sem tint por padrão

    // TODO: Adicionar suporte a texturas quando implementarmos o sistema de recursos
    // std::shared_ptr<ITexture> m_Texture;
};

} // namespace Drift::UI 