#include <iostream>
#include <vector>
#include <cstdint>

// Simulação das estruturas para teste
struct GlyphInfo {
    float uv0[2];
    float uv1[2];
    float size[2];
    float bearing[2];
    float advance;
};

// Simulação do stb_truetype
struct stbtt_bakedchar {
    unsigned short x0, y0, x1, y1;
    float xoff, yoff, xadvance;
};

// Função para testar a geração do atlas
void TestFontAtlasGeneration() {
    std::cout << "=== Teste de Geração de Atlas de Fonte ===" << std::endl;
    
    const int atlasSize = 512;
    std::vector<unsigned char> bitmap(atlasSize * atlasSize, 0);
    std::vector<stbtt_bakedchar> baked(96);
    
    // Simular dados de glyph (valores de exemplo)
    for (int i = 0; i < 96; ++i) {
        baked[i].x0 = i * 8;
        baked[i].y0 = 0;
        baked[i].x1 = (i + 1) * 8;
        baked[i].y1 = 16;
        baked[i].xoff = 0;
        baked[i].yoff = 0;
        baked[i].xadvance = 8;
        
        // Simular dados de bitmap (criar um padrão simples)
        for (int y = baked[i].y0; y < baked[i].y1; ++y) {
            for (int x = baked[i].x0; x < baked[i].x1; ++x) {
                if (x < atlasSize && y < atlasSize) {
                    bitmap[y * atlasSize + x] = 255; // Branco
                }
            }
        }
    }
    
    // Converter para GlyphInfo
    std::vector<GlyphInfo> glyphs(96);
    for (int i = 0; i < 96; ++i) {
        stbtt_bakedchar& bc = baked[i];
        GlyphInfo& g = glyphs[i];
        
        g.uv0[0] = bc.x0 / float(atlasSize);
        g.uv0[1] = bc.y0 / float(atlasSize);
        g.uv1[0] = bc.x1 / float(atlasSize);
        g.uv1[1] = bc.y1 / float(atlasSize);
        g.size[0] = bc.x1 - bc.x0;
        g.size[1] = bc.y1 - bc.y0;
        g.bearing[0] = bc.xoff;
        g.bearing[1] = bc.yoff;
        g.advance = bc.xadvance;
        
        std::cout << "Glyph " << (32 + i) << " (char '" << (char)(32 + i) << "'):" << std::endl;
        std::cout << "  Size: (" << g.size[0] << ", " << g.size[1] << ")" << std::endl;
        std::cout << "  UV: (" << g.uv0[0] << ", " << g.uv0[1] << ") -> (" << g.uv1[0] << ", " << g.uv1[1] << ")" << std::endl;
        std::cout << "  Bearing: (" << g.bearing[0] << ", " << g.bearing[1] << ")" << std::endl;
        std::cout << "  Advance: " << g.advance << std::endl;
    }
    
    // Verificar se há dados no bitmap
    int nonZeroPixels = 0;
    for (int i = 0; i < atlasSize * atlasSize; ++i) {
        if (bitmap[i] > 0) {
            nonZeroPixels++;
        }
    }
    
    std::cout << "Pixels não-zero no bitmap: " << nonZeroPixels << std::endl;
    std::cout << "Total de pixels: " << (atlasSize * atlasSize) << std::endl;
    std::cout << "Densidade: " << (float(nonZeroPixels) / (atlasSize * atlasSize) * 100.0f) << "%" << std::endl;
}

// Função para testar a conversão de coordenadas
void TestCoordinateConversion() {
    std::cout << "\n=== Teste de Conversão de Coordenadas ===" << std::endl;
    
    float screenW = 1280.0f;
    float screenH = 720.0f;
    
    // Testar conversão de coordenadas de tela para clip space
    auto ToClipX = [screenW](float px) -> float {
        return (px / screenW) * 2.0f - 1.0f;
    };
    
    auto ToClipY = [screenH](float py) -> float {
        return 1.0f - (py / screenH) * 2.0f;
    };
    
    // Testar algumas posições
    std::vector<std::pair<float, float>> testPositions = {
        {0, 0},           // Canto superior esquerdo
        {screenW/2, screenH/2}, // Centro
        {screenW, screenH},     // Canto inferior direito
        {100, 100},       // Posição arbitrária
        {screenW-100, screenH-100} // Outra posição
    };
    
    for (const auto& pos : testPositions) {
        float clipX = ToClipX(pos.first);
        float clipY = ToClipY(pos.second);
        
        std::cout << "Screen (" << pos.first << ", " << pos.second << ") -> Clip (" << clipX << ", " << clipY << ")" << std::endl;
    }
}

// Função para testar a conversão de cores
void TestColorConversion() {
    std::cout << "\n=== Teste de Conversão de Cores ===" << std::endl;
    
    // Simular Drift::Color (ARGB)
    auto ConvertARGBtoRGBA = [](uint32_t argb) -> uint32_t {
        uint8_t a = (argb >> 24) & 0xFF;
        uint8_t r = (argb >> 16) & 0xFF;
        uint8_t g = (argb >> 8) & 0xFF;
        uint8_t b = argb & 0xFF;
        
        return (a << 24) | (r << 16) | (g << 8) | b;
    };
    
    // Testar algumas cores
    std::vector<uint32_t> testColors = {
        0xFFFFFFFF, // Branco
        0xFF000000, // Preto
        0xFFFF0000, // Vermelho
        0xFF00FF00, // Verde
        0xFF0000FF, // Azul
        0x80000000, // Preto semi-transparente
    };
    
    for (uint32_t color : testColors) {
        uint32_t converted = ConvertARGBtoRGBA(color);
        
        uint8_t a1 = (color >> 24) & 0xFF;
        uint8_t r1 = (color >> 16) & 0xFF;
        uint8_t g1 = (color >> 8) & 0xFF;
        uint8_t b1 = color & 0xFF;
        
        uint8_t a2 = (converted >> 24) & 0xFF;
        uint8_t r2 = (converted >> 16) & 0xFF;
        uint8_t g2 = (converted >> 8) & 0xFF;
        uint8_t b2 = converted & 0xFF;
        
        std::cout << "Original ARGB: (" << (int)a1 << ", " << (int)r1 << ", " << (int)g1 << ", " << (int)b1 << ")" << std::endl;
        std::cout << "Converted RGBA: (" << (int)a2 << ", " << (int)r2 << ", " << (int)g2 << ", " << (int)b2 << ")" << std::endl;
        std::cout << "---" << std::endl;
    }
}

int main() {
    std::cout << "Iniciando testes de correção de fontes..." << std::endl;
    
    TestFontAtlasGeneration();
    TestCoordinateConversion();
    TestColorConversion();
    
    std::cout << "\nTestes concluídos!" << std::endl;
    return 0;
} 