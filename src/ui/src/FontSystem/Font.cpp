#include "Drift/UI/FontSystem/Font.h"
#include "Drift/Core/Log.h"
#include "stb_truetype.h"
#include <fstream>
#include <algorithm>

using namespace Drift::UI;

Font::Font(std::string name, float size, FontQuality quality)
    : m_Name(std::move(name)), m_Size(size), m_Quality(quality) {
    // Inicializar vector ASCII com tamanho fixo para caracteres 32-127
    m_GlyphsASCII.resize(96);  // 127 - 32 + 1 = 96 caracteres
}

const GlyphInfo* Font::GetGlyph(uint32_t codepoint) const {
    // Otimização: caracteres ASCII comuns (32-127) usam vector
    if (codepoint >= 32 && codepoint <= 127) {
        size_t index = codepoint - 32;
        if (index < m_GlyphsASCII.size()) {
            const GlyphInfo& g = m_GlyphsASCII[index];
            
            // Retornar o glyph se ele tem UVs válidas ou se é um espaço
            if (codepoint == 32 || g.uv0.x >= 0.0f || g.uv0.y >= 0.0f || g.advance > 0.0f) {
                return &g;
            }
        }
    }
    
    // Caracteres Unicode usam unordered_map
    auto it = m_GlyphsExtended.find(codepoint);
    if (it != m_GlyphsExtended.end()) {
        return &it->second;
    }
    
    return nullptr;
}

int Font::GetAtlasSize() const {
    switch (m_Quality) {
        case FontQuality::Low: return 256;
        case FontQuality::Medium: return 512;
        case FontQuality::High: return 1024;
        case FontQuality::Ultra: return 2048;
        default: return 1024;
    }
}

bool Font::LoadFromFile(const std::string& path, Drift::RHI::IDevice* device) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Drift::Core::LogError("[Font] Falha ao abrir arquivo: " + path);
        return false;
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        Drift::Core::LogError("[Font] Falha ao ler arquivo: " + path);
        return false;
    }
    
    return LoadFromMemory(buffer.data(), buffer.size(), device);
}

bool Font::LoadFromMemory(const unsigned char* data, size_t size, Drift::RHI::IDevice* device) {
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, data, stbtt_GetFontOffsetForIndex(data, 0))) {
        Drift::Core::LogError("[Font] stbtt_InitFont falhou");
        return false;
    }

    // Obter métricas da fonte
    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&info, m_Size);
    m_Ascent = ascent * scale;
    m_Descent = descent * scale;
    
    // Criar atlas de textura
    const int atlasSize = GetAtlasSize();
    std::vector<unsigned char> bitmap(atlasSize * atlasSize, 0);
    std::vector<stbtt_bakedchar> baked(96);
    
    int result = stbtt_BakeFontBitmap(data, 0, m_Size, bitmap.data(), atlasSize, atlasSize, 32, 96, baked.data());
    if (result <= 0) {
        Drift::Core::LogError("[Font] stbtt_BakeFontBitmap falhou");
        return false;
    }
    
    // Converter dados para GlyphInfo - otimização para ASCII
    for (int i = 0; i < 96; ++i) {
        stbtt_bakedchar& bc = baked[i];
        GlyphInfo& g = m_GlyphsASCII[i];
        
        g.uv0 = glm::vec2(bc.x0 / float(atlasSize), bc.y0 / float(atlasSize));
        g.uv1 = glm::vec2(bc.x1 / float(atlasSize), bc.y1 / float(atlasSize));
        g.size = glm::vec2(bc.x1 - bc.x0, bc.y1 - bc.y0);
        g.bearing = glm::vec2(bc.xoff, bc.yoff);
        g.advance = bc.xadvance;
        
        // Tratamento especial para o espaço (ASCII 32)
        if (i == 0) { // i=0 corresponde ao caractere 32 (espaço)
            // Obter métricas específicas do espaço usando stb_truetype
            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(&info, 32, &advanceWidth, &leftSideBearing);
            float spaceAdvance = advanceWidth * scale;
            
            // Garantir que o espaço tenha um advance mínimo
            if (g.advance <= 0.0f || spaceAdvance > g.advance) {
                g.advance = spaceAdvance > 0.0f ? spaceAdvance : m_Size * 0.3f;
            }
            
            // Para espaços, garantir que tenham uma área mínima no atlas
            if (g.size.x <= 0.0f || g.size.y <= 0.0f) {
                // Usar uma área mínima de 1x1 pixel no atlas
                g.size = glm::vec2(1.0f, 1.0f);
                g.uv0 = glm::vec2(0.0f, 0.0f);
                g.uv1 = glm::vec2(1.0f / float(atlasSize), 1.0f / float(atlasSize));
                g.bearing = glm::vec2(0.0f, 0.0f);
            }
        }
    }
    
    // Carregar caracteres Unicode adicionais se necessário
    LoadUnicodeGlyphs(data, atlasSize, bitmap);
    
    // Criar textura no device
    if (device) {
        Drift::RHI::TextureDesc desc;
        desc.width = atlasSize;
        desc.height = atlasSize;
        desc.format = Drift::RHI::Format::R8_UNORM;
        
        try {
            m_Texture = device->CreateTexture(desc);
            if (m_Texture) {
                m_Texture->UpdateSubresource(0, 0, bitmap.data(), atlasSize, atlasSize * atlasSize);
            } else {
                Drift::Core::LogError("[Font] Falha ao criar textura no device");
                return false;
            }
        } catch (const std::exception& e) {
            Drift::Core::LogException("[Font] Exceção ao criar textura", e);
            return false;
        }
    }

    Drift::Core::LogRHI("[Font] Fonte carregada: " + m_Name + " (tamanho: " + std::to_string(m_Size) + ")");
    return true;
}

void Font::LoadUnicodeGlyphs(const unsigned char* data, int atlasSize, std::vector<unsigned char>& bitmap) {
    // Carregar caracteres Unicode comuns (espaço para expansão)
    // Por enquanto, apenas ASCII é suportado para otimização
    // Em uma implementação completa, aqui carregaríamos caracteres Unicode específicos
}

