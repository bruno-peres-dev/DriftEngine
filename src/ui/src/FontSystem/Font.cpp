#include "Drift/UI/FontSystem/Font.h"
#include "Drift/Core/Log.h"
#include "stb_truetype.h"
#include <fstream>

using namespace Drift::UI;

Font::Font(std::string name, float size, FontQuality quality)
    : m_Name(std::move(name)), m_Size(size), m_Quality(quality) {
    m_Ascent = 0.0f;
    m_Descent = 0.0f;
}

const GlyphInfo* Font::GetGlyph(uint32_t codepoint) const {
    auto it = m_Glyphs.find(codepoint);
    if (it != m_Glyphs.end()) {
        return &it->second;
    }
    return nullptr;
}

bool Font::LoadFromFile(const std::string& path, Drift::RHI::IDevice* device) {
    Drift::Core::LogRHI("[Font] Iniciando carregamento da fonte: " + path);
    
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Drift::Core::LogError("[Font] Falha ao abrir arquivo: " + path);
        return false;
    }
    
    std::streamsize size = file.tellg();
    Drift::Core::LogRHIDebug("[Font] Tamanho do arquivo: " + std::to_string(size) + " bytes");
    
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        Drift::Core::LogError("[Font] Falha ao ler arquivo: " + path);
        return false;
    }
    
    Drift::Core::LogRHIDebug("[Font] Arquivo lido com sucesso");
    
    return LoadFromMemory(buffer.data(), buffer.size(), device);
}

bool Font::LoadFromMemory(const unsigned char* data, size_t size, Drift::RHI::IDevice* device) {
    Drift::Core::LogRHIDebug("[Font] Inicializando stb_truetype...");
    
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, data, stbtt_GetFontOffsetForIndex(data, 0))) {
        Drift::Core::LogError("[Font] stbtt_InitFont falhou");
        return false;
    }

    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&info, m_Size);
    m_Ascent = ascent * scale;
    m_Descent = descent * scale;
    
    Drift::Core::LogRHIDebug("[Font] stb_truetype inicializado com sucesso");

    Drift::Core::LogRHIDebug("[Font] Criando atlas de textura...");
    
    const int atlasSize = 512;
    std::vector<unsigned char> bitmap(atlasSize * atlasSize);
    std::vector<stbtt_bakedchar> baked(96);
    
    Drift::Core::LogRHIDebug("[Font] Chamando stbtt_BakeFontBitmap...");
    int result = stbtt_BakeFontBitmap(data, 0, m_Size, bitmap.data(), atlasSize, atlasSize, 32, 96, baked.data());
    if (result <= 0) {
        Drift::Core::LogError("[Font] stbtt_BakeFontBitmap falhou");
        return false;
    }
    
    Drift::Core::LogRHIDebug("[Font] Atlas criado com sucesso. Resultado: " + std::to_string(result));

    Drift::Core::LogRHIDebug("[Font] Convertendo dados de glyphs...");
    
    // Converter baked data para GlyphInfo
    for (int i = 0; i < 96; ++i) {
        stbtt_bakedchar& bc = baked[i];
        GlyphInfo g;
        g.uv0 = glm::vec2(bc.x0 / float(atlasSize), bc.y0 / float(atlasSize));
        g.uv1 = glm::vec2(bc.x1 / float(atlasSize), bc.y1 / float(atlasSize));
        g.size = glm::vec2(bc.x1 - bc.x0, bc.y1 - bc.y0);
        
        // O bearing do stb_truetype já está correto
        // xoff é o offset horizontal do glyph
        // yoff é o offset vertical do glyph (já relativo à baseline)
        g.bearing = glm::vec2(bc.xoff, bc.yoff);
        g.advance = bc.xadvance;
        
        Drift::Core::LogRHIDebug("[Font] Glyph " + std::to_string(32 + i) + 
                                " size: (" + std::to_string(g.size.x) + ", " + std::to_string(g.size.y) + ")" +
                                " bearing: (" + std::to_string(g.bearing.x) + ", " + std::to_string(g.bearing.y) + ")" +
                                " advance: " + std::to_string(g.advance));
        
        m_Glyphs[32 + i] = g;
    }
    
    Drift::Core::LogRHIDebug("[Font] Glyphs convertidos com sucesso");

    if (device) {
        Drift::Core::LogRHIDebug("[Font] Criando textura no device...");
        
        // Usar formato R8_UNORM para compatibilidade com shader bitmap
        // O bitmap já está no formato correto (um canal)
        Drift::RHI::TextureDesc desc;
        desc.width = atlasSize;
        desc.height = atlasSize;
        desc.format = Drift::RHI::Format::R8_UNORM;
        
        try {
            m_Texture = device->CreateTexture(desc);
            if (m_Texture) {
                Drift::Core::LogRHIDebug("[Font] Textura criada, atualizando subresource...");
                m_Texture->UpdateSubresource(0, 0, bitmap.data(), atlasSize, atlasSize * atlasSize);
                Drift::Core::LogRHIDebug("[Font] Subresource atualizado com sucesso");
            } else {
                Drift::Core::LogError("[Font] Falha ao criar textura no device");
            }
        } catch (const std::exception& e) {
            Drift::Core::LogException("[Font] Exceção ao criar textura", e);
            return false;
        }
    }

    Drift::Core::LogRHI("[Font] Fonte carregada com sucesso: " + m_Name + " (tamanho: " + std::to_string(m_Size) + ")");
    return true;
}

