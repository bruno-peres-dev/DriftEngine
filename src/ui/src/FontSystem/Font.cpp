#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <fstream>
#include "stb_truetype.h"

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

    LOG_INFO("Loading font: {} from {}", m_Name, m_FilePath);

    std::ifstream file(m_FilePath, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to open font file: {}", m_FilePath);
        return false;
    }
    file.seekg(0, std::ios::end);
    size_t len = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    m_TTFBuffer.resize(len);
    file.read(reinterpret_cast<char*>(m_TTFBuffer.data()), len);
    file.close();

    if (!stbtt_InitFont(&m_FontInfo, m_TTFBuffer.data(), 0)) {
        LOG_ERROR("stbtt_InitFont failed for {}", m_FilePath);
        return false;
    }

    m_Scale = stbtt_ScaleForPixelHeight(&m_FontInfo, m_Size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_FontInfo, &ascent, &descent, &lineGap);
    m_Ascender = ascent * m_Scale;
    m_Descender = -descent * m_Scale;
    m_LineHeight = (ascent - descent + lineGap) * m_Scale;

    m_BitmapWidth = 512;
    m_BitmapHeight = 512;
    m_Bitmap.assign(m_BitmapWidth * m_BitmapHeight, 0);

    std::vector<stbtt_bakedchar> baked(96);
    int res = stbtt_BakeFontBitmap(m_TTFBuffer.data(), 0, m_Size,
                                   m_Bitmap.data(), m_BitmapWidth, m_BitmapHeight,
                                   32, 96, baked.data());
    if (res <= 0) {
        LOG_ERROR("stbtt_BakeFontBitmap failed for {}", m_FilePath);
        return false;
    }

    for (int i = 0; i < 96; ++i) {
        const auto& bc = baked[i];
        Glyph g{};
        g.codepoint = i + 32;
        g.position = {static_cast<float>(bc.x0), static_cast<float>(bc.y0)};
        g.size = {static_cast<float>(bc.x1 - bc.x0), static_cast<float>(bc.y1 - bc.y0)};
        g.offset = {bc.xoff, bc.yoff};
        g.advance = bc.xadvance;
        g.uvMin = {bc.x0 / static_cast<float>(m_BitmapWidth), bc.y0 / static_cast<float>(m_BitmapHeight)};
        g.uvMax = {bc.x1 / static_cast<float>(m_BitmapWidth), bc.y1 / static_cast<float>(m_BitmapHeight)};
        g.isValid = true;
        m_Glyphs[g.codepoint] = g;
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
    m_Bitmap.clear();
    m_TTFBuffer.clear();
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
    
    return stbtt_GetCodepointKernAdvance(&m_FontInfo, left, right) * m_Scale;
}

glm::vec2 Font::MeasureText(const std::string& text) const {
    if (!m_IsLoaded || text.empty()) {
        return glm::vec2(0.0f);
    }
    
    float width = 0.0f;
    float height = m_LineHeight;
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
    return m_LineHeight;
}

float Font::GetAscender() const {
    if (!m_IsLoaded) {
        return m_Size * 0.8f;
    }
    return m_Ascender;
}

float Font::GetDescender() const {
    if (!m_IsLoaded) {
        return -m_Size * 0.2f;
    }
    return m_Descender;
}

void Font::LoadBasicGlyphs() {
    // Obsoleto: glyphs são gerados em Font::Load usando stb_truetype
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