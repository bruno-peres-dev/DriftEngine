#include "Drift/UI/Widgets/Grid.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/Core/Log.h"
#include <algorithm>

namespace Drift::UI {

Grid::Grid(UIContext* context) : UIElement(context) {
    SetName("Grid");
    
    // Set default layout properties for grid
    LayoutProperties props;
    props.layoutType = LayoutType::Grid;
    SetLayoutProperties(props);
}

void Grid::SetRowDefinitions(const std::vector<GridUnit>& rows) {
    m_RowDefinitions = rows;
    MarkLayoutDirty();
}

void Grid::SetColumnDefinitions(const std::vector<GridUnit>& columns) {
    m_ColumnDefinitions = columns;
    MarkLayoutDirty();
}

void Grid::SetGridDefinition(const GridDefinition& definition) {
    m_RowDefinitions = definition.rows;
    m_ColumnDefinitions = definition.columns;
    MarkLayoutDirty();
}

void Grid::RecalculateLayout() {
    if (!IsLayoutDirty()) return;
    
    const auto& children = GetChildren();
    if (children.empty()) {
        ClearLayoutDirty();
        return;
    }
    
    auto props = GetLayoutProperties();
    auto availableRect = LayoutRect(0, 0, GetSize().x, GetSize().y);
    
    // Apply padding
    availableRect.x += props.padding.x; // left
    availableRect.y += props.padding.y; // top
    availableRect.width -= props.padding.x + props.padding.z; // left + right
    availableRect.height -= props.padding.y + props.padding.w; // top + bottom
    
    // Calculate grid sizes
    auto rowSizes = CalculateGridSizes(m_RowDefinitions, availableRect.height);
    auto columnSizes = CalculateGridSizes(m_ColumnDefinitions, availableRect.width);
    
    // Position children in grid
    for (auto& child : children) {
        if (!child->IsVisible()) continue;
        
        auto childProps = child->GetLayoutProperties();
        auto childSize = child->GetSize();
        
        // Get grid position
        int row = childProps.gridRow;
        int col = childProps.gridColumn;
        int rowSpan = childProps.gridRowSpan;
        int colSpan = childProps.gridColumnSpan;
        
        // Clamp to grid bounds
        row = std::max(0, std::min(row, static_cast<int>(rowSizes.size()) - 1));
        col = std::max(0, std::min(col, static_cast<int>(columnSizes.size()) - 1));
        rowSpan = std::min(rowSpan, static_cast<int>(rowSizes.size()) - row);
        colSpan = std::min(colSpan, static_cast<int>(columnSizes.size()) - col);
        
        // Calculate cell bounds
        float cellX = availableRect.x;
        float cellY = availableRect.y;
        float cellWidth = 0.0f;
        float cellHeight = 0.0f;
        
        // Calculate X position and width
        for (int i = 0; i < col; ++i) {
            cellX += columnSizes[i];
        }
        for (int i = 0; i < colSpan; ++i) {
            cellWidth += columnSizes[col + i];
        }
        
        // Calculate Y position and height
        for (int i = 0; i < row; ++i) {
            cellY += rowSizes[i];
        }
        for (int i = 0; i < rowSpan; ++i) {
            cellHeight += rowSizes[row + i];
        }
        
        // Apply margins
        float marginLeft = childProps.margin.x;
        float marginTop = childProps.margin.y;
        float marginRight = childProps.margin.z;
        float marginBottom = childProps.margin.w;
        
        // Calculate final position and size
        float x = cellX + marginLeft;
        float y = cellY + marginTop;
        float width = cellWidth - marginLeft - marginRight;
        float height = cellHeight - marginTop - marginBottom;
        
        // Handle alignment
        if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Center) {
            x = cellX + (cellWidth - childSize.x) * 0.5f;
            width = childSize.x;
        } else if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Right) {
            x = cellX + cellWidth - childSize.x - marginRight;
            width = childSize.x;
        } else if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Stretch) {
            // Use calculated width
        } else { // Left
            width = childSize.x;
        }
        
        if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Center) {
            y = cellY + (cellHeight - childSize.y) * 0.5f;
            height = childSize.y;
        } else if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Bottom) {
            y = cellY + cellHeight - childSize.y - marginBottom;
            height = childSize.y;
        } else if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Stretch) {
            // Use calculated height
        } else { // Top
            height = childSize.y;
        }
        
        // Set child position and size
        child->SetPosition(glm::vec2(x, y));
        child->SetSize(glm::vec2(width, height));
    }
    
    ClearLayoutDirty();
}

void Grid::CalculateGridLayout() {
    // This method is called by the layout engine to calculate grid cell positions
    RecalculateLayout();
}

std::vector<float> Grid::CalculateGridSizes(const std::vector<GridUnit>& definitions, float availableSize) {
    std::vector<float> sizes;
    
    if (definitions.empty()) {
        // Default to single auto-sized cell
        sizes.push_back(availableSize);
        return sizes;
    }
    
    // First pass: calculate fixed and auto sizes
    float fixedSize = 0.0f;
    float starWeight = 0.0f;
    int autoCount = 0;
    int starCount = 0;
    
    for (const auto& def : definitions) {
        switch (def.type) {
            case GridUnitType::Fixed:
                fixedSize += def.value;
                sizes.push_back(def.value);
                break;
            case GridUnitType::Auto:
                autoCount++;
                sizes.push_back(0.0f); // Placeholder
                break;
            case GridUnitType::Star:
                starWeight += def.value;
                starCount++;
                sizes.push_back(0.0f); // Placeholder
                break;
        }
    }
    
    // Calculate remaining space for auto and star units
    float remainingSpace = availableSize - fixedSize;
    
    if (remainingSpace <= 0.0f) {
        // Not enough space, distribute equally
        float equalSize = availableSize / definitions.size();
        for (auto& size : sizes) {
            if (size == 0.0f) size = equalSize;
        }
        return sizes;
    }
    
    // Second pass: distribute remaining space
    int sizeIndex = 0;
    for (const auto& def : definitions) {
        if (def.type == GridUnitType::Auto) {
            // Auto units get equal share of remaining space
            sizes[sizeIndex] = remainingSpace / autoCount;
        } else if (def.type == GridUnitType::Star) {
            // Star units get proportional share. Safeguard against zero weight.
            if (starWeight == 0.0f && starCount > 0) {
                sizes[sizeIndex] = remainingSpace / starCount;
            } else {
                sizes[sizeIndex] = (def.value / starWeight) * remainingSpace;
            }
        }
        sizeIndex++;
    }
    
    return sizes;
}

void Grid::Render(Drift::RHI::IUIBatcher& batch)
{
    if (!IsVisible() || GetOpacity() <= 0.0f)
        return;

    // Render background if we have a color
    if (GetRenderColor() != 0x00000000) {
        glm::vec2 absPos = GetAbsolutePosition();
        Drift::Color color = GetRenderColor();
        
        // Apply opacity
        unsigned alpha = static_cast<unsigned>(((color >> 24) & 0xFF) * GetOpacity());
        color = (color & 0x00FFFFFF) | (alpha << 24);
        
        batch.AddRect(absPos.x, absPos.y, GetSize().x, GetSize().y, color);
    }

    // Render children
    UIElement::Render(batch);
}

} // namespace Drift::UI 