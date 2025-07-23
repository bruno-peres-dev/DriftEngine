#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIContext.h"
#include <glm/gtc/matrix_transform.hpp>
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
    (void)deltaSeconds;
}

void UIElement::PreRender(const glm::mat4& parentTransform)
{
    if (m_Dirty)
    {
        RecalculateLayout();
        m_Dirty = false;
    }

    glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(m_Position, 0.0f));
    local = glm::translate(local, glm::vec3(m_Transform.position, 0.0f));
    local = glm::rotate(local, m_Transform.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    local = glm::scale(local, glm::vec3(m_Transform.scale, 1.0f));
    m_WorldTransform = parentTransform * local;

    for (auto& c : m_Children)
        c->PreRender(m_WorldTransform);
}

void UIElement::PostRender()
{
    for (auto& c : m_Children)
        c->PostRender();
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
    if (!m_Visible || m_Opacity <= 0.0f)
        return;

    // Só renderiza se tiver tamanho > 0
    if (m_Size.x > 0 && m_Size.y > 0) {
        glm::vec2 absPos = GetAbsolutePosition();
        unsigned color = GetRenderColor();
        unsigned alpha = (unsigned)(((color >> 24) & 0xFF) * m_Opacity);
        color = (color & 0x00FFFFFF) | (alpha << 24);
        batch.AddRect(absPos.x, absPos.y, m_Size.x, m_Size.y, color);
    }

    for (auto& child : m_Children)
        child->Render(batch);
}
