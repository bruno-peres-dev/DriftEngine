#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <glm/mat4x4.hpp>
#include <chrono>

using namespace Drift::UI;

TextRenderer::TextRenderer() = default;

TextRenderer::~TextRenderer() = default;

// Implementação da função de decodificação UTF-8
std::vector<uint32_t> TextRenderer::DecodeUTF8(const std::string& utf8_string) {
    std::vector<uint32_t> codepoints;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(utf8_string.data());
    size_t len = utf8_string.length();
    
    for (size_t i = 0; i < len; ) {
        uint32_t codepoint = 0;
        
        if (bytes[i] <= 0x7F) {
            // ASCII (1 byte)
            codepoint = bytes[i];
            i += 1;
        } else if ((bytes[i] & 0xE0) == 0xC0) {
            // 2 bytes
            if (i + 1 < len && (bytes[i + 1] & 0xC0) == 0x80) {
                codepoint = ((bytes[i] & 0x1F) << 6) | (bytes[i + 1] & 0x3F);
                i += 2;
            } else {
                // Sequência inválida, usar caractere de substituição
                codepoint = 0xFFFD;
                i += 1;
            }
        } else if ((bytes[i] & 0xF0) == 0xE0) {
            // 3 bytes
            if (i + 2 < len && (bytes[i + 1] & 0xC0) == 0x80 && (bytes[i + 2] & 0xC0) == 0x80) {
                codepoint = ((bytes[i] & 0x0F) << 12) | ((bytes[i + 1] & 0x3F) << 6) | (bytes[i + 2] & 0x3F);
                i += 3;
            } else {
                // Sequência inválida, usar caractere de substituição
                codepoint = 0xFFFD;
                i += 1;
            }
        } else if ((bytes[i] & 0xF8) == 0xF0) {
            // 4 bytes
            if (i + 3 < len && (bytes[i + 1] & 0xC0) == 0x80 && (bytes[i + 2] & 0xC0) == 0x80 && (bytes[i + 3] & 0xC0) == 0x80) {
                codepoint = ((bytes[i] & 0x07) << 18) | ((bytes[i + 1] & 0x3F) << 12) | ((bytes[i + 2] & 0x3F) << 6) | (bytes[i + 3] & 0x3F);
                i += 4;
            } else {
                // Sequência inválida, usar caractere de substituição
                codepoint = 0xFFFD;
                i += 1;
            }
        } else {
            // Byte inicial inválido, usar caractere de substituição
            codepoint = 0xFFFD;
            i += 1;
        }
        
        codepoints.push_back(codepoint);
    }
    
    return codepoints;
}

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
    if (!font) {
        // Tentar carregar a fonte se não encontrada
        font = fm.GetOrLoadFont(fontName, fm.GetDefaultFontPath(), fontSize, UI::FontQuality::High);
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
    
    // Marcar início de renderização de texto
    m_Batcher->BeginText();
    m_Batcher->SetTexture(0, const_cast<Drift::RHI::ITexture*>(texture));

    float baseline = pos.y + font->GetAscent();
    float x = pos.x;
    
    // Decodificar UTF-8 para codepoints antes de renderizar
    std::vector<uint32_t> codepoints = DecodeUTF8(text);
    for (uint32_t codepoint : codepoints) {
        x += RenderGlyph(codepoint, font, x, baseline, color);
    }
    
    // Marcar fim de renderização de texto
    m_Batcher->EndText();
}

float TextRenderer::RenderGlyph(uint32_t codepoint, const std::shared_ptr<Font>& font, float x, float baseline, const glm::vec4& color) {
    const GlyphInfo* g = font->GetGlyph(codepoint);
    if (!g) {
        // Log apenas para debug quando necessário
        if (codepoint >= 33 && codepoint <= 126) {  // ASCII imprimível
            Drift::Core::LogWarning("[TextRenderer] Glyph não encontrado para codepoint ASCII: " + std::to_string(codepoint));
        }
        return 0.0f;  // Retorna 0 se glyph não encontrado
    }

    // Para espaços, apenas avançar a posição sem renderizar
    if (codepoint == 32) {
        return g->advance;
    }
    
    // Para outros caracteres invisíveis, apenas avançar a posição
    if (g->size.x <= 0.0f || g->size.y <= 0.0f) {
        Drift::Core::LogWarning("[TextRenderer] Glyph com tamanho inválido para codepoint: " + std::to_string(codepoint));
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
    
    // Decodificar UTF-8 para codepoints antes de medir
    std::vector<uint32_t> codepoints = DecodeUTF8(text);
    for (uint32_t codepoint : codepoints) {
        const GlyphInfo* g = font->GetGlyph(codepoint);
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

