#include "Drift/UI/Widgets/Label.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/FontSystem/FontManager.h"
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
        m_TextSize = CalculateTextSize();
        if (m_TextSize.x > 0 && m_TextSize.y > 0) {
            SetSize(m_TextSize);
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

    // Renderiza o texto real
    if (!m_Text.empty()) {
        glm::vec2 absPos = GetAbsolutePosition();
        
        // Calcula posição baseado no alinhamento
        float xOffset = 0.0f;
        switch (m_TextAlign) {
            case TextAlign::Center:
                xOffset = (m_Size.x - m_TextSize.x) * 0.5f;
                break;
            case TextAlign::Right:
                xOffset = m_Size.x - m_TextSize.x;
                break;
            case TextAlign::Left:
            default:
                xOffset = 0.0f;
                break;
        }
        
        // Renderiza o texto real usando o sistema de fontes
        batch.AddText(absPos.x + xOffset, absPos.y, m_Text.c_str(), m_TextColor);
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

    // Usa o sistema de fontes para calcular o tamanho real
    auto& fontManager = Drift::UI::FontManager::GetInstance();
    auto font = fontManager.GetFont(m_FontFamily, m_FontSize);
    
    if (font) {
        return font->MeasureText(m_Text);
    } else {
        // Fallback para cálculo aproximado se a fonte não estiver disponível
        float textWidth = m_Text.length() * m_FontSize * 0.6f; // Aproximação
        float textHeight = m_FontSize * 1.2f; // Inclui line height
        return glm::vec2(textWidth, textHeight);
    }
} 