#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include "stb_truetype.h"

// Simular a estrutura GlyphInfo
struct GlyphInfo {
    float uv0[2];
    float uv1[2];
    float size[2];
    float bearing[2];
    float advance;
};

// Simular a estrutura stbtt_bakedchar
struct stbtt_bakedchar {
    unsigned short x0, y0, x1, y1;
    float xoff, yoff, xadvance;
};

void TestFontAtlasCreation() {
    std::cout << "=== Teste de Criação do Atlas de Fontes ===" << std::endl;
    
    // Carregar arquivo de fonte
    std::string fontPath = "fonts/Arial-Regular.ttf";
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cout << "ERRO: Não foi possível abrir o arquivo de fonte: " << fontPath << std::endl;
        return;
    }
    
    std::streamsize size = file.tellg();
    std::cout << "Tamanho do arquivo: " << size << " bytes" << std::endl;
    
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cout << "ERRO: Falha ao ler arquivo de fonte" << std::endl;
        return;
    }
    
    // Inicializar stb_truetype
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, buffer.data(), stbtt_GetFontOffsetForIndex(buffer.data(), 0))) {
        std::cout << "ERRO: stbtt_InitFont falhou" << std::endl;
        return;
    }
    
    std::cout << "stb_truetype inicializado com sucesso" << std::endl;
    
    // Obter métricas da fonte
    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    float fontSize = 24.0f;
    float scale = stbtt_ScaleForPixelHeight(&info, fontSize);
    
    std::cout << "Métricas da fonte:" << std::endl;
    std::cout << "  Ascent: " << ascent << " (escalado: " << (ascent * scale) << ")" << std::endl;
    std::cout << "  Descent: " << descent << " (escalado: " << (descent * scale) << ")" << std::endl;
    std::cout << "  LineGap: " << lineGap << " (escalado: " << (lineGap * scale) << ")" << std::endl;
    std::cout << "  Scale: " << scale << std::endl;
    
    // Criar atlas
    const int atlasSize = 512;
    std::vector<unsigned char> bitmap(atlasSize * atlasSize, 0);  // Inicializar com zeros
    std::vector<stbtt_bakedchar> baked(96);
    
    std::cout << "Criando atlas de " << atlasSize << "x" << atlasSize << " pixels..." << std::endl;
    
    int result = stbtt_BakeFontBitmap(buffer.data(), 0, fontSize, bitmap.data(), atlasSize, atlasSize, 32, 96, baked.data());
    if (result <= 0) {
        std::cout << "ERRO: stbtt_BakeFontBitmap falhou com resultado: " << result << std::endl;
        return;
    }
    
    std::cout << "Atlas criado com sucesso! Resultado: " << result << std::endl;
    
    // Verificar se há dados no bitmap
    int nonZeroPixels = 0;
    int maxValue = 0;
    for (int i = 0; i < atlasSize * atlasSize; ++i) {
        if (bitmap[i] > 0) {
            nonZeroPixels++;
            if (bitmap[i] > maxValue) {
                maxValue = bitmap[i];
            }
        }
    }
    
    std::cout << "Estatísticas do bitmap:" << std::endl;
    std::cout << "  Pixels não-zero: " << nonZeroPixels << std::endl;
    std::cout << "  Total de pixels: " << (atlasSize * atlasSize) << std::endl;
    std::cout << "  Densidade: " << (float(nonZeroPixels) / (atlasSize * atlasSize) * 100.0f) << "%" << std::endl;
    std::cout << "  Valor máximo: " << maxValue << std::endl;
    
    // Verificar alguns glyphs específicos
    std::vector<char> testChars = {'A', 'B', 'C', 'a', 'b', 'c', '0', '1', '2', ' '};
    
    std::cout << "\nVerificando glyphs específicos:" << std::endl;
    for (char c : testChars) {
        int index = c - 32;
        if (index >= 0 && index < 96) {
            stbtt_bakedchar& bc = baked[index];
            std::cout << "  '" << c << "' (ASCII " << (int)c << "):" << std::endl;
            std::cout << "    Posição: (" << bc.x0 << ", " << bc.y0 << ") -> (" << bc.x1 << ", " << bc.y1 << ")" << std::endl;
            std::cout << "    Tamanho: " << (bc.x1 - bc.x0) << "x" << (bc.y1 - bc.y0) << std::endl;
            std::cout << "    Bearing: (" << bc.xoff << ", " << bc.yoff << ")" << std::endl;
            std::cout << "    Advance: " << bc.xadvance << std::endl;
            
            // Verificar se há dados no bitmap para este glyph
            int glyphPixels = 0;
            for (int y = bc.y0; y < bc.y1; ++y) {
                for (int x = bc.x0; x < bc.x1; ++x) {
                    if (y >= 0 && y < atlasSize && x >= 0 && x < atlasSize) {
                        if (bitmap[y * atlasSize + x] > 0) {
                            glyphPixels++;
                        }
                    }
                }
            }
            std::cout << "    Pixels não-zero: " << glyphPixels << std::endl;
        }
    }
    
    // Salvar uma amostra do bitmap para debug (primeiros 64x64 pixels)
    std::cout << "\nSalvando amostra do bitmap (64x64 pixels)..." << std::endl;
    std::ofstream debugFile("font_atlas_debug.txt");
    if (debugFile.is_open()) {
        debugFile << "Amostra do atlas de fontes (64x64 pixels):" << std::endl;
        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 64; ++x) {
                int index = y * atlasSize + x;
                if (index < atlasSize * atlasSize) {
                    int value = bitmap[index];
                    if (value > 0) {
                        debugFile << std::setw(3) << value << " ";
                    } else {
                        debugFile << "  0 ";
                    }
                }
            }
            debugFile << std::endl;
        }
        debugFile.close();
        std::cout << "Amostra salva em 'font_atlas_debug.txt'" << std::endl;
    }
    
    std::cout << "\nTeste concluído!" << std::endl;
}

int main() {
    TestFontAtlasCreation();
    return 0;
} 