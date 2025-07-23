#pragma once

#include "Drift/UI/LayoutTypes.h"
#include <memory>
#include <vector>

namespace Drift::UI {
class UIElement;

class LayoutEngine {
public:
    LayoutEngine() = default;
    ~LayoutEngine() = default;

    // Calcula layout recursivamente a partir do root
    void Layout(UIElement& root);
    
private:
    static void CalculateLayout(UIElement& element, const LayoutRect& availableSpace);
    static LayoutRect CalculateElementRect(const UIElement& element, const LayoutRect& availableSpace, const LayoutConfig& config);
    static void LayoutChildren(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config);
    
    // Métodos específicos para cada tipo de layout
    static void LayoutHorizontal(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config);
    static void LayoutVertical(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config);
    static void LayoutGrid(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config);
    static void LayoutFlow(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config);
};

} // namespace Drift::UI 