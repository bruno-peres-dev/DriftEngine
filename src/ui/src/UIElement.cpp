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

glm::vec2 UIElement::GetAbsolutePosition() const
{
    glm::vec2 absolutePos = m_Position;
    UIElement* current = m_Parent;
    
    // Soma as posições de todos os pais
    while (current) {
        absolutePos += current->m_Position;
        current = current->GetParent();
    }
    
    return absolutePos;
}

void UIElement::Render(Drift::RHI::IUIBatcher& batch)
{
    // Só renderiza se tiver tamanho > 0 e não for transparente
    if (m_Size.x > 0 && m_Size.y > 0 && ((m_Color >> 24) & 0xFF) > 0) {
        glm::vec2 absPos = GetAbsolutePosition();
        batch.AddRect(absPos.x, absPos.y, m_Size.x, m_Size.y, GetRenderColor());
    }

    // Desenha filhos
    for (auto& child : m_Children)
        child->Render(batch);
} 