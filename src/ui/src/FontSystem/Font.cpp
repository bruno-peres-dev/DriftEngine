#include "Drift/UI/FontSystem/Font.h"
#include "Drift/Core/Log.h"
#include "stb_truetype.h"
#include <fstream>
#include <algorithm>

using namespace Drift::UI;

Font::Font(std::string name, float size, FontQuality quality)
    : m_Name(std::move(name)), m_Size(size), m_Quality(quality) {
    // Inicializar vector para caracteres Latin-1 (32-255)
    m_GlyphsASCII.resize(224);  // 255 - 32 + 1 = 224 caracteres
}

const GlyphInfo* Font::GetGlyph(uint32_t codepoint) const {
    // Otimização: caracteres Latin-1 (32-255) usam vector
    if (codepoint >= 32 && codepoint <= 255) {
        size_t index = codepoint - 32;
        if (index < m_GlyphsASCII.size()) {
            const GlyphInfo& g = m_GlyphsASCII[index];
            
            // Retornar o glyph se ele for um espaço ou se tem dados válidos
            if (codepoint == 32 ||
                (g.advance > 0.0f &&
                 g.uv1.x > g.uv0.x &&
                 g.uv1.y > g.uv0.y)) {
                return &g;
            }
        }
    }
    
    // Caracteres Unicode usam unordered_map
    auto it = m_GlyphsExtended.find(codepoint);
    if (it != m_GlyphsExtended.end()) {
        return &it->second;
    }
    
    // Sistema de fallback para caracteres acentuados comuns
    uint32_t fallback = GetFallbackCodepoint(codepoint);
    if (fallback != codepoint && fallback >= 32 && fallback <= 255) {
        size_t index = fallback - 32;
        if (index < m_GlyphsASCII.size()) {
            const GlyphInfo& g = m_GlyphsASCII[index];
            if (g.advance > 0.0f && g.uv1.x > g.uv0.x && g.uv1.y > g.uv0.y) {
                return &g;
            }
        }
    }
    
    return nullptr;
}

uint32_t Font::GetFallbackCodepoint(uint32_t codepoint) const {
    // Mapeamento de caracteres acentuados para seus equivalentes sem acento
    static const std::unordered_map<uint32_t, uint32_t> fallbackMap = {
        // Vogais acentuadas -> vogais simples
        {225, 97},   // á -> a
        {224, 97},   // à -> a  
        {226, 97},   // â -> a
        {227, 97},   // ã -> a
        {228, 97},   // ä -> a
        {233, 101},  // é -> e
        {232, 101},  // è -> e
        {234, 101},  // ê -> e
        {235, 101},  // ë -> e
        {237, 105},  // í -> i
        {236, 105},  // ì -> i
        {238, 105},  // î -> i
        {239, 105},  // ï -> i
        {243, 111},  // ó -> o
        {242, 111},  // ò -> o
        {244, 111},  // ô -> o
        {245, 111},  // õ -> o
        {246, 111},  // ö -> o
        {250, 117},  // ú -> u
        {249, 117},  // ù -> u
        {251, 117},  // û -> u
        {252, 117},  // ü -> u
        {231, 99},   // ç -> c
        {241, 110},  // ñ -> n
        
        // Maiúsculas acentuadas -> maiúsculas simples
        {193, 65},   // Á -> A
        {192, 65},   // À -> A
        {194, 65},   // Â -> A
        {195, 65},   // Ã -> A
        {196, 65},   // Ä -> A
        {201, 69},   // É -> E
        {200, 69},   // È -> E
        {202, 69},   // Ê -> E
        {203, 69},   // Ë -> E
        {205, 73},   // Í -> I
        {204, 73},   // Ì -> I
        {206, 73},   // Î -> I
        {207, 73},   // Ï -> I
        {211, 79},   // Ó -> O
        {210, 79},   // Ò -> O
        {212, 79},   // Ô -> O
        {213, 79},   // Õ -> O
        {214, 79},   // Ö -> O
        {218, 85},   // Ú -> U
        {217, 85},   // Ù -> U
        {219, 85},   // Û -> U
        {220, 85},   // Ü -> U
        {199, 67},   // Ç -> C
        {209, 78},   // Ñ -> N
    };
    
    auto it = fallbackMap.find(codepoint);
    return (it != fallbackMap.end()) ? it->second : codepoint;
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
    std::vector<stbtt_bakedchar> baked(224);  // Para caracteres 32-255
    
    int result = stbtt_BakeFontBitmap(data, 0, m_Size, bitmap.data(), atlasSize, atlasSize, 32, 224, baked.data());
    if (result <= 0) {
        Drift::Core::LogError("[Font] stbtt_BakeFontBitmap falhou");
        return false;
    }
    
    // Converter dados para GlyphInfo - caracteres Latin-1 (32-255)
    for (int i = 0; i < 224; ++i) {
        stbtt_bakedchar& bc = baked[i];
        GlyphInfo& g = m_GlyphsASCII[i];
        
        uint32_t codepoint = 32 + i;  // Calcular o codepoint atual
        
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

