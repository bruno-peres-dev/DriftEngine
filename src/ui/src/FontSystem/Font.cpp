#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <algorithm>

namespace Drift::UI {

Font::Font(const std::string& name, const std::string& filePath, float size, FontQuality quality)
    : m_Name(name)
    , m_FilePath(filePath)
    , m_Size(size)
    , m_Quality(quality)
    , m_IsLoaded(false) {
}

Font::~Font() {
    Unload();
}

bool Font::Load() {
    if (m_IsLoaded) {
        return true;
    }
    
    // Aqui seria implementada a lógica real de carregamento de fonte
    // Por enquanto, vamos simular o carregamento
    
    LOG_INFO("Loading font: {} from {}", m_Name, m_FilePath);
    
    // Simular carregamento de glyphs básicos
    LoadBasicGlyphs();
    
    // Criar atlas de fontes
    m_Atlas = std::make_unique<FontAtlas>();
    if (!m_Atlas) {
        LOG_ERROR("Failed to create font atlas for: {}", m_Name);
        return false;
    }
    
    m_IsLoaded = true;
    LOG_INFO("Font loaded successfully: {} (size: {})", m_Name, m_Size);
    
    return true;
}

void Font::Unload() {
    if (!m_IsLoaded) {
        return;
    }
    
    m_Glyphs.clear();
    m_Atlas.reset();
    m_IsLoaded = false;
    
    LOG_INFO("Font unloaded: {}", m_Name);
}

const Glyph* Font::GetGlyph(uint32_t character) const {
    if (!m_IsLoaded) {
        return nullptr;
    }
    
    auto it = m_Glyphs.find(character);
    if (it != m_Glyphs.end()) {
        return &it->second;
    }
    
    // Se o glyph não existe, retornar o glyph padrão (espaço)
    return GetGlyph(' ');
}

float Font::GetKerning(uint32_t left, uint32_t right) const {
    if (!m_IsLoaded) {
        return 0.0f;
    }
    
    // Implementação simples de kerning
    // Em uma implementação real, isso seria baseado em dados da fonte
    uint64_t key = (static_cast<uint64_t>(left) << 32) | right;
    
    auto it = m_Kerning.find(key);
    if (it != m_Kerning.end()) {
        return it->second;
    }
    
    return 0.0f;
}

glm::vec2 Font::MeasureText(const std::string& text) const {
    if (!m_IsLoaded || text.empty()) {
        return glm::vec2(0.0f);
    }
    
    float width = 0.0f;
    float height = 0.0f;
    float lineHeight = 0.0f;
    
    for (size_t i = 0; i < text.length(); ++i) {
        uint32_t character = static_cast<uint32_t>(text[i]);
        
        if (character == '\n') {
            width = std::max(width, lineHeight);
            lineHeight = 0.0f;
            continue;
        }
        
        const Glyph* glyph = GetGlyph(character);
        if (glyph) {
            lineHeight += glyph->advance.x;
            height = std::max(height, glyph->size.y);
            
            // Aplicar kerning se não for o último caractere
            if (i < text.length() - 1) {
                uint32_t nextChar = static_cast<uint32_t>(text[i + 1]);
                lineHeight += GetKerning(character, nextChar);
            }
        }
    }
    
    width = std::max(width, lineHeight);
    
    return glm::vec2(width, height);
}

float Font::GetLineHeight() const {
    if (!m_IsLoaded) {
        return m_Size;
    }
    
    // Em uma implementação real, isso seria baseado na métrica da fonte
    return m_Size * 1.2f; // 120% do tamanho da fonte
}

float Font::GetAscender() const {
    if (!m_IsLoaded) {
        return m_Size * 0.8f;
    }
    
    // Em uma implementação real, isso seria baseado na métrica da fonte
    return m_Size * 0.8f;
}

float Font::GetDescender() const {
    if (!m_IsLoaded) {
        return -m_Size * 0.2f;
    }
    
    // Em uma implementação real, isso seria baseado na métrica da fonte
    return -m_Size * 0.2f;
}

void Font::LoadBasicGlyphs() {
    // Carregar glyphs básicos (ASCII 32-126)
    for (uint32_t i = 32; i <= 126; ++i) {
        Glyph glyph;
        glyph.character = i;
        glyph.size = glm::vec2(m_Size * 0.6f, m_Size); // Largura aproximada
        glyph.bearing = glm::vec2(0.0f, m_Size * 0.8f); // Bearing aproximado
        glyph.advance = glm::vec2(m_Size * 0.6f, 0.0f); // Advance aproximado
        glyph.atlasRegion = nullptr; // Será configurado pelo atlas
        
        m_Glyphs[i] = glyph;
    }
    
    // Adicionar glyph de espaço
    Glyph spaceGlyph;
    spaceGlyph.character = ' ';
    spaceGlyph.size = glm::vec2(m_Size * 0.3f, m_Size);
    spaceGlyph.bearing = glm::vec2(0.0f, 0.0f);
    spaceGlyph.advance = glm::vec2(m_Size * 0.3f, 0.0f);
    spaceGlyph.atlasRegion = nullptr;
    
    m_Glyphs[' '] = spaceGlyph;
    
    LOG_INFO("Loaded {} basic glyphs for font: {}", m_Glyphs.size(), m_Name);
}

const std::string& Font::GetName() const {
    return m_Name;
}

const std::string& Font::GetFilePath() const {
    return m_FilePath;
}

float Font::GetSize() const {
    return m_Size;
}

FontQuality Font::GetQuality() const {
    return m_Quality;
}

bool Font::IsLoaded() const {
    return m_IsLoaded;
}

FontAtlas* Font::GetAtlas() const {
    return m_Atlas.get();
}

} // namespace Drift::UI 