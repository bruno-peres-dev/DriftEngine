#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/Core/Color.h"
#include <string>

namespace Drift::UI {

class Panel : public UIElement {
public:
    explicit Panel(UIContext* context);
    ~Panel() override = default;

    // === PROPRIEDADES ESPECÍFICAS ===
    void SetBackgroundColor(Drift::Color color)
    {
        m_BackgroundColor = color;
        SetColor(color); // Keep the render color in sync
        MarkDirty();
    }
    Drift::Color GetBackgroundColor() const { return m_BackgroundColor; }
    
    void SetBorderColor(Drift::Color color) { m_BorderColor = color; }
    Drift::Color GetBorderColor() const { return m_BorderColor; }
    
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
    Drift::Color m_BackgroundColor{0xFF202020}; // Cinza escuro
    Drift::Color m_BorderColor{0xFF404040};     // Cinza médio
    float m_BorderWidth{1.0f};
    float m_CornerRadius{0.0f};
    bool m_ProportionalBorders{false};      // Bordas proporcionais
    float m_BorderProportion{0.01f};        // Proporção da borda (1% por padrão)
};

} // namespace Drift::UI 