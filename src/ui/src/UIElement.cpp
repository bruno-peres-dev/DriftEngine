#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIContext.h"
#include <algorithm>

using namespace Drift::UI;

UIElement::UIElement(UIContext* context)
    : m_Context(context)
{
}

void UIElement::AddChild(const std::shared_ptr<UIElement>& child)
{
    if (!child) return;
    child->m_Parent = this;
    m_Children.push_back(child);
    MarkDirty();
}

void UIElement::RemoveChild(const std::shared_ptr<UIElement>& child)
{
    m_Children.erase(std::remove(m_Children.begin(), m_Children.end(), child), m_Children.end());
    MarkDirty();
}

void UIElement::Update(float deltaSeconds)
{
    if (m_Dirty)
    {
        RecalculateLayout();
        m_Dirty = false;
    }

    // Atualiza filhos
    for (auto& c : m_Children)
        c->Update(deltaSeconds);
}

void UIElement::RecalculateLayout()
{
    // Placeholder: layout trivial (posição absoluta) - será substituído por LayoutEngine
}

void UIElement::Render(Drift::RHI::IUIBatcher& batch)
{
    Drift::Core::Log("[UIElement] Rendering element - pos=(" + std::to_string(m_Position.x) + "," + 
                     std::to_string(m_Position.y) + ") size=(" + std::to_string(m_Size.x) + "," + 
                     std::to_string(m_Size.y) + ") color=0x" + std::to_string(m_Color));
    
    batch.AddRect(m_Position.x, m_Position.y, m_Size.x, m_Size.y, m_Color);

    // Desenha filhos
    for (auto& child : m_Children)
        child->Render(batch);
} 