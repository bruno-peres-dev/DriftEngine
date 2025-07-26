#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <fstream>
#include "stb_truetype.h"
#include "Drift/UI/FontSystem/MSDFGenerator.h"

namespace Drift::UI {

Font::Font(const std::string& name, const std::string& filePath, float size, FontQuality quality)
    : m_Name(name)
    , m_FilePath(filePath)
    , m_Size(size)
    , m_Quality(quality)
    , m_IsLoaded(false) {
    Core::Log("[Font] Construtor chamado para: " + name);
    Core::Log("[Font]   - Arquivo: " + filePath);
    Core::Log("[Font]   - Tamanho: " + std::to_string(size));
    Core::Log("[Font]   - Qualidade: " + std::to_string(static_cast<int>(quality)));
}

Font::~Font() {
    Core::Log("[Font] Destrutor chamado para: " + m_Name);
    Unload();
    Core::Log("[Font] Destrutor concluido para: " + m_Name);
}

bool Font::Load() {
    if (m_IsLoaded) {
        return true;
    }

    LOG_INFO("Loading font: " + m_Name + " from " + m_FilePath);

    std::ifstream file(m_FilePath, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to open font file: " + m_FilePath);
        return false;
    }
    file.seekg(0, std::ios::end);
    size_t len = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    m_TTFBuffer.resize(len);
    file.read(reinterpret_cast<char*>(m_TTFBuffer.data()), len);
    file.close();

    if (!stbtt_InitFont(&m_FontInfo, m_TTFBuffer.data(), 0)) {
        LOG_ERROR("stbtt_InitFont failed for " + m_FilePath);
        return false;
    }

    m_Scale = stbtt_ScaleForPixelHeight(&m_FontInfo, m_Size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_FontInfo, &ascent, &descent, &lineGap);
    m_Metrics.ascender = ascent * m_Scale;
    m_Metrics.descender = -descent * m_Scale;
    m_Metrics.lineHeight = (ascent - descent + lineGap) * m_Scale;

    // Gerar glyphs usando MSDF e armazenar no atlas
    m_Atlas = std::make_unique<FontAtlas>();
    MSDFGenerator generator;
    for (uint32_t cp = 32; cp < 128; ++cp) {
        MSDFData msdf;
        if (!generator.GenerateFromGlyph(m_TTFBuffer.data(), cp, msdf)) {
            continue;
        }

        std::vector<uint8_t> pixels;
        generator.ConvertToRGBA8(msdf, pixels);

        AtlasRegion* region = m_Atlas->AllocateRegion(msdf.width, msdf.height, cp);
        if (!region) {
            continue;
        }
        m_Atlas->UploadMSDFData(region, pixels.data(), msdf.width, msdf.height);

        int ax, lsb;
        stbtt_GetCodepointHMetrics(&m_FontInfo, cp, &ax, &lsb);
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&m_FontInfo, cp, m_Scale, m_Scale, &x0, &y0, &x1, &y1);

        Glyph g{};
        g.codepoint = cp;
        g.position = {static_cast<float>(region->x), static_cast<float>(region->y)};
        g.size = {static_cast<float>(msdf.width), static_cast<float>(msdf.height)};
        g.offset = {static_cast<float>(x0), static_cast<float>(y0)};
        g.advance = static_cast<float>(ax) * m_Scale;
        g.uvMin = {region->x / static_cast<float>(m_Atlas->GetWidth()), region->y / static_cast<float>(m_Atlas->GetHeight())};
        g.uvMax = {(region->x + region->width) / static_cast<float>(m_Atlas->GetWidth()),
                   (region->y + region->height) / static_cast<float>(m_Atlas->GetHeight())};
        g.isValid = true;
        m_Glyphs[g.codepoint] = g;
    }

    m_IsLoaded = true;
    LOG_INFO("Font loaded successfully: " + m_Name + " (size: " + std::to_string(m_Size) + ")");
    return true;
}

void Font::Unload() {
    if (!m_IsLoaded) {
        return;
    }

    m_Glyphs.clear();
    m_TTFBuffer.clear();
    m_Atlas.reset();
    m_IsLoaded = false;

    LOG_INFO("Font unloaded: " + m_Name);
}

const Glyph* Font::GetGlyph(uint32_t character) const {
    if (!m_IsLoaded) {
        return nullptr;
    }
    
    auto it = m_Glyphs.find(character);
    if (it != m_Glyphs.end()) {
        return &it->second;
    }
    
    // Se o glyph não existe, tentar retornar o glyph do espaço
    if (character != ' ') {
        return GetGlyph(' ');
    }
    
    // Se nem o espaço existe, retornar nullptr
    return nullptr;
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
    float height = m_Metrics.lineHeight;
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
            lineHeight += glyph->advance;
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
    return m_Metrics.lineHeight;
}

float Font::GetAscender() const {
    if (!m_IsLoaded) {
        return m_Size * 0.8f;
    }
    return m_Metrics.ascender;
}

float Font::GetDescender() const {
    if (!m_IsLoaded) {
        return -m_Size * 0.2f;
    }
    return m_Metrics.descender;
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
    return m_IsLoaded.load();
}

const std::unique_ptr<FontAtlas>& Font::GetAtlas() const {
    return m_Atlas;
}

} // namespace Drift::UI 