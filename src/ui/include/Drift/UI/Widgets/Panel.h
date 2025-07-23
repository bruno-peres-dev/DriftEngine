#pragma once

#include "Drift/UI/UIElement.h"
#include <string>

namespace Drift::UI {

class Panel : public UIElement {
public:
    explicit Panel(UIContext* context);
    ~Panel() override = default;

    // Propriedades do painel
    void SetTitle(const std::string& title) { m_Title = title; }
    const std::string& GetTitle() const { return m_Title; }

    // Bordas e padding
    void SetBorderWidth(float width) { m_BorderWidth = width; MarkDirty(); }
    float GetBorderWidth() const { return m_BorderWidth; }

    void SetBorderColor(unsigned color) { m_BorderColor = color; }
    unsigned GetBorderColor() const { return m_BorderColor; }

    void SetPadding(float padding) { 
        m_Padding = padding; 
        MarkDirty(); 
    }
    float GetPadding() const { return m_Padding; }

    // Visibilidade da borda
    void SetShowBorder(bool show) { m_ShowBorder = show; }
    bool GetShowBorder() const { return m_ShowBorder; }

    // Overrides
    void Update(float deltaSeconds) override;
    void Render(Drift::RHI::IUIBatcher& batch) override;

    // Layout do painel
    glm::vec2 GetContentArea() const;

private:
    std::string m_Title;
    float m_BorderWidth{1.0f};
    unsigned m_BorderColor{0xFF666666}; // Cinza
    float m_Padding{5.0f};
    bool m_ShowBorder{true};

    // Constantes de cor para painéis
    static constexpr unsigned COLOR_PANEL_BG = 0xFF2A2A2A;
    static constexpr unsigned COLOR_BORDER = 0xFF666666;
    static constexpr unsigned COLOR_TITLE = 0xFFFFFFFF;
};

} // namespace Drift::UI 