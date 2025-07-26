#include "Drift/UI/FontSystem/Font.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/Core/Log.h"
#include <iostream>

using namespace Drift::UI;

int main() {
    std::cout << "=== Teste das Correções de Fonte ===" << std::endl;
    
    // Teste 1: Carregamento de fonte
    std::cout << "\n1. Testando carregamento de fonte..." << std::endl;
    
    FontManager& fm = FontManager::GetInstance();
    auto font = fm.GetOrLoadFont("Arial", "fonts/Arial-Regular.ttf", 24.0f);
    
    if (!font) {
        std::cout << "ERRO: Falha ao carregar fonte" << std::endl;
        return -1;
    }
    
    std::cout << "SUCESSO: Fonte carregada" << std::endl;
    std::cout << "  - Nome: " << font->GetName() << std::endl;
    std::cout << "  - Tamanho: " << font->GetSize() << std::endl;
    std::cout << "  - Ascent: " << font->GetAscent() << std::endl;
    std::cout << "  - Descent: " << font->GetDescent() << std::endl;
    
    // Teste 2: Verificação de glyphs
    std::cout << "\n2. Testando glyphs..." << std::endl;
    
    const char* testChars = "Hello World!";
    for (const char* c = testChars; *c; ++c) {
        const GlyphInfo* glyph = font->GetGlyph(static_cast<unsigned char>(*c));
        if (glyph) {
            std::cout << "  '" << *c << "': size(" << glyph->size.x << ", " << glyph->size.y 
                      << ") bearing(" << glyph->bearing.x << ", " << glyph->bearing.y 
                      << ") advance: " << glyph->advance << std::endl;
        } else {
            std::cout << "  '" << *c << "': GLYPH NÃO ENCONTRADO" << std::endl;
        }
    }
    
    // Teste 3: Medição de texto
    std::cout << "\n3. Testando medição de texto..." << std::endl;
    
    TextRenderer renderer;
    glm::vec2 size = renderer.MeasureText("Hello World!", "Arial", 24.0f);
    std::cout << "  Tamanho do texto 'Hello World!': (" << size.x << ", " << size.y << ")" << std::endl;
    
    // Teste 4: Verificação de posicionamento
    std::cout << "\n4. Testando posicionamento..." << std::endl;
    
    // Simular o cálculo de posicionamento que o TextRenderer faz
    glm::vec2 pos(100.0f, 100.0f);
    float baseline = pos.y + font->GetAscent();
    std::cout << "  Posição base: (" << pos.x << ", " << pos.y << ")" << std::endl;
    std::cout << "  Baseline: " << baseline << std::endl;
    
    float x = pos.x;
    for (const char* c = testChars; *c; ++c) {
        const GlyphInfo* g = font->GetGlyph(static_cast<unsigned char>(*c));
        if (g) {
            float xpos = x + g->bearing.x;
            float ypos = baseline + g->bearing.y; // CORREÇÃO: agora adiciona em vez de subtrair
            
            std::cout << "  '" << *c << "': pos(" << xpos << ", " << ypos << ")" << std::endl;
            x += g->advance;
        }
    }
    
    std::cout << "\n=== Teste Concluído ===" << std::endl;
    std::cout << "Correções aplicadas:" << std::endl;
    std::cout << "1. Formato de textura alterado de R8_UNORM para RGBA8_UNORM" << std::endl;
    std::cout << "2. Shaders simplificados para funcionar com bitmap em vez de MSDF" << std::endl;
    std::cout << "3. Posicionamento vertical corrigido (yoff agora é adicionado)" << std::endl;
    
    return 0;
} 