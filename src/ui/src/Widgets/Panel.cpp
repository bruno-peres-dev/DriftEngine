#include "Drift/UI/Widgets/Panel.h"
#include "Drift/UI/UIContext.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

Panel::Panel(UIContext* context)
    : UIElement(context)
{
    // Tamanho padrão do painel
    SetSize(glm::vec2(200.0f, 150.0f));
    SetColor(COLOR_PANEL_BG);
}

void Panel::Update(float deltaSeconds)
{
    UIElement::Update(deltaSeconds);
}

void Panel::Render(Drift::RHI::IUIBatcher& batch)
{
    glm::vec2 absPos = GetAbsolutePosition();
    
    // Renderiza o fundo do painel
    if (m_Color != 0x00000000) {
        batch.AddRect(absPos.x, absPos.y, m_Size.x, m_Size.y, m_Color);
    }

    // Renderiza a borda se habilitada
    if (m_ShowBorder && m_BorderWidth > 0) {
        // Borda superior
        batch.AddRect(absPos.x, absPos.y, m_Size.x, m_BorderWidth, m_BorderColor);
        // Borda inferior
        batch.AddRect(absPos.x, absPos.y + m_Size.y - m_BorderWidth, m_Size.x, m_BorderWidth, m_BorderColor);
        // Borda esquerda
        batch.AddRect(absPos.x, absPos.y, m_BorderWidth, m_Size.y, m_BorderColor);
        // Borda direita
        batch.AddRect(absPos.x + m_Size.x - m_BorderWidth, absPos.y, m_BorderWidth, m_Size.y, m_BorderColor);
    }

    // Renderiza o título se existir
    if (!m_Title.empty()) {
        // TODO: Implementar renderização de texto real
        // Por enquanto, renderiza um retângulo representando o título
        float titleHeight = 20.0f;
        batch.AddRect(absPos.x + m_Padding, absPos.y + m_Padding, 
                     m_Size.x - 2 * m_Padding, titleHeight, COLOR_TITLE);
    }

    // Renderiza filhos (com offset de padding)
    for (auto& child : m_Children) {
        child->Render(batch);
    }
}

glm::vec2 Panel::GetContentArea() const
{
    float contentWidth = m_Size.x - 2 * m_Padding;
    float contentHeight = m_Size.y - 2 * m_Padding;
    
    // Ajusta para o título se existir
    if (!m_Title.empty()) {
        contentHeight -= 25.0f; // Altura do título + padding
    }
    
    return glm::vec2(contentWidth, contentHeight);
} 