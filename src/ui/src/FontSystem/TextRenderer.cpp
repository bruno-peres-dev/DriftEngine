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
    
    // Log apenas em nível Trace para evitar spam
    Drift::Core::LogTrace("[TextRenderer] Renderizando texto: '" + text + "' em pos (" + 
                            std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");

    auto& fm = FontManager::GetInstance();
    
    auto font = fm.GetFont(fontName, fontSize);
    if (!font) {
        Drift::Core::LogWarning("[TextRenderer] Fonte não encontrada: " + fontName + " (tamanho: " + std::to_string(fontSize) + ")");
        
        // Tentar carregar a fonte se não encontrada
        Drift::Core::LogDebug("[TextRenderer] Tentando lazy loading da fonte...");
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

        // Debug: verificar valores (apenas em nível Trace para evitar spam)
        Drift::Core::LogTrace("[TextRenderer] Glyph '" + std::string(1, c) + 
                                "' pos: (" + std::to_string(xpos) + ", " + std::to_string(ypos) + ")" +
                                " size: (" + std::to_string(w) + ", " + std::to_string(h) + ")" +
                                " uv: (" + std::to_string(uv0.x) + ", " + std::to_string(uv0.y) + ") -> (" + 
                                std::to_string(uv1.x) + ", " + std::to_string(uv1.y) + ")");

        // Corrigir conversão de cores: glm::vec4 (RGBA) para Drift::Color (ARGB)
        // glm::vec4: (r, g, b, a) onde cada componente é 0.0-1.0
        // Drift::Color: ARGB onde cada componente é 0-255
        Drift::Color textColor = Drift::Color(
            (uint8_t)(color.a * 255.0f) << 24 |  // Alpha no byte mais significativo
            (uint8_t)(color.r * 255.0f) << 16 |  // Red
            (uint8_t)(color.g * 255.0f) << 8 |   // Green
            (uint8_t)(color.b * 255.0f)          // Blue no byte menos significativo
        );
        
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

