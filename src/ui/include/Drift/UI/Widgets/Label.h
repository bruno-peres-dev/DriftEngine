#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/Core/Color.h"
#include <string>
#include <glm/vec2.hpp>

namespace Drift::UI {

class Label : public UIElement {
public:
    explicit Label(UIContext* context);
    ~Label() override = default;

    // Propriedades do texto
    void SetText(const std::string& text) { m_Text = text; MarkDirty(); }
    const std::string& GetText() const { return m_Text; }

    // Propriedades de fonte
    void SetFontSize(float size) { m_FontSize = size; MarkDirty(); }
    float GetFontSize() const { return m_FontSize; }

    void SetFontFamily(const std::string& family) { m_FontFamily = family; MarkDirty(); }
    const std::string& GetFontFamily() const { return m_FontFamily; }

    // Alinhamento de texto
    enum class TextAlign {
        Left,
        Center,
        Right
    };
    
    void SetTextAlign(TextAlign align) { m_TextAlign = align; MarkDirty(); }
    TextAlign GetTextAlign() const { return m_TextAlign; }

    // Cor do texto
    void SetTextColor(Drift::Color color) { m_TextColor = color; }
    Drift::Color GetTextColor() const { return m_TextColor; }

    // Overrides
    void Update(float deltaSeconds) override;
    void Render(Drift::RHI::IUIBatcher& batch) override;

    // Cálculo de tamanho baseado no texto
    glm::vec2 CalculateTextSize() const;

private:
    std::string m_Text;
    float m_FontSize{16.0f};
    std::string m_FontFamily{"Arial"};
    TextAlign m_TextAlign{TextAlign::Left};
    Drift::Color m_TextColor{0xFFFFFFFF}; // Branco por padrão
    glm::vec2 m_TextSize{0.0f, 0.0f}; // Tamanho calculado do texto

    // Constantes de cor para texto
    static constexpr Drift::Color COLOR_WHITE = 0xFFFFFFFF;
    static constexpr Drift::Color COLOR_BLACK = 0xFF000000;
    static constexpr Drift::Color COLOR_GRAY = 0xFF808080;
    
    // Otimização de renderização de texto
    glm::vec2 m_LastTextPos{0.0f, 0.0f};
    Drift::Color m_LastTextColor{0xFFFFFFFF};
};

} // namespace Drift::UI 