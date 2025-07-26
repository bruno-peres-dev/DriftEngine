#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <glm/mat4x4.hpp>

using namespace Drift::UI;

TextRenderer::TextRenderer() = default;

void TextRenderer::AddText(const std::string& text, const glm::vec2& pos,
                           const std::string& fontName, float fontSize, const glm::vec4& color) {
    if (!m_Batcher) {
        Drift::Core::LogError("[TextRenderer] Batcher não configurado");
        return;
    }
    
    Drift::Core::LogRHIDebug("[TextRenderer] Renderizando texto: '" + text + "' em pos (" + 
                            std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");

    auto& fm = FontManager::GetInstance();
    
    auto font = fm.GetFont(fontName, fontSize);
    if (!font) {
        Drift::Core::LogWarning("[TextRenderer] Fonte não encontrada: " + fontName + " (tamanho: " + std::to_string(fontSize) + ")");
        
        // Tentar carregar a fonte se não encontrada
        Drift::Core::LogRHIDebug("[TextRenderer] Tentando lazy loading da fonte...");
        font = fm.GetOrLoadFont(fontName, "fonts/Arial-Regular.ttf", fontSize);
        if (!font) {
            Drift::Core::LogError("[TextRenderer] Falha no lazy loading da fonte");
            return;
        }
    }

    const auto* texture = font->GetAtlasTexture().get();
    if (!texture) {
        Drift::Core::LogError("[TextRenderer] Textura do atlas não encontrada");
        return;
    }
    
    m_Batcher->SetTexture(0, const_cast<Drift::RHI::ITexture*>(texture));

    float baseline = pos.y + font->GetAscent();
    float x = pos.x;
    for (char c : text) {
        const GlyphInfo* g = font->GetGlyph(static_cast<unsigned char>(c));
        if (!g) {
            continue;
        }

        float xpos = x + g->bearing.x;
        float ypos = baseline + g->bearing.y;

        glm::vec2 uv0 = g->uv0;
        glm::vec2 uv1 = g->uv1;
        float w = g->size.x;
        float h = g->size.y;

        // Debug: verificar valores
        Drift::Core::LogRHIDebug("[TextRenderer] Glyph '" + std::string(1, c) + 
                                "' pos: (" + std::to_string(xpos) + ", " + std::to_string(ypos) + ")" +
                                " size: (" + std::to_string(w) + ", " + std::to_string(h) + ")" +
                                " uv: (" + std::to_string(uv0.x) + ", " + std::to_string(uv0.y) + ") -> (" + 
                                std::to_string(uv1.x) + ", " + std::to_string(uv1.y) + ")");

        Drift::Color textColor = Drift::Color((uint8_t)(color.w * 255.0f) << 24 |
                                              (uint8_t)(color.x * 255.0f) << 16 |
                                              (uint8_t)(color.y * 255.0f) << 8 |
                                              (uint8_t)(color.z * 255.0f));
        
        m_Batcher->AddTexturedRect(xpos, ypos, w, h, uv0, uv1, textColor, 0);
        x += g->advance;
    }
}

glm::vec2 TextRenderer::MeasureText(const std::string& text, const std::string& fontName, float size) {
    auto& fm = FontManager::GetInstance();
    auto font = fm.GetFont(fontName, size);
    if (!font) return glm::vec2(0.0f);

    float width = 0.0f;
    float height = font->GetAscent() - font->GetDescent();
    for (char c : text) {
        const GlyphInfo* g = font->GetGlyph(static_cast<unsigned char>(c));
        if (!g) continue;
        width += g->advance;
    }
    return glm::vec2(width, height);
}

