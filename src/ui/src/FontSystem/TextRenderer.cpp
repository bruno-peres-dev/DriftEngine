#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <glm/mat4x4.hpp>
#include <chrono>

using namespace Drift::UI;

TextRenderer::TextRenderer() = default;

TextRenderer::~TextRenderer() = default;

void TextRenderer::BeginTextRendering() {
    // Preparação para renderização de texto
    // Pode ser usado para otimizações futuras
}

void TextRenderer::EndTextRendering() {
    // Finalização da renderização de texto
    // Pode ser usado para otimizações futuras
}

void TextRenderer::AddText(const std::string& text, const glm::vec2& pos,
                           const std::string& fontName, float fontSize, const glm::vec4& color) {
    if (!m_Batcher) {
        Drift::Core::LogError("[TextRenderer] Batcher não configurado");
        return;
    }

    auto& fm = FontManager::GetInstance();
    
    auto font = fm.GetFont(fontName, fontSize, UI::FontQuality::High);
    if (font) {
        Drift::Core::Log("[TextRenderer] Fonte encontrada: " + fontName + " (tamanho: " + std::to_string(fontSize) + ")");
    } else {
        Drift::Core::Log("[TextRenderer] Fonte NÃO encontrada: " + fontName + " (tamanho: " + std::to_string(fontSize) + ")");
    }
    if (!font) {
        // Tentar carregar a fonte se não encontrada
        font = fm.GetOrLoadFont(fontName, "fonts/Arial-Regular.ttf", fontSize, UI::FontQuality::High);
        if (!font) {
            Drift::Core::LogError("[TextRenderer] Falha no carregamento da fonte");
            return;
        }
    }

    const auto* texture = font->GetAtlasTexture().get();
    if (!texture) {
        Drift::Core::LogError("[TextRenderer] Textura do atlas não encontrada");
        return;
    }
    
    Drift::Core::Log("[TextRenderer] Configurando textura do atlas no batcher...");
    
    // Marcar início de renderização de texto
    m_Batcher->BeginText();
    m_Batcher->SetTexture(0, const_cast<Drift::RHI::ITexture*>(texture));

    float baseline = pos.y + font->GetAscent();
    float x = pos.x;
    
    for (char c : text) {
        x += RenderGlyph(c, font, x, baseline, color);
    }
    
    // Marcar fim de renderização de texto
    m_Batcher->EndText();
}

float TextRenderer::RenderGlyph(char c, const std::shared_ptr<Font>& font, float x, float baseline, const glm::vec4& color) {
    const GlyphInfo* g = font->GetGlyph(static_cast<unsigned char>(c));
    if (!g) {
        Drift::Core::Log("[TextRenderer] Glyph não encontrado para caractere: '" + std::string(1, c) + "' (codepoint: " + std::to_string(static_cast<unsigned char>(c)) + ")");
        return 0.0f;  // Retorna 0 se glyph não encontrado
    }

    Drift::Core::Log("[TextRenderer] Renderizando glyph '" + std::string(1, c) + "' em (" + std::to_string(x) + ", " + std::to_string(baseline) + ") - UV: (" + std::to_string(g->uv0.x) + "," + std::to_string(g->uv0.y) + ") a (" + std::to_string(g->uv1.x) + "," + std::to_string(g->uv1.y) + ")");

    // Para espaços, apenas avançar a posição sem renderizar
    if (c == ' ') {
        return g->advance;
    }
    
    // Para outros caracteres invisíveis, apenas avançar a posição
    if (g->size.x <= 0.0f || g->size.y <= 0.0f) {
        return g->advance;
    }

    float xpos = x + g->bearing.x;
    float ypos = baseline + g->bearing.y;

    glm::vec2 uv0 = g->uv0;
    glm::vec2 uv1 = g->uv1;
    float w = g->size.x;
    float h = g->size.y;

    // Converter cor: glm::vec4 (RGBA) para Drift::Color (ARGB)
    Drift::Color textColor = Drift::Color(
        (uint8_t)(color.a * 255.0f) << 24 |  // Alpha
        (uint8_t)(color.r * 255.0f) << 16 |  // Red
        (uint8_t)(color.g * 255.0f) << 8 |   // Green
        (uint8_t)(color.b * 255.0f)          // Blue
    );
    
    m_Batcher->AddTexturedRect(xpos, ypos, w, h, uv0, uv1, textColor, 0);
    return g->advance;
}

glm::vec2 TextRenderer::MeasureText(const std::string& text, const std::string& fontName, float size) {
    if (m_Config.enableTextCache) {
        return GetCachedTextMeasure(text, fontName, size);
    }
    
    auto& fm = FontManager::GetInstance();
    auto font = fm.GetFont(fontName, size, UI::FontQuality::High);
    if (!font) {
        return glm::vec2(0.0f);
    }
    
    return CalculateTextMeasure(text, font);
}

glm::vec2 TextRenderer::GetCachedTextMeasure(const std::string& text, const std::string& fontName, float size) {
    TextCacheKey key{text, fontName, size};
    auto it = m_TextCache.find(key);
    
    if (it != m_TextCache.end()) {
        it->second.lastUsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
        return it->second.size;
    }
    
    // Calcular medida e armazenar no cache
    auto& fm = FontManager::GetInstance();
    auto font = fm.GetFont(fontName, size, UI::FontQuality::High);
    if (!font) {
        return glm::vec2(0.0f);
    }
    
    glm::vec2 measure = CalculateTextMeasure(text, font);
    
    // Verificar se cache está cheio
    if (m_TextCache.size() >= m_Config.maxCachedStrings) {
        TrimTextCache();
    }
    
    TextMeasureCache cacheEntry;
    cacheEntry.size = measure;
    cacheEntry.lastUsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    m_TextCache[key] = cacheEntry;
    return measure;
}

glm::vec2 TextRenderer::CalculateTextMeasure(const std::string& text, const std::shared_ptr<Font>& font) {
    float width = 0.0f;
    float height = font->GetAscent() - font->GetDescent();
    
    for (char c : text) {
        const GlyphInfo* g = font->GetGlyph(static_cast<unsigned char>(c));
        if (!g) continue;
        width += g->advance;
    }
    
    return glm::vec2(width, height);
}

void TextRenderer::ClearTextCache() {
    m_TextCache.clear();
}

size_t TextRenderer::GetTextCacheSize() const {
    return m_TextCache.size();
}

void TextRenderer::TrimTextCache() {
    if (m_TextCache.size() <= m_Config.maxCachedStrings / 2) {
        return;  // Não precisa fazer trim se está bem abaixo do limite
    }
    
    std::vector<std::pair<TextCacheKey, TextMeasureCache>> entries;
    entries.reserve(m_TextCache.size());
    
    for (auto& [key, cache] : m_TextCache) {
        entries.emplace_back(key, cache);
    }
    
    // Ordenar por último uso (LRU)
    std::sort(entries.begin(), entries.end(), 
        [](const auto& a, const auto& b) {
            return a.second.lastUsed < b.second.lastUsed;
        });
    
    // Remover metade das entradas mais antigas
    size_t toRemove = m_TextCache.size() / 2;
    for (size_t i = 0; i < toRemove; ++i) {
        m_TextCache.erase(entries[i].first);
    }
}

