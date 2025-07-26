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

    auto& fm = FontManager::GetInstance();
    auto font = fm.GetFont(fontName, fontSize);
    if (!font) {
        Drift::Core::LogWarning("[TextRenderer] Fonte não encontrada: " + fontName);
        return;
    }

    const auto* texture = font->GetAtlasTexture().get();
    if (!texture) return;

    float x = pos.x;
    for (char c : text) {
        const GlyphInfo* g = font->GetGlyph(static_cast<unsigned char>(c));
        if (!g) continue;

        float xpos = x + g->bearing.x;
        float ypos = pos.y - g->bearing.y;

        glm::vec2 uv0 = g->uv0;
        glm::vec2 uv1 = g->uv1;
        float w = g->size.x;
        float h = g->size.y;

        m_Batcher->AddTexturedRect(xpos, ypos, w, h, uv0, uv1,
                                   Drift::Color((uint8_t)(color.w * 255.0f) << 24 |
                                                (uint8_t)(color.x * 255.0f) << 16 |
                                                (uint8_t)(color.y * 255.0f) << 8 |
                                                (uint8_t)(color.z * 255.0f)), 1);
        x += g->advance;
    }
}

glm::vec2 TextRenderer::MeasureText(const std::string& text, const std::string& fontName, float size) {
    auto& fm = FontManager::GetInstance();
    auto font = fm.GetFont(fontName, size);
    if (!font) return glm::vec2(0.0f);

    float width = 0.0f;
    float height = 0.0f;
    for (char c : text) {
        const GlyphInfo* g = font->GetGlyph(static_cast<unsigned char>(c));
        if (!g) continue;
        width += g->advance;
        height = std::max(height, g->size.y);
    }
    return glm::vec2(width, height);
}

