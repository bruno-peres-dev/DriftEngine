#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/LayoutTypes.h"
#include "Drift/Core/Log.h"
#include <algorithm>

using namespace Drift::UI;

void LayoutEngine::Layout(UIElement& root)
{
    // Sempre recalcula layout para o root
    if (root.IsLayoutDirty()) {
        LayoutRect availableSpace(0, 0, root.GetSize().x, root.GetSize().y);
        CalculateLayout(root, availableSpace);
    }
}

LayoutMeasure LayoutEngine::MeasureElement(UIElement& element, const LayoutRect& availableSpace)
{
    const auto& layoutProps = element.GetLayoutProperties();
    
    // Medida base do elemento
    glm::vec2 size = element.GetSize();
    
    // Aplica constraints de tamanho
    size = ClampSize(size, layoutProps.minSize, layoutProps.maxSize);
    
    return LayoutMeasure(size.x, size.y);
}

void LayoutEngine::ArrangeElement(UIElement& element, const LayoutRect& finalRect)
{
    element.SetPosition(finalRect.GetPosition());
    element.SetSize(finalRect.GetSize());
}

void LayoutEngine::CalculateLayout(UIElement& element, const LayoutRect& availableSpace)
{
    if (!IsElementVisible(element)) return;
    
    const auto& layoutProps = element.GetLayoutProperties();
    
    // Para LayoutType::Absolute, não aplica layout automático
    if (layoutProps.layoutType == LayoutType::Absolute) {
        // Apenas processa filhos recursivamente, mas não modifica este elemento
        auto& children = element.GetChildren();
        for (auto& child : children) {
            if (IsElementVisible(*child)) {
                CalculateLayout(*child, availableSpace);
            }
        }
        return;
    }
    
    // Aplica margens ao espaço disponível
    LayoutMargins margins;
    margins.left = layoutProps.margin.x;
    margins.top = layoutProps.margin.y;
    margins.right = layoutProps.margin.z;
    margins.bottom = layoutProps.margin.w;
    LayoutRect contentSpace = ApplyMargins(availableSpace, margins);
    
    // Aplica padding
    LayoutMargins padding;
    padding.left = layoutProps.padding.x;
    padding.top = layoutProps.padding.y;
    padding.right = layoutProps.padding.z;
    padding.bottom = layoutProps.padding.w;
    LayoutRect paddingSpace = ApplyPadding(contentSpace, padding);
    
    // Calcula posição e tamanho do elemento baseado no alinhamento
    LayoutRect elementRect = CalculateElementRect(element, paddingSpace, layoutProps);
    ArrangeElement(element, elementRect);
    
    // Calcula layout dos filhos
    auto& children = element.GetChildren();
    if (!children.empty()) {
        LayoutChildren(children, elementRect, layoutProps);
    }
    
    // Recursão para filhos
    for (auto& child : children) {
        if (IsElementVisible(*child)) {
            CalculateLayout(*child, elementRect);
        }
    }
}

LayoutRect LayoutEngine::CalculateElementRect(const UIElement& element, const LayoutRect& availableSpace, const LayoutProperties& layoutProps)
{
    LayoutRect rect;
    glm::vec2 size = element.GetSize();
    
    // Aplica constraints de tamanho
    size = ClampSize(size, layoutProps.minSize, layoutProps.maxSize);
    
    // Aplica alinhamento horizontal
    switch (layoutProps.horizontalAlign) {
        case LayoutProperties::HorizontalAlign::Left:
            rect.x = availableSpace.x;
            rect.width = size.x;
            break;
        case LayoutProperties::HorizontalAlign::Center:
            rect.x = availableSpace.x + (availableSpace.width - size.x) * 0.5f;
            rect.width = size.x;
            break;
        case LayoutProperties::HorizontalAlign::Right:
            rect.x = availableSpace.x + availableSpace.width - size.x;
            rect.width = size.x;
            break;
        case LayoutProperties::HorizontalAlign::Stretch:
            rect.x = availableSpace.x;
            rect.width = availableSpace.width;
            break;
    }
    
    // Aplica alinhamento vertical
    switch (layoutProps.verticalAlign) {
        case LayoutProperties::VerticalAlign::Top:
            rect.y = availableSpace.y;
            rect.height = size.y;
            break;
        case LayoutProperties::VerticalAlign::Center:
            rect.y = availableSpace.y + (availableSpace.height - size.y) * 0.5f;
            rect.height = size.y;
            break;
        case LayoutProperties::VerticalAlign::Bottom:
            rect.y = availableSpace.y + availableSpace.height - size.y;
            rect.height = size.y;
            break;
        case LayoutProperties::VerticalAlign::Stretch:
            rect.y = availableSpace.y;
            rect.height = availableSpace.height;
            break;
    }
    
    return rect;
}

void LayoutEngine::LayoutChildren(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutProperties& layoutProps)
{
    // Respeita o tipo de layout
    switch (layoutProps.layoutType) {
        case LayoutType::Stack:
            LayoutVertical(children, parentRect, layoutProps);
            break;
        case LayoutType::Grid:
            // TODO: Implementar layout de grid
            break;
        case LayoutType::Absolute:
            break;
    }
}

void LayoutEngine::LayoutHorizontal(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutProperties& layoutProps)
{
    float currentX = parentRect.x;
    float maxHeight = 0.0f;
    
    for (auto& child : children) {
        if (!child->IsVisible()) continue;
        
        auto childProps = child->GetLayoutProperties();
        auto childSize = child->GetSize();
        
        // Aplica margens
        float marginLeft = childProps.margin.x;
        float marginTop = childProps.margin.y;
        float marginRight = childProps.margin.z;
        float marginBottom = childProps.margin.w;
        
        // Calcula posição
        float x = currentX + marginLeft;
        float y = parentRect.y + marginTop;
        
        // Handle alignment
        if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Center) {
            y = parentRect.y + (parentRect.height - childSize.y) * 0.5f;
        } else if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Bottom) {
            y = parentRect.y + parentRect.height - childSize.y - marginBottom;
        } else if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Stretch) {
            y = parentRect.y + marginTop;
            childSize.y = parentRect.height - marginTop - marginBottom;
        }
        
        // Set child position and size
        child->SetPosition(glm::vec2(x, y));
        child->SetSize(childSize);
        
        // Update position for next child
        currentX = x + childSize.x + marginRight;
        maxHeight = std::max(maxHeight, childSize.y + marginTop + marginBottom);
    }
}

void LayoutEngine::LayoutVertical(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutProperties& layoutProps)
{
    float currentY = parentRect.y;
    float maxWidth = 0.0f;
    
    for (auto& child : children) {
        if (!child->IsVisible()) continue;
        
        auto childProps = child->GetLayoutProperties();
        auto childSize = child->GetSize();
        
        // Aplica margens
        float marginLeft = childProps.margin.x;
        float marginTop = childProps.margin.y;
        float marginRight = childProps.margin.z;
        float marginBottom = childProps.margin.w;
        
        // Calcula posição
        float x = parentRect.x + marginLeft;
        float y = currentY + marginTop;
        
        // Handle alignment
        if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Center) {
            x = parentRect.x + (parentRect.width - childSize.x) * 0.5f;
        } else if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Right) {
            x = parentRect.x + parentRect.width - childSize.x - marginRight;
        } else if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Stretch) {
            x = parentRect.x + marginLeft;
            childSize.x = parentRect.width - marginLeft - marginRight;
        }
        
        // Set child position and size
        child->SetPosition(glm::vec2(x, y));
        child->SetSize(childSize);
        
        // Update position for next child
        currentY = y + childSize.y + marginBottom;
        maxWidth = std::max(maxWidth, childSize.x + marginLeft + marginRight);
    }
}

// Helper functions
LayoutRect LayoutEngine::ApplyMargins(const LayoutRect& rect, const LayoutMargins& margins)
{
    return LayoutRect(
        rect.x + margins.left,
        rect.y + margins.top,
        rect.width - margins.left - margins.right,
        rect.height - margins.top - margins.bottom
    );
}

LayoutRect LayoutEngine::ApplyPadding(const LayoutRect& rect, const LayoutMargins& padding)
{
    return LayoutRect(
        rect.x + padding.left,
        rect.y + padding.top,
        rect.width - padding.left - padding.right,
        rect.height - padding.top - padding.bottom
    );
}

glm::vec2 LayoutEngine::ClampSize(const glm::vec2& size, const glm::vec2& minSize, const glm::vec2& maxSize)
{
    return glm::vec2(
        std::clamp(size.x, minSize.x, maxSize.x),
        std::clamp(size.y, minSize.y, maxSize.y)
    );
}

bool LayoutEngine::IsElementVisible(const UIElement& element)
{
    return element.IsVisible() && element.GetSize().x > 0 && element.GetSize().y > 0;
} 