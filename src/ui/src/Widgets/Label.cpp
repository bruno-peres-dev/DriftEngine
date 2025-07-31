#include "Drift/UI/Widgets/Label.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/FontRendering.h"
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
        // Renderiza apenas o fundo sem chamar UIElement::Render para evitar renderização dupla
        if (m_Visible && m_Opacity > 0.0f && m_Size.x > 0 && m_Size.y > 0) {
            Drift::Color color = GetRenderColor();
            unsigned alpha = static_cast<unsigned>(((color >> 24) & 0xFF) * m_Opacity);
            color = (color & 0x00FFFFFF) | (alpha << 24);
            
            if (alpha > 0) {
                batch.AddQuad(m_WorldTransform, m_Size.x, m_Size.y, color);
            }
        }
    }

    // Renderiza o texto real apenas se necessário
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
        
        glm::vec2 textPos = glm::vec2(absPos.x + xOffset, absPos.y);
        
        // FORÇAR RENDERIZAÇÃO SEMPRE PARA DEBUG
        m_LastTextPos = textPos;
        m_LastTextColor = m_TextColor;
        
        // Renderiza o texto real usando o novo sistema de fontes
        auto* textRenderer = m_Context ? m_Context->GetTextRenderer() : nullptr;
        if (textRenderer) {
            textRenderer->RenderText(m_Text, textPos, m_FontFamily, m_FontSize, TextColorToVec4(m_TextColor));
        } else {
            batch.AddText(textPos.x, textPos.y, m_Text.c_str(), m_TextColor); // fallback
        }
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
    // Usa o novo sistema de fontes para calcular o tamanho real
    auto* textRenderer = m_Context ? m_Context->GetTextRenderer() : nullptr;
    if (textRenderer) {
        return textRenderer->MeasureText(m_Text, m_FontFamily, m_FontSize);
    } else {
        // Fallback para cálculo aproximado se o renderizador não estiver disponível
        float textWidth = m_Text.length() * m_FontSize * 0.6f;
        float textHeight = m_FontSize * 1.2f;
        return glm::vec2(textWidth, textHeight);
    }
}

// Função utilitária para converter Drift::Color para glm::vec4
glm::vec4 Label::TextColorToVec4(Drift::Color color) const {
    // Drift::Color: ARGB onde cada componente é 0-255
    // glm::vec4: RGBA onde cada componente é 0.0-1.0
    float a = static_cast<float>((color >> 24) & 0xFF) / 255.0f;  // Alpha
    float r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;  // Red
    float g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;   // Green
    float b = static_cast<float>(color & 0xFF) / 255.0f;          // Blue
    return glm::vec4(r, g, b, a);
} 