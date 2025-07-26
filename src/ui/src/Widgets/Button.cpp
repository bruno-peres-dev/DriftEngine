#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/DataDriven/UIComponentRegistry.h"
#include "Drift/UI/Widgets/Label.h"
#include "Drift/UI/Widgets/Panel.h"
#include "Drift/UI/Widgets/Image.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

// Registra o widget Button no sistema de componentes
REGISTER_UI_WIDGET("button", Button)

Button::Button(UIContext* context)
    : UIElement(context)
{
    // Tamanho padrão do botão
    SetSize(glm::vec2(120.0f, 40.0f));
}

void Button::Update(float deltaSeconds)
{
    // Chama o update da classe base
    UIElement::Update(deltaSeconds);
    
    // Atualiza o estado visual se necessário
    UpdateState();
}

void Button::Render(Drift::RHI::IUIBatcher& batch)
{
    // Chama o render da classe base (UIElement)
    UIElement::Render(batch);
    
    // Renderiza o texto do botão apenas se não estiver vazio
    if (!m_Text.empty()) {
        glm::vec2 absPos = GetAbsolutePosition();
        glm::vec2 size = GetSize();
        
        // Centraliza o texto no botão
        glm::vec2 textPos = absPos + size * 0.5f;
        
        // Renderiza o texto com a cor atual do botão
        Drift::Color textColor = GetCurrentColor();
        
        // Verificação adicional de segurança
        if (m_Text.c_str() != nullptr) {
            batch.AddText(textPos.x, textPos.y, m_Text.c_str(), textColor);
        }
    }
}

void Button::OnMouseEnter()
{
    if (!m_Enabled) return;
    
    m_IsHovered = true;
    UpdateState();
    
    if (m_OnHover) {
        ButtonHoverEvent event{this, true};
        m_OnHover(event);
    }
}

void Button::OnMouseLeave()
{
    if (!m_Enabled) return;
    
    m_IsHovered = false;
    m_IsPressed = false;
    UpdateState();
    
    if (m_OnHover) {
        ButtonHoverEvent event{this, false};
        m_OnHover(event);
    }
}

void Button::OnMouseDown(const glm::vec2& position)
{
    if (!m_Enabled) return;
    
    m_IsPressed = true;
    UpdateState();
}

void Button::OnMouseUp(const glm::vec2& position)
{
    if (!m_Enabled) return;
    
    m_IsPressed = false;
    UpdateState();
}

void Button::OnMouseClick(const glm::vec2& position)
{
    if (!m_Enabled) return;
    
    // Dispara o evento de click
    if (m_OnClick) {
        ButtonClickEvent event{this, position};
        m_OnClick(event);
    }
}

void Button::UpdateState()
{
    ButtonState newState = ButtonState::Normal;
    
    if (!m_Enabled) {
        newState = ButtonState::Disabled;
    } else if (m_IsPressed) {
        newState = ButtonState::Pressed;
    } else if (m_IsHovered) {
        newState = ButtonState::Hover;
    }
    
    if (newState != m_CurrentState) {
        m_CurrentState = newState;
        MarkDirty();
    }
}

Drift::Color Button::GetCurrentColor() const
{
    Drift::Color color;
    switch (m_CurrentState) {
        case ButtonState::Normal:
            color = m_NormalColor;
            break;
        case ButtonState::Hover:
            color = m_HoverColor;
            break;
        case ButtonState::Pressed:
            color = m_PressedColor;
            break;
        case ButtonState::Disabled:
            color = m_DisabledColor;
            break;
        default:
            color = m_NormalColor;
            break;
    }
    
    return color;
} 

Drift::Color Button::GetRenderColor() const
{
    return GetCurrentColor();
} 

// Registra os widgets no registry (apenas uma vez)
REGISTER_UI_WIDGET("label", Label);
REGISTER_UI_WIDGET("panel", Panel);
REGISTER_UI_WIDGET("image", Image);