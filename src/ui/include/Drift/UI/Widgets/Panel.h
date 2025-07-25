#pragma once

#include "Drift/UI/UIElement.h"
#include <string>

namespace Drift::UI {

class Panel : public UIElement {
public:
    explicit Panel(UIContext* context);
    ~Panel() override = default;

    // === PROPRIEDADES ESPECÍFICAS ===
    void SetBackgroundColor(unsigned color)
    {
        m_BackgroundColor = color;
        SetColor(color); // Keep the render color in sync
        MarkDirty();
    }
    unsigned GetBackgroundColor() const { return m_BackgroundColor; }
    
    void SetBorderColor(unsigned color) { m_BorderColor = color; }
    unsigned GetBorderColor() const { return m_BorderColor; }
    
    void SetBorderWidth(float width) { m_BorderWidth = width; }
    float GetBorderWidth() const { return m_BorderWidth; }
    
    void SetCornerRadius(float radius) { m_CornerRadius = radius; }
    float GetCornerRadius() const { return m_CornerRadius; }
    
    void SetProportionalBorders(bool proportional) { m_ProportionalBorders = proportional; }
    bool GetProportionalBorders() const { return m_ProportionalBorders; }
    
    void SetBorderProportion(float proportion) { m_BorderProportion = proportion; }
    float GetBorderProportion() const { return m_BorderProportion; }
    
    // === RENDERIZAÇÃO ===
    void Render(Drift::RHI::IUIBatcher& batch) override;

private:
    unsigned m_BackgroundColor{0xFF202020}; // Cinza escuro
    unsigned m_BorderColor{0xFF404040};     // Cinza médio
    float m_BorderWidth{1.0f};
    float m_CornerRadius{0.0f};
    bool m_ProportionalBorders{false};      // Bordas proporcionais
    float m_BorderProportion{0.01f};        // Proporção da borda (1% por padrão)
};

} // namespace Drift::UI 