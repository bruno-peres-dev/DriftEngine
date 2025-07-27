#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector> // Added for std::vector

// Simular a função ConvertARGBtoRGBA corrigida
inline uint32_t ConvertARGBtoRGBA(uint32_t argb) {
    uint8_t a = (argb >> 24) & 0xFF;
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >> 8) & 0xFF;
    uint8_t b = argb & 0xFF;
    
    // Converter ARGB para RGBA: reordenar bytes para R8G8B8A8_UNORM
    // ARGB: AAAA RRRR GGGG BBBB
    // RGBA: RRRR GGGG BBBB AAAA (reordenar para GPU)
    return (r << 24) | (g << 16) | (b << 8) | a;
}

// Função antiga (incorreta) para comparação
inline uint32_t ConvertARGBtoRGBA_OLD(uint32_t argb) {
    uint8_t a = (argb >> 24) & 0xFF;
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >> 8) & 0xFF;
    uint8_t b = argb & 0xFF;
    
    // Retorna o mesmo valor (incorreto)
    return (a << 24) | (r << 16) | (g << 8) | b;
}

void PrintColor(const std::string& name, uint32_t color) {
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    std::cout << std::setw(15) << name << ": ";
    std::cout << "A=" << std::setw(3) << (int)a << " ";
    std::cout << "R=" << std::setw(3) << (int)r << " ";
    std::cout << "G=" << std::setw(3) << (int)g << " ";
    std::cout << "B=" << std::setw(3) << (int)b << " ";
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << color << std::dec << std::setfill(' ');
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Teste de Conversão ARGB para RGBA ===" << std::endl;
    std::cout << std::endl;
    
    // Cores de teste
    std::vector<std::pair<std::string, uint32_t>> testColors = {
        {"Branco", 0xFFFFFFFF},
        {"Preto", 0xFF000000},
        {"Vermelho", 0xFFFF0000},
        {"Verde", 0xFF00FF00},
        {"Azul", 0xFF0000FF},
        {"Amarelo", 0xFFFFFF00},
        {"Magenta", 0xFFFF00FF},
        {"Ciano", 0xFF00FFFF},
        {"Cinza 50%", 0xFF808080},
        {"Transparente", 0x00000000},
        {"Semi-transparente", 0x80000000},
    };
    
    for (const auto& [name, originalColor] : testColors) {
        std::cout << "--- " << name << " ---" << std::endl;
        
        PrintColor("Original (ARGB)", originalColor);
        
        uint32_t convertedOld = ConvertARGBtoRGBA_OLD(originalColor);
        PrintColor("Convertido (OLD)", convertedOld);
        
        uint32_t convertedNew = ConvertARGBtoRGBA(originalColor);
        PrintColor("Convertido (NEW)", convertedNew);
        
        // Verificar se a conversão está correta
        uint8_t a_orig = (originalColor >> 24) & 0xFF;
        uint8_t r_orig = (originalColor >> 16) & 0xFF;
        uint8_t g_orig = (originalColor >> 8) & 0xFF;
        uint8_t b_orig = originalColor & 0xFF;
        
        uint8_t a_new = (convertedNew >> 24) & 0xFF;
        uint8_t r_new = (convertedNew >> 16) & 0xFF;
        uint8_t g_new = (convertedNew >> 8) & 0xFF;
        uint8_t b_new = convertedNew & 0xFF;
        
        bool correct = (r_new == r_orig) && (g_new == g_orig) && (b_new == b_orig) && (a_new == a_orig);
        
        if (correct) {
            std::cout << "✅ Conversão CORRETA" << std::endl;
        } else {
            std::cout << "❌ Conversão INCORRETA" << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "=== Resumo ===" << std::endl;
    std::cout << "A conversão ARGB->RGBA deve reordenar os bytes:" << std::endl;
    std::cout << "ARGB: AAAA RRRR GGGG BBBB" << std::endl;
    std::cout << "RGBA: RRRR GGGG BBBB AAAA" << std::endl;
    std::cout << std::endl;
    std::cout << "Isso garante que as cores sejam interpretadas corretamente" << std::endl;
    std::cout << "pelo formato R8G8B8A8_UNORM do DirectX." << std::endl;
    
    return 0;
} 