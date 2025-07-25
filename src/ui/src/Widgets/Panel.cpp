#include "Drift/UI/Widgets/Panel.h"
#include "Drift/RHI/Buffer.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Drift::UI;

Panel::Panel(UIContext* context)
    : UIElement(context)
{
    SetName("Panel");
    // Configuração padrão do panel
    SetColor(m_BackgroundColor);
}

void Panel::Render(Drift::RHI::IUIBatcher& batch)
{
    if (!IsVisible() || GetOpacity() <= 0.0f)
        return;

    // Aplica clipping se habilitado (igual ao UIElement base)
    bool clippingApplied = false;
    if (GetLayoutProperties().clipContent) {
        glm::vec2 absPos = GetAbsolutePosition();
        batch.PushScissorRect(absPos.x, absPos.y, GetSize().x, GetSize().y);
        clippingApplied = true;
    }

    // Só renderiza se tiver tamanho > 0
    if (GetSize().x > 0 && GetSize().y > 0) {
        unsigned color = GetRenderColor();
        
        // Aplica opacidade
        unsigned alpha = static_cast<unsigned>(((color >> 24) & 0xFF) * GetOpacity());
        color = (color & 0x00FFFFFF) | (alpha << 24);
        
        // Só renderiza se o alpha final for > 0
        if (alpha > 0) {
            batch.AddQuad(GetWorldTransform(), GetSize().x, GetSize().y, color);
        }
        
        // Renderiza borda se necessário
        if (m_BorderWidth > 0.0f && m_BorderColor != 0) {
            unsigned borderAlpha = static_cast<unsigned>(((m_BorderColor >> 24) & 0xFF) * GetOpacity());
            unsigned borderColor = (m_BorderColor & 0x00FFFFFF) | (borderAlpha << 24);
            
            // Só renderiza bordas se o alpha for > 0
            if (borderAlpha > 0) {
                // Bordas simples (pode ser melhorado com linhas)
                float w = GetSize().x;
                float h = GetSize().y;
                
                // Verifica se as dimensões são válidas antes de renderizar bordas
                if (w > 0 && h > 0) {
                    // Calcula espessura da borda
                    float borderThickness = m_BorderWidth;
                    
                    // Se bordas proporcionais estão habilitadas, calcula baseado no tamanho
                    if (m_ProportionalBorders) {
                        float minDimension = std::min(w, h);
                        float proportionalBorder = std::max(1.0f, minDimension * m_BorderProportion);
                        borderThickness = std::min(borderThickness, proportionalBorder);
                    }
                    
                    // Garante um mínimo de 1 pixel
                    borderThickness = std::max(1.0f, borderThickness);
                    
                    glm::mat4 base = GetWorldTransform();
                    using glm::translate;
                    glm::mat4 m;
                    // Borda superior
                    m = base * translate(glm::mat4(1.0f), {0.0f, 0.0f, 0.0f});
                    batch.AddQuad(m, w, borderThickness, borderColor);
                    // Borda inferior
                    m = base * translate(glm::mat4(1.0f), {0.0f, h - borderThickness, 0.0f});
                    batch.AddQuad(m, w, borderThickness, borderColor);
                    // Borda esquerda
                    m = base * translate(glm::mat4(1.0f), {0.0f, 0.0f, 0.0f});
                    batch.AddQuad(m, borderThickness, h, borderColor);
                    // Borda direita
                    m = base * translate(glm::mat4(1.0f), {w - borderThickness, 0.0f, 0.0f});
                    batch.AddQuad(m, borderThickness, h, borderColor);
                }
            }
        }
    }

    // Renderiza filhos (com clipping aplicado)
    for (auto& child : GetChildren()) {
        child->Render(batch);
    }
    
    // Remove clipping se foi aplicado
    if (clippingApplied) {
        batch.PopScissorRect();
    }
} 