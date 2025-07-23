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
    
    // Debug: Log quando um filho é adicionado
    Core::Log("[UIElement] AddChild - Adicionando filho:");
    Core::Log("[UIElement] - Posição do filho: (" + std::to_string(child->GetPosition().x) + ", " + std::to_string(child->GetPosition().y) + ")");
    Core::Log("[UIElement] - Tamanho do filho: (" + std::to_string(child->GetSize().x) + ", " + std::to_string(child->GetSize().y) + ")");
    Core::Log("[UIElement] - Cor do filho: 0x" + std::to_string(child->GetColor()));
    Core::Log("[UIElement] - Posição do pai: (" + std::to_string(m_Position.x) + ", " + std::to_string(m_Position.y) + ")");
    
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
    
    // Debug: Log para verificar o cálculo
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 100 == 0) { // Log a cada 100 chamadas
        Core::Log("[UIElement] GetAbsolutePosition - Posição local: (" + 
                 std::to_string(m_Position.x) + ", " + std::to_string(m_Position.y) + 
                 ") -> Absoluta: (" + std::to_string(absolutePos.x) + ", " + std::to_string(absolutePos.y) + ")");
    }
    
    return absolutePos;
}

void UIElement::Render(Drift::RHI::IUIBatcher& batch)
{
    // Debug: Log para identificar qual elemento está sendo renderizado
    static int renderElementCount = 0;
    renderElementCount++;
    if (renderElementCount % 10 == 0) { // Log a cada 10 renders
        Core::Log("[UIElement] Render - Elemento sendo renderizado:");
        Core::Log("[UIElement] - Posição local: (" + std::to_string(m_Position.x) + ", " + std::to_string(m_Position.y) + ")");
        Core::Log("[UIElement] - Tamanho: (" + std::to_string(m_Size.x) + ", " + std::to_string(m_Size.y) + ")");
        Core::Log("[UIElement] - Cor: 0x" + std::to_string(m_Color));
        Core::Log("[UIElement] - Número de filhos: " + std::to_string(m_Children.size()));
        Core::Log("[UIElement] - É elemento raiz: " + std::string(m_Parent == nullptr ? "SIM" : "NÃO"));
    }
    
    // Só renderiza se tiver tamanho > 0 e não for transparente
    if (m_Size.x > 0 && m_Size.y > 0 && ((m_Color >> 24) & 0xFF) > 0) {
        static int renderCount = 0;
        renderCount++;
        if (renderCount % 60 == 0) { // Log a cada segundo
            glm::vec2 absPos = GetAbsolutePosition();
            Core::Log("[UIElement] Renderizando elemento na posição absoluta (" + 
                     std::to_string(absPos.x) + ", " + std::to_string(absPos.y) + 
                     ") com tamanho (" + std::to_string(m_Size.x) + ", " + std::to_string(m_Size.y) + 
                     ") e cor 0x" + std::to_string(GetRenderColor()));
        }
        
        glm::vec2 absPos = GetAbsolutePosition();
        
        // Debug: Log detalhado para cada render
        static int detailedRenderCount = 0;
        detailedRenderCount++;
        if (detailedRenderCount % 10 == 0) { // Log a cada 10 renders
            Core::Log("[UIElement] DEBUG RENDER - Elemento:");
            Core::Log("[UIElement] - Posição local: (" + std::to_string(m_Position.x) + ", " + std::to_string(m_Position.y) + ")");
            Core::Log("[UIElement] - Posição absoluta calculada: (" + std::to_string(absPos.x) + ", " + std::to_string(absPos.y) + ")");
            Core::Log("[UIElement] - Tamanho: (" + std::to_string(m_Size.x) + ", " + std::to_string(m_Size.y) + ")");
            Core::Log("[UIElement] - Cor: 0x" + std::to_string(GetRenderColor()));
            Core::Log("[UIElement] - Chamando batch.AddRect(" + std::to_string(absPos.x) + ", " + std::to_string(absPos.y) + 
                     ", " + std::to_string(m_Size.x) + ", " + std::to_string(m_Size.y) + ", 0x" + std::to_string(GetRenderColor()) + ")");
        }
        
        batch.AddRect(absPos.x, absPos.y, m_Size.x, m_Size.y, GetRenderColor());
    }

    // Desenha filhos
    for (auto& child : m_Children)
        child->Render(batch);
} 