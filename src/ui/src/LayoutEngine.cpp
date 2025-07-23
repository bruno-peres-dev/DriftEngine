#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/LayoutTypes.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <vector>
#include <glm/vec2.hpp>

using namespace Drift::UI;

void LayoutEngine::Layout(UIElement& root)
{
    // Calcula o layout recursivamente começando pela raiz
    LayoutRect availableSpace = {0.0f, 0.0f, 800.0f, 600.0f}; // TODO: obter do viewport
    CalculateLayout(root, availableSpace);
}

void LayoutEngine::CalculateLayout(UIElement& element, const LayoutRect& availableSpace)
{
    // Configuração padrão de layout
    LayoutConfig config;
    
    // Aplica margens ao espaço disponível
    LayoutRect contentSpace = availableSpace;
    contentSpace.x += config.margin.left;
    contentSpace.y += config.margin.top;
    contentSpace.width -= (config.margin.left + config.margin.right);
    contentSpace.height -= (config.margin.top + config.margin.bottom);
    
    // Aplica padding
    LayoutRect paddingSpace = contentSpace;
    paddingSpace.x += config.padding.left;
    paddingSpace.y += config.padding.top;
    paddingSpace.width -= (config.padding.left + config.padding.right);
    paddingSpace.height -= (config.padding.top + config.padding.bottom);
    
    // Calcula posição e tamanho do elemento baseado no alinhamento
    LayoutRect elementRect = CalculateElementRect(element, paddingSpace, config);
    element.SetPosition(glm::vec2(elementRect.x, elementRect.y));
    element.SetSize(glm::vec2(elementRect.width, elementRect.height));
    
    // Calcula layout dos filhos
    auto& children = element.GetChildren();
    if (!children.empty() && config.type != LayoutType::None) {
        LayoutChildren(children, elementRect, config);
    }
    
    // Recursão para filhos
    for (auto& child : children) {
        CalculateLayout(*child, elementRect);
    }
}

LayoutRect LayoutEngine::CalculateElementRect(const UIElement& element, const LayoutRect& availableSpace, const LayoutConfig& config)
{
    LayoutRect rect;
    
    // Se deve expandir para preencher
    if (config.expandToFill) {
        rect = availableSpace;
    } else {
        // Usa tamanho atual do elemento como preferido
        glm::vec2 size = element.GetSize();
        rect.width = size.x;
        rect.height = size.y;
        
        // Aplica alinhamento horizontal
        switch (config.horizontalAlign) {
            case HorizontalAlignment::Left:
                rect.x = availableSpace.x;
                break;
            case HorizontalAlignment::Center:
                rect.x = availableSpace.x + (availableSpace.width - rect.width) * 0.5f;
                break;
            case HorizontalAlignment::Right:
                rect.x = availableSpace.GetRight() - rect.width;
                break;
            case HorizontalAlignment::Stretch:
                rect.x = availableSpace.x;
                rect.width = availableSpace.width;
                break;
        }
        
        // Aplica alinhamento vertical
        switch (config.verticalAlign) {
            case VerticalAlignment::Top:
                rect.y = availableSpace.y;
                break;
            case VerticalAlignment::Center:
                rect.y = availableSpace.y + (availableSpace.height - rect.height) * 0.5f;
                break;
            case VerticalAlignment::Bottom:
                rect.y = availableSpace.GetBottom() - rect.height;
                break;
            case VerticalAlignment::Stretch:
                rect.y = availableSpace.y;
                rect.height = availableSpace.height;
                break;
        }
    }
    
    return rect;
}

void LayoutEngine::LayoutChildren(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config)
{
    if (children.empty()) return;
    
    switch (config.type) {
        case LayoutType::Horizontal:
            LayoutHorizontal(children, parentRect, config);
            break;
        case LayoutType::Vertical:
            LayoutVertical(children, parentRect, config);
            break;
        case LayoutType::Grid:
            LayoutGrid(children, parentRect, config);
            break;
        case LayoutType::Flow:
            LayoutFlow(children, parentRect, config);
            break;
        default:
            break;
    }
}

void LayoutEngine::LayoutHorizontal(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config)
{
    float currentX = parentRect.x + config.padding.left;
    float availableHeight = parentRect.height - config.padding.top - config.padding.bottom;
    
    for (size_t i = 0; i < children.size(); ++i) {
        auto& child = children[i];
        LayoutConfig childConfig; // Configuração padrão
        
        // Calcula altura do filho
        float childHeight = availableHeight;
        if (!childConfig.expandToFill) {
            glm::vec2 childSize = child->GetSize();
            childHeight = std::min(childSize.y, availableHeight);
        }
        
        // Aplica alinhamento vertical
        float childY = parentRect.y + config.padding.top;
        if (childConfig.verticalAlign == VerticalAlignment::Center) {
            childY += (availableHeight - childHeight) * 0.5f;
        } else if (childConfig.verticalAlign == VerticalAlignment::Bottom) {
            childY += availableHeight - childHeight;
        }
        
        // Define posição e tamanho
        glm::vec2 childSize = child->GetSize();
        child->SetPosition(glm::vec2(currentX, childY));
        child->SetSize(glm::vec2(childSize.x, childHeight));
        
        // Avança para próximo elemento
        currentX += childSize.x + config.spacing;
    }
}

void LayoutEngine::LayoutVertical(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config)
{
    float currentY = parentRect.y + config.padding.top;
    float availableWidth = parentRect.width - config.padding.left - config.padding.right;
    
    for (size_t i = 0; i < children.size(); ++i) {
        auto& child = children[i];
        LayoutConfig childConfig; // Configuração padrão
        
        // Calcula largura do filho
        float childWidth = availableWidth;
        if (!childConfig.expandToFill) {
            glm::vec2 childSize = child->GetSize();
            childWidth = std::min(childSize.x, availableWidth);
        }
        
        // Aplica alinhamento horizontal
        float childX = parentRect.x + config.padding.left;
        if (childConfig.horizontalAlign == HorizontalAlignment::Center) {
            childX += (availableWidth - childWidth) * 0.5f;
        } else if (childConfig.horizontalAlign == HorizontalAlignment::Right) {
            childX += availableWidth - childWidth;
        }
        
        // Define posição e tamanho
        glm::vec2 childSize = child->GetSize();
        child->SetPosition(glm::vec2(childX, currentY));
        child->SetSize(glm::vec2(childWidth, childSize.y));
        
        // Avança para próximo elemento
        currentY += childSize.y + config.spacing;
    }
}

void LayoutEngine::LayoutGrid(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config)
{
    // Layout de grade simples - 2 colunas por padrão
    const int columns = 2;
    const int rows = (children.size() + columns - 1) / columns;
    
    float cellWidth = (parentRect.width - config.padding.left - config.padding.right) / columns;
    float cellHeight = (parentRect.height - config.padding.top - config.padding.bottom) / rows;
    
    for (size_t i = 0; i < children.size(); ++i) {
        int row = static_cast<int>(i) / columns;
        int col = static_cast<int>(i) % columns;
        
        float x = parentRect.x + config.padding.left + col * cellWidth;
        float y = parentRect.y + config.padding.top + row * cellHeight;
        
        children[i]->SetPosition(glm::vec2(x, y));
        children[i]->SetSize(glm::vec2(cellWidth - config.spacing, cellHeight - config.spacing));
    }
}

void LayoutEngine::LayoutFlow(const std::vector<std::shared_ptr<UIElement>>& children, const LayoutRect& parentRect, const LayoutConfig& config)
{
    float currentX = parentRect.x + config.padding.left;
    float currentY = parentRect.y + config.padding.top;
    float maxHeightInRow = 0.0f;
    float availableWidth = parentRect.width - config.padding.left - config.padding.right;
    
    for (size_t i = 0; i < children.size(); ++i) {
        auto& child = children[i];
        glm::vec2 childSize = child->GetSize();
        float childWidth = childSize.x;
        float childHeight = childSize.y;
        
        // Verifica se precisa quebrar linha
        if (currentX + childWidth > parentRect.GetRight() - config.padding.right && currentX > parentRect.x + config.padding.left) {
            currentX = parentRect.x + config.padding.left;
            currentY += maxHeightInRow + config.spacing;
            maxHeightInRow = 0.0f;
        }
        
        child->SetPosition(glm::vec2(currentX, currentY));
        child->SetSize(glm::vec2(childWidth, childHeight));
        
        currentX += childWidth + config.spacing;
        maxHeightInRow = std::max(maxHeightInRow, childHeight);
    }
} 