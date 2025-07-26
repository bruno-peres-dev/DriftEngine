#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIContext.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>

using namespace Drift::UI;

UIElement::UIElement(UIContext* context)
    : m_Context(context)
{
}

void UIElement::AddChild(const std::shared_ptr<UIElement>& child)
{
    if (!child) return;
    
    // Remove de pai anterior se existir
    child->RemoveFromParent();
    
    child->m_Parent = this;
    m_Children.push_back(child);
    MarkDirty();
    MarkLayoutDirty();
}

void UIElement::RemoveChild(const std::shared_ptr<UIElement>& child)
{
    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end()) {
        (*it)->m_Parent = nullptr;
        m_Children.erase(it);
        MarkDirty();
        MarkLayoutDirty();
    }
}

void UIElement::RemoveFromParent()
{
    if (m_Parent) {
        m_Parent->RemoveChild(shared_from_this());
    }
}

UIElement* UIElement::GetRoot() const
{
    UIElement* current = const_cast<UIElement*>(this);
    while (current->m_Parent) {
        current = current->m_Parent;
    }
    return current;
}

void UIElement::Update(float deltaSeconds)
{
    // Atualiza filhos
    for (auto& child : m_Children) {
        if (child->IsVisible()) {
            child->Update(deltaSeconds);
        }
    }
    
    (void)deltaSeconds;
}

void UIElement::PreRender(const glm::mat4& parentTransform)
{
    if (!m_Visible) return;
    
    // Recalcula transformação se necessário
    if (m_Dirty) {
        RecalculateTransform(parentTransform);
        m_Dirty = false;
    }
    
    // Renderiza filhos
    for (auto& child : m_Children) {
        child->PreRender(m_WorldTransform);
    }
}

void UIElement::PostRender()
{
    if (!m_Visible) return;
    
    // Processa filhos
    for (auto& child : m_Children) {
        child->PostRender();
    }
}

void UIElement::RecalculateTransform(const glm::mat4& parentTransform)
{
    // Calcula transformação local com posição, rotação e escala
    glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(m_Position, 0.0f));
    local = glm::rotate(local, m_Transform.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    local = glm::scale(local, glm::vec3(m_Transform.scale, 1.0f));
    
    // Aplica transformação do pai
    m_WorldTransform = parentTransform * local;
}

glm::vec2 UIElement::GetAbsolutePosition() const
{
    // Usa a matriz de transformação mundial que já foi calculada
    // A posição absoluta é a translação da matriz de transformação
    glm::vec4 worldPos = m_WorldTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    return glm::vec2(worldPos.x, worldPos.y);
}

glm::vec2 UIElement::GetAbsoluteSize() const
{
    // Usa a matriz de transformação mundial para calcular o tamanho absoluto
    // Aplica a escala da matriz de transformação ao tamanho local
    glm::vec4 scaleVec = m_WorldTransform * glm::vec4(m_Size.x, m_Size.y, 0.0f, 0.0f);
    return glm::vec2(std::abs(scaleVec.x), std::abs(scaleVec.y));
}

void UIElement::Render(Drift::RHI::IUIBatcher& batch)
{
    if (!m_Visible || m_Opacity <= 0.0f)
        return;

    // Aplica clipping se habilitado
    bool clippingApplied = false;
    if (m_LayoutProps.clipContent) {
        glm::vec2 absPos = GetAbsolutePosition();
        batch.PushScissorRect(absPos.x, absPos.y, m_Size.x, m_Size.y);
        clippingApplied = true;
    }

    // Só renderiza se tiver tamanho > 0
    if (m_Size.x > 0 && m_Size.y > 0) {
        Drift::Color color = GetRenderColor();

        // Aplica opacidade
        unsigned alpha = static_cast<unsigned>(((color >> 24) & 0xFF) * m_Opacity);
        color = (color & 0x00FFFFFF) | (alpha << 24);

        // Só renderiza se o alpha final for > 0
        if (alpha > 0) {
            Core::Log("[UIElement] Renderizando elemento '" + m_Name + "' com cor 0x" + std::to_string(color) + 
                      " em posição (" + std::to_string(GetAbsolutePosition().x) + ", " + std::to_string(GetAbsolutePosition().y) + 
                      ") tamanho (" + std::to_string(m_Size.x) + ", " + std::to_string(m_Size.y) + ")");
            batch.AddQuad(m_WorldTransform, m_Size.x, m_Size.y, color);
        }
    }

    // Renderiza filhos
    for (auto& child : m_Children) {
        child->Render(batch);
    }
    
    // Remove clipping se foi aplicado
    if (clippingApplied) {
        batch.PopScissorRect();
    }
}

bool UIElement::HitTest(const glm::vec2& point) const
{
    if (!m_Visible || m_Opacity <= 0.0f) return false;
    
    glm::vec2 absPos = GetAbsolutePosition();
    return point.x >= absPos.x && point.x <= absPos.x + m_Size.x &&
           point.y >= absPos.y && point.y <= absPos.y + m_Size.y;
}

UIElement* UIElement::HitTestChildren(const glm::vec2& point)
{
    // Testa filhos em ordem reversa (último renderizado primeiro)
    for (auto it = m_Children.rbegin(); it != m_Children.rend(); ++it) {
        auto& child = *it;
        
        // Testa recursivamente
        UIElement* hit = child->HitTestChildren(point);
        if (hit) return hit;
        
        // Testa este filho
        if (child->HitTest(point)) {
            return child.get();
        }
    }
    
    return nullptr;
}

void UIElement::RecalculateLayout()
{
    // Implementação base - não faz nada
    // Widgets especializados podem sobrescrever este método
    ClearLayoutDirty();
}
