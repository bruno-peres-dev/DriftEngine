#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <fstream>
#include <chrono>
#include "stb_truetype.h"
#include "Drift/UI/FontSystem/MSDFGenerator.h"

namespace Drift::UI {

Font::Font(const std::string& name, const std::string& filePath, float size, FontQuality quality)
    : m_Name(name)
    , m_FilePath(filePath)
    , m_Size(size)
    , m_Quality(quality)
    , m_IsLoaded(false) {
    // Construtor otimizado - sem logs desnecessários
}

Font::~Font() {
    Unload();
}

bool Font::Load() {
    if (m_IsLoaded) {
        return true;
    }

    LOG_INFO("Loading font: " + m_Name + " from " + m_FilePath);

    // Carregar arquivo TTF
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

    // Inicializar fonte STB
    if (!stbtt_InitFont(&m_FontInfo, m_TTFBuffer.data(), 0)) {
        LOG_ERROR("stbtt_InitFont failed for " + m_FilePath);
        return false;
    }

    // Calcular escala e métricas
    m_Scale = stbtt_ScaleForPixelHeight(&m_FontInfo, m_Size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_FontInfo, &ascent, &descent, &lineGap);
    m_Metrics.ascender = ascent * m_Scale;
    m_Metrics.descender = -descent * m_Scale;
    m_Metrics.lineHeight = (ascent - descent + lineGap) * m_Scale;

    // Criar atlas e gerar glyphs essenciais
    m_Atlas = std::make_unique<FontAtlas>();
    MSDFGenerator generator;
    
    // Carregar apenas caracteres essenciais inicialmente para melhor performance
    std::vector<uint32_t> essentialChars = {
        32,  // espaço
        33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,  // !"#$%&'()*+,-./
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57,  // 0-9
        58, 59, 60, 61, 62, 63, 64,  // :;<=>?@
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,  // A-Z
        91, 92, 93, 94, 95, 96,  // [\]^_`
        97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,  // a-z
        123, 124, 125, 126  // {|}~
    };
    
    // Gerar glyphs essenciais
    for (uint32_t cp : essentialChars) {
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

        // Calcular métricas do glyph
        int ax, lsb;
        stbtt_GetCodepointHMetrics(&m_FontInfo, cp, &ax, &lsb);
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&m_FontInfo, cp, m_Scale, m_Scale, &x0, &y0, &x1, &y1);

        // Criar e configurar glyph
        Glyph g{};
        g.codepoint = cp;
        g.position = {static_cast<float>(region->x), static_cast<float>(region->y)};
        g.size = {static_cast<float>(msdf.width), static_cast<float>(msdf.height)};
        g.offset = {static_cast<float>(x0), static_cast<float>(y0)};
        g.advance = static_cast<float>(ax) * m_Scale;
        g.uvMin = {region->x / static_cast<float>(m_Atlas->GetWidth()), 
                   region->y / static_cast<float>(m_Atlas->GetHeight())};
        g.uvMax = {(region->x + region->width) / static_cast<float>(m_Atlas->GetWidth()),
                   (region->y + region->height) / static_cast<float>(m_Atlas->GetHeight())};
        g.isValid = true;
        m_Glyphs[g.codepoint] = g;
    }

    m_IsLoaded = true;
    LOG_INFO("Font loaded successfully: " + m_Name + " (size: " + std::to_string(m_Size) + 
             ", glyphs: " + std::to_string(m_Glyphs.size()) + ")");
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
    
    std::lock_guard<std::mutex> lock(m_GlyphMutex);
    
    auto it = m_Glyphs.find(character);
    if (it != m_Glyphs.end()) {
        return &it->second;
    }
    
    // Lazy loading: carregar o glyph sob demanda
    if (character >= 32 && character <= 126) {
        // Usar const_cast para permitir modificação temporária durante lazy loading
        const_cast<Font*>(this)->LoadGlyph(character);
        it = m_Glyphs.find(character);
        if (it != m_Glyphs.end()) {
            return &it->second;
        }
    }
    
    // Se o glyph não existe, tentar retornar o glyph do espaço
    if (character != ' ') {
        return GetGlyph(' ');
    }
    
    // Se nem o espaço existe, retornar nullptr
    return nullptr;
}

bool Font::HasGlyph(uint32_t character) const {
    if (!m_IsLoaded) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_GlyphMutex);
    return m_Glyphs.find(character) != m_Glyphs.end();
}

void Font::PreloadGlyphs(const std::vector<uint32_t>& characters) {
    if (!m_IsLoaded) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_GlyphMutex);
    
    for (uint32_t character : characters) {
        if (m_Glyphs.find(character) == m_Glyphs.end()) {
            LoadGlyph(character);
        }
    }
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
    float lineWidth = 0.0f;
    
    for (size_t i = 0; i < text.length(); ++i) {
        uint32_t character = static_cast<uint32_t>(text[i]);
        
        if (character == '\n') {
            width = std::max(width, lineWidth);
            lineWidth = 0.0f;
            continue;
        }
        
        const Glyph* glyph = GetGlyph(character);
        if (glyph) {
            lineWidth += glyph->advance;
            height = std::max(height, glyph->size.y);
            
            // Aplicar kerning se não for o último caractere
            if (i < text.length() - 1) {
                uint32_t nextChar = static_cast<uint32_t>(text[i + 1]);
                lineWidth += GetKerning(character, nextChar);
            }
        }
    }
    
    width = std::max(width, lineWidth);
    
    return glm::vec2(width, height);
}

glm::vec2 Font::MeasureText(const std::wstring& text) const {
    if (!m_IsLoaded || text.empty()) {
        return glm::vec2(0.0f);
    }
    
    float width = 0.0f;
    float height = m_Metrics.lineHeight;
    float lineWidth = 0.0f;
    
    for (size_t i = 0; i < text.length(); ++i) {
        uint32_t character = static_cast<uint32_t>(text[i]);
        
        if (character == '\n') {
            width = std::max(width, lineWidth);
            lineWidth = 0.0f;
            continue;
        }
        
        const Glyph* glyph = GetGlyph(character);
        if (glyph) {
            lineWidth += glyph->advance;
            height = std::max(height, glyph->size.y);
            
            // Aplicar kerning se não for o último caractere
            if (i < text.length() - 1) {
                uint32_t nextChar = static_cast<uint32_t>(text[i + 1]);
                lineWidth += GetKerning(character, nextChar);
            }
        }
    }
    
    width = std::max(width, lineWidth);
    
    return glm::vec2(width, height);
}

std::vector<glm::vec2> Font::GetGlyphPositions(const std::string& text, float x, float y) const {
    if (!m_IsLoaded || text.empty()) {
        return {};
    }
    
    std::vector<glm::vec2> positions;
    positions.reserve(text.length());
    
    float currentX = x;
    float currentY = y;
    
    for (size_t i = 0; i < text.length(); ++i) {
        uint32_t character = static_cast<uint32_t>(text[i]);
        
        if (character == '\n') {
            currentX = x;
            currentY += m_Metrics.lineHeight;
            continue;
        }
        
        const Glyph* glyph = GetGlyph(character);
        if (glyph) {
            positions.emplace_back(currentX + glyph->offset.x, currentY + glyph->offset.y);
            currentX += glyph->advance;
            
            // Aplicar kerning se não for o último caractere
            if (i < text.length() - 1) {
                uint32_t nextChar = static_cast<uint32_t>(text[i + 1]);
                currentX += GetKerning(character, nextChar);
            }
        } else {
            positions.emplace_back(currentX, currentY);
        }
    }
    
    return positions;
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

void Font::LoadGlyph(uint32_t character) {
    if (!m_IsLoaded || !m_Atlas) {
        return;
    }
    
    // Verificar se o glyph já foi carregado
    if (m_Glyphs.find(character) != m_Glyphs.end()) {
        return;
    }
    
    // Calcular métricas do glyph primeiro
    int ax, lsb;
    stbtt_GetCodepointHMetrics(&m_FontInfo, character, &ax, &lsb);
    
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(&m_FontInfo, character, m_Scale, m_Scale, &x0, &y0, &x1, &y1);
    
    // Para caracteres como espaço, criar um glyph especial
    if (character == 32 || (x1 - x0) == 0) {
        // Criar um glyph vazio para o espaço
        Glyph g{};
        g.codepoint = character;
        g.position = {0.0f, 0.0f};
        g.size = {1.0f, 1.0f}; // Tamanho mínimo
        g.offset = {0.0f, 0.0f};
        g.advance = static_cast<float>(ax) * m_Scale;
        if (g.advance == 0.0f) {
            g.advance = m_Size * 0.3f; // Espaço padrão de 30% do tamanho da fonte
        }
        g.uvMin = {0.0f, 0.0f};
        g.uvMax = {0.0f, 0.0f};
        g.isValid = true;
        g.atlasId = 0; // Não está no atlas
        
        std::lock_guard<std::mutex> lock(m_GlyphMutex);
        m_Glyphs[g.codepoint] = g;
        return;
    }
    
    // Para caracteres normais, gerar MSDF
    MSDFGenerator generator;
    MSDFData msdf;
    if (!generator.GenerateFromGlyph(m_TTFBuffer.data(), character, msdf)) {
        // Se falhar, criar um glyph vazio mas com métricas corretas
        Glyph g{};
        g.codepoint = character;
        g.position = {0.0f, 0.0f};
        g.size = {static_cast<float>(x1 - x0), static_cast<float>(y1 - y0)};
        g.offset = {static_cast<float>(x0), static_cast<float>(y0)};
        g.advance = static_cast<float>(ax) * m_Scale;
        g.uvMin = {0.0f, 0.0f};
        g.uvMax = {0.0f, 0.0f};
        g.isValid = true;
        g.atlasId = 0;
        
        std::lock_guard<std::mutex> lock(m_GlyphMutex);
        m_Glyphs[g.codepoint] = g;
        return;
    }

    std::vector<uint8_t> pixels;
    generator.ConvertToRGBA8(msdf, pixels);

    AtlasRegion* region = m_Atlas->AllocateRegion(msdf.width, msdf.height, character);
    if (!region) {
        // Se não conseguir alocar região, criar glyph sem atlas
        Glyph g{};
        g.codepoint = character;
        g.position = {0.0f, 0.0f};
        g.size = {static_cast<float>(msdf.width), static_cast<float>(msdf.height)};
        g.offset = {static_cast<float>(x0), static_cast<float>(y0)};
        g.advance = static_cast<float>(ax) * m_Scale;
        g.uvMin = {0.0f, 0.0f};
        g.uvMax = {0.0f, 0.0f};
        g.isValid = true;
        g.atlasId = 0;
        
        std::lock_guard<std::mutex> lock(m_GlyphMutex);
        m_Glyphs[g.codepoint] = g;
        return;
    }
    
    m_Atlas->UploadMSDFData(region, pixels.data(), msdf.width, msdf.height);

    // Criar e configurar glyph
    Glyph g{};
    g.codepoint = character;
    g.position = {static_cast<float>(region->x), static_cast<float>(region->y)};
    g.size = {static_cast<float>(msdf.width), static_cast<float>(msdf.height)};
    g.offset = {static_cast<float>(x0), static_cast<float>(y0)};
    g.advance = static_cast<float>(ax) * m_Scale;
    g.uvMin = {region->x / static_cast<float>(m_Atlas->GetWidth()), 
               region->y / static_cast<float>(m_Atlas->GetHeight())};
    g.uvMax = {(region->x + region->width) / static_cast<float>(m_Atlas->GetWidth()),
               (region->y + region->height) / static_cast<float>(m_Atlas->GetHeight())};
    g.isValid = true;
    g.atlasId = 1; // Está no atlas
    
    std::lock_guard<std::mutex> lock(m_GlyphMutex);
    m_Glyphs[g.codepoint] = g;
}

void Font::LoadBasicGlyphs() {
    // Este método é chamado durante o carregamento inicial da fonte
    // Os glyphs básicos já são carregados no método Load()
    // Este método pode ser usado para carregar glyphs adicionais se necessário
}

void Font::CalculateMetrics() {
    if (!m_IsLoaded) {
        return;
    }
    
    // As métricas já são calculadas no método Load()
    // Este método pode ser usado para recalcular métricas se necessário
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_FontInfo, &ascent, &descent, &lineGap);
    m_Metrics.ascender = ascent * m_Scale;
    m_Metrics.descender = -descent * m_Scale;
    m_Metrics.lineHeight = (ascent - descent + lineGap) * m_Scale;
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

uint32_t Font::GetAtlasTextureId() const {
    if (m_Atlas) {
        Drift::RHI::ITexture* texture = m_Atlas->GetTexture();
        if (texture) {
            // Retornar um ID único baseado no ponteiro da textura
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(texture));
        }
    }
    return 0;
}

void Font::Touch() {
    m_LastUsed = std::chrono::steady_clock::now().time_since_epoch().count();
}

size_t Font::GetMemoryUsage() const {
    size_t totalSize = 0;
    
    // Tamanho do buffer TTF
    totalSize += m_TTFBuffer.size();
    
    // Tamanho do mapa de glyphs
    totalSize += m_Glyphs.size() * sizeof(Glyph);
    
    // Tamanho do mapa de kerning
    totalSize += m_Kerning.size() * sizeof(std::pair<uint64_t, float>);
    
    // Tamanho do atlas (se existir)
    if (m_Atlas) {
        totalSize += m_Atlas->GetWidth() * m_Atlas->GetHeight() * 4; // 4 bytes por pixel (RGBA)
    }
    
    return totalSize;
}

} // namespace Drift::UI 