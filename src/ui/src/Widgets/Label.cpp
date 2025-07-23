#include "Drift/UI/Widgets/Label.h"
#include "Drift/UI/UIContext.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

Label::Label(UIContext* context)
    : UIElement(context)
{
    // Tamanho padrão baseado no texto
    SetSize(glm::vec2(100.0f, 20.0f));
}

void Label::Update(float deltaSeconds)
{
    // Atualiza o tamanho baseado no texto se necessário
    if (m_Dirty) {
        glm::vec2 textSize = CalculateTextSize();
        if (textSize.x > 0 && textSize.y > 0) {
            SetSize(textSize);
        }
    }
    
    UIElement::Update(deltaSeconds);
}

void Label::Render(Drift::RHI::IUIBatcher& batch)
{
    // Renderiza o fundo se tiver cor
    if (m_Color != 0x00000000) {
        UIElement::Render(batch);
    }

    // Renderiza o texto (placeholder por enquanto)
    if (!m_Text.empty()) {
        glm::vec2 absPos = GetAbsolutePosition();
        
        // TODO: Implementar renderização de texto real
        // Por enquanto, renderiza um retângulo representando o texto
        float textWidth = m_Text.length() * m_FontSize * 0.6f; // Aproximação
        float textHeight = m_FontSize;
        
        // Ajusta posição baseado no alinhamento
        float xOffset = 0.0f;
        switch (m_TextAlign) {
            case TextAlign::Center:
                xOffset = (m_Size.x - textWidth) * 0.5f;
                break;
            case TextAlign::Right:
                xOffset = m_Size.x - textWidth;
                break;
            case TextAlign::Left:
            default:
                xOffset = 0.0f;
                break;
        }
        
        // Renderiza retângulo representando o texto (placeholder)
        batch.AddRect(absPos.x + xOffset, absPos.y, textWidth, textHeight, m_TextColor);
    }

    // Renderiza filhos
    for (auto& child : m_Children) {
        child->Render(batch);
    }
}

glm::vec2 Label::CalculateTextSize() const
{
    if (m_Text.empty()) {
        return glm::vec2(0.0f, 0.0f);
    }

    // Cálculo aproximado do tamanho do texto
    // TODO: Implementar cálculo real baseado na fonte
    float textWidth = m_Text.length() * m_FontSize * 0.6f; // Aproximação
    float textHeight = m_FontSize * 1.2f; // Inclui line height
    
    return glm::vec2(textWidth, textHeight);
} 