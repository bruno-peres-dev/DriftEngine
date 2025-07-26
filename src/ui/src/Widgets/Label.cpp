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
        LOG_INFO("Label::Render: renderizando fundo com cor 0x" + std::to_string(m_Color));
        UIElement::Render(batch);
    } else {
        LOG_INFO("Label::Render: fundo transparente, pulando renderização");
    }

    // Renderiza o texto real apenas se necessário
    if (!m_Text.empty()) {
        glm::vec2 absPos = GetAbsolutePosition();
        
        LOG_DEBUG("Label::Render: renderizando texto '" + m_Text + "' em (" + 
                 std::to_string(absPos.x) + ", " + std::to_string(absPos.y) + ")");
        
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
        
        glm::vec2 textPos = glm::vec2(absPos.x + xOffset, absPos.y);
        
        LOG_DEBUG("Label::Render: posição final do texto: (" + 
                 std::to_string(textPos.x) + ", " + std::to_string(textPos.y) + ")");
        
        // FORÇAR RENDERIZAÇÃO SEMPRE PARA DEBUG
        m_LastTextPos = textPos;
        m_LastTextColor = m_TextColor;
        
        LOG_DEBUG("Label::Render: chamando batch.AddText com cor 0x" + 
                 std::to_string(m_TextColor));
        
        // Renderiza o texto real usando o sistema de fontes
        Core::Log("[Label] Adicionando texto '" + m_Text + "' em (" + std::to_string(textPos.x) + ", " + std::to_string(textPos.y) + ")");
        batch.AddText(textPos.x, textPos.y, m_Text.c_str(), m_TextColor);
        Core::Log("[Label] Texto adicionado com sucesso");
    } else {
        LOG_DEBUG("Label::Render: texto vazio, pulando renderização");
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