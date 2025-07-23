#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/DataDriven/UIComponentRegistry.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

// Registra o widget Button no sistema de componentes
REGISTER_UI_WIDGET("button", Button)

Button::Button(UIContext* context)
    : UIElement(context)
{
    // Tamanho padrão do botão
    SetSize(glm::vec2(120.0f, 40.0f));
    
    // Debug: Log cores padrão
    Core::Log("[Button] Construtor - Cores padrão:");
    Core::Log("[Button] - Normal: 0x" + std::to_string(m_NormalColor));
    Core::Log("[Button] - Hover: 0x" + std::to_string(m_HoverColor));
    Core::Log("[Button] - Pressed: 0x" + std::to_string(m_PressedColor));
    Core::Log("[Button] - Disabled: 0x" + std::to_string(m_DisabledColor));
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
    
    // O Button não precisa renderizar nada adicional por enquanto
    // pois a classe base já renderiza o retângulo com a cor correta
    // baseada no estado do botão (normal, hover, pressed, disabled)
    
    // TODO: Renderizar texto quando o sistema de texto estiver implementado
    // if (!m_Text.empty()) {
    //     glm::vec2 absPos = GetAbsolutePosition();
    //     glm::vec2 textPos = absPos + GetSize() * 0.5f;
    //     batch.AddText(textPos.x, textPos.y, m_Text.c_str(), 0xFFFFFFFF);
    // }
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
    
    bool wasPressed = m_IsPressed;
    m_IsPressed = false;
    UpdateState();
    
    // Se estava pressionado e soltou dentro do botão, dispara o evento de click
    if (wasPressed && m_IsHovered && m_OnClick) {
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

unsigned Button::GetCurrentColor() const
{
    unsigned color;
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
    
    // Debug: Log a cor sendo retornada
    static int colorLogCount = 0;
    colorLogCount++;
    if (colorLogCount % 60 == 0) { // Log a cada segundo
        Core::Log("[Button] GetCurrentColor - Estado: " + std::to_string((int)m_CurrentState) + 
                 " -> Cor: 0x" + std::to_string(color));
    }
    
    return color;
} 

unsigned Button::GetRenderColor() const
{
    return GetCurrentColor();
} 