#pragma once

namespace Drift::UI {
class UIElement;

class LayoutEngine {
public:
    LayoutEngine() = default;
    ~LayoutEngine() = default;

    // Calcula layout recursivamente a partir do root
    void Layout(UIElement& root);
};

} // namespace Drift::UI 