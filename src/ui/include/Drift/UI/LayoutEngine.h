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

    // === LAYOUT PRINCIPAL ===
    void Layout(UIElement& root);
    
    // === MEDIDA E ARRANJO ===
    static LayoutMeasure MeasureElement(UIElement& element, const LayoutRect& availableSpace);
    static void ArrangeElement(UIElement& element, const LayoutRect& finalRect);

private:
    // === LAYOUT RECURSIVO ===
    static void CalculateLayout(UIElement& element, const LayoutRect& availableSpace);
    static LayoutRect CalculateElementRect(const UIElement& element, const LayoutRect& availableSpace, const LayoutProperties& layoutProps);
    static void LayoutChildren(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutProperties& layoutProps);
    
    // === LAYOUTS BÁSICOS ===
    static void LayoutHorizontal(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutProperties& layoutProps);
    static void LayoutVertical(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutProperties& layoutProps);
    
    // === UTILITÁRIOS ===
    static LayoutRect ApplyMargins(const LayoutRect& rect, const LayoutMargins& margins);
    static LayoutRect ApplyPadding(const LayoutRect& rect, const LayoutMargins& padding);
    static glm::vec2 ClampSize(const glm::vec2& size, const glm::vec2& minSize, const glm::vec2& maxSize);
    static bool IsElementVisible(const UIElement& element);
};

} // namespace Drift::UI 