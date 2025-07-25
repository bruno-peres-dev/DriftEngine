#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/UI/LayoutTypes.h"

namespace Drift::UI {

class StackPanel : public UIElement {
public:
    StackPanel(UIContext* context);
    virtual ~StackPanel() = default;

    // Stack properties
    void SetDirection(StackDirection direction);
    StackDirection GetDirection() const { return m_Direction; }
    
    void SetSpacing(float spacing);
    float GetSpacing() const { return m_Spacing; }

    // Layout override
    virtual void RecalculateLayout() override;
    
    // Render override
    virtual void Render(Drift::RHI::IUIBatcher& batch) override;

protected:
    StackDirection m_Direction = StackDirection::Vertical;
    float m_Spacing = 0.0f;
};

} // namespace Drift::UI 