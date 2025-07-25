#include "Drift/UI/Widgets/StackPanel.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/Core/Log.h"

namespace Drift::UI {

StackPanel::StackPanel(UIContext* context) : UIElement(context) {
    SetName("StackPanel");
    
    // Set default layout properties for stack panel
    LayoutProperties props;
    props.layoutType = LayoutType::Stack;
    props.stackDirection = StackDirection::Vertical;
    SetLayoutProperties(props);
}

void StackPanel::SetDirection(StackDirection direction) {
    m_Direction = direction;
    
    // Update layout properties
    auto props = GetLayoutProperties();
    props.stackDirection = direction;
    SetLayoutProperties(props);
    
    MarkLayoutDirty();
}

void StackPanel::SetSpacing(float spacing) {
    m_Spacing = spacing;
    
    // Update layout properties
    auto props = GetLayoutProperties();
    props.stackSpacing = spacing;
    SetLayoutProperties(props);
    
    MarkLayoutDirty();
}

void StackPanel::RecalculateLayout() {
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
    
    if (m_Direction == StackDirection::Vertical) {
        // Vertical stacking
        float currentY = availableRect.y;
        float maxWidth = 0.0f;
        
        for (auto& child : children) {
            if (!child->IsVisible()) continue;
            
            auto childProps = child->GetLayoutProperties();
            auto childSize = child->GetSize();
            
            // Apply margins
            float marginLeft = childProps.margin.x;
            float marginTop = childProps.margin.y;
            float marginRight = childProps.margin.z;
            float marginBottom = childProps.margin.w;
            
            // Calculate position
            float x = availableRect.x + marginLeft;
            float y = currentY + marginTop;
            
            // Handle alignment
            if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Center) {
                x = availableRect.x + (availableRect.width - childSize.x) * 0.5f;
            } else if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Right) {
                x = availableRect.x + availableRect.width - childSize.x - marginRight;
            } else if (childProps.horizontalAlign == LayoutProperties::HorizontalAlign::Stretch) {
                x = availableRect.x + marginLeft;
                childSize.x = availableRect.width - marginLeft - marginRight;
            }
            
            // Set child position and size
            child->SetPosition(glm::vec2(x, y));
            child->SetSize(childSize);
            
            // Update position for next child
            currentY = y + childSize.y + marginBottom + m_Spacing;
            maxWidth = std::max(maxWidth, childSize.x + marginLeft + marginRight);
        }
        
        // Update panel size if needed
        if (props.verticalAlign == LayoutProperties::VerticalAlign::Stretch) {
            SetSize(glm::vec2(GetSize().x, currentY - availableRect.y + props.padding.y + props.padding.w));
        }
        
    } else {
        // Horizontal stacking
        float currentX = availableRect.x;
        float maxHeight = 0.0f;
        
        for (auto& child : children) {
            if (!child->IsVisible()) continue;
            
            auto childProps = child->GetLayoutProperties();
            auto childSize = child->GetSize();
            
            // Apply margins
            float marginLeft = childProps.margin.x;
            float marginTop = childProps.margin.y;
            float marginRight = childProps.margin.z;
            float marginBottom = childProps.margin.w;
            
            // Calculate position
            float x = currentX + marginLeft;
            float y = availableRect.y + marginTop;
            
            // Handle alignment
            if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Center) {
                y = availableRect.y + (availableRect.height - childSize.y) * 0.5f;
            } else if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Bottom) {
                y = availableRect.y + availableRect.height - childSize.y - marginBottom;
            } else if (childProps.verticalAlign == LayoutProperties::VerticalAlign::Stretch) {
                y = availableRect.y + marginTop;
                childSize.y = availableRect.height - marginTop - marginBottom;
            }
            
            // Set child position and size
            child->SetPosition(glm::vec2(x, y));
            child->SetSize(childSize);
            
            // Update position for next child
            currentX = x + childSize.x + marginRight + m_Spacing;
            maxHeight = std::max(maxHeight, childSize.y + marginTop + marginBottom);
        }
        
        // Update panel size if needed
        if (props.horizontalAlign == LayoutProperties::HorizontalAlign::Stretch) {
            SetSize(glm::vec2(currentX - availableRect.x + props.padding.x + props.padding.z, GetSize().y));
        }
    }
    
    ClearLayoutDirty();
}

void StackPanel::Render(Drift::RHI::IUIBatcher& batch)
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