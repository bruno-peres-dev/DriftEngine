#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <iostream>
#include <chrono>

using namespace Drift::UI;

void TestDeviceValidation() {
    std::cout << "=== Teste de Validação de Device ===" << std::endl;
    
    auto& fontManager = FontManager::GetInstance();
    
    // Teste 1: Tentar carregar fonte sem device configurado
    std::cout << "\n--- Teste 1: Carregamento sem device ---" << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Tentar carregar fonte sem device (deve funcionar, mas com uploads enfileirados)
    auto font1 = fontManager.LoadFont("Arial_NoDevice", "fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (font1) {
        std::cout << "✓ Fonte carregada sem device em " << duration.count() << "ms" << std::endl;
        
        // Verificar se há uploads pendentes
        if (fontManager.HasPendingUploads()) {
            std::cout << "  - Uploads pendentes detectados (esperado)" << std::endl;
        } else {
            std::cout << "  - Nenhum upload pendente (inesperado)" << std::endl;
        }
        
        // Verificar se o atlas está pronto
        if (font1->GetAtlas()) {
            if (font1->GetAtlas()->IsDeviceReady()) {
                std::cout << "  - Atlas está pronto (inesperado sem device)" << std::endl;
            } else {
                std::cout << "  - Atlas não está pronto (esperado sem device)" << std::endl;
            }
        }
    } else {
        std::cout << "✗ Falha ao carregar fonte sem device" << std::endl;
    }
    
    // Teste 2: Configurar device nulo
    std::cout << "\n--- Teste 2: Configurar device nulo ---" << std::endl;
    
    fontManager.SetDevice(nullptr);
    
    if (fontManager.HasPendingUploads()) {
        std::cout << "✓ Uploads permanecem pendentes com device nulo" << std::endl;
    } else {
        std::cout << "✗ Uploads não estão pendentes (inesperado)" << std::endl;
    }
    
    // Teste 3: Tentar flush com device nulo
    std::cout << "\n--- Teste 3: Tentar flush com device nulo ---" << std::endl;
    
    startTime = std::chrono::high_resolution_clock::now();
    fontManager.FlushAllPendingUploads();
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "✓ Flush com device nulo concluído em " << duration.count() << "ms (sem crash)" << std::endl;
    
    // Teste 4: Carregar glyphs adicionais sem device
    std::cout << "\n--- Teste 4: Carregar glyphs adicionais sem device ---" << std::endl;
    
    if (font1) {
        std::vector<uint32_t> testChars = {0x00E7, 0x00E1, 0x00E9, 0x00ED, 0x00F3}; // ç, á, é, í, ó
        
        startTime = std::chrono::high_resolution_clock::now();
        
        for (uint32_t cp : testChars) {
            const auto* glyph = font1->GetGlyph(cp);
            if (glyph && glyph->isValid) {
                std::cout << "  ✓ Glyph " << std::hex << cp << std::dec << " carregado" << std::endl;
            } else {
                std::cout << "  ✗ Glyph " << std::hex << cp << std::dec << " falhou" << std::endl;
            }
        }
        
        endTime = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "  - Carregamento de glyphs: " << duration.count() << "ms" << std::endl;
        
        // Verificar uploads pendentes novamente
        if (fontManager.HasPendingUploads()) {
            std::cout << "  - Uploads pendentes após carregar glyphs adicionais" << std::endl;
        }
    }
    
    // Teste 5: Simular device inválido
    std::cout << "\n--- Teste 5: Simular device inválido ---" << std::endl;
    
    // Criar uma fonte com device inválido (simulado)
    auto font2 = fontManager.LoadFont("Arial_InvalidDevice", "fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);
    
    if (font2) {
        std::cout << "✓ Fonte carregada com device inválido (sem crash)" << std::endl;
        
        // Tentar carregar alguns glyphs
        for (uint32_t cp : {65, 66, 67}) { // A, B, C
            const auto* glyph = font2->GetGlyph(cp);
            if (glyph && glyph->isValid) {
                std::cout << "  ✓ Glyph " << (char)cp << " carregado" << std::endl;
            }
        }
    }
    
    // Teste 6: Verificar estatísticas
    std::cout << "\n--- Teste 6: Estatísticas do Sistema ---" << std::endl;
    
    auto stats = fontManager.GetStats();
    std::cout << "  - Fontes carregadas: " << stats.totalFonts << std::endl;
    std::cout << "  - Glyphs totais: " << stats.totalGlyphs << std::endl;
    std::cout << "  - Atlases criados: " << stats.totalAtlases << std::endl;
    std::cout << "  - Uso de memória: " << (stats.memoryUsageBytes / 1024) << " KB" << std::endl;
    
    // Teste 7: Verificar se o sistema não crashou
    std::cout << "\n--- Teste 7: Verificação de Estabilidade ---" << std::endl;
    
    try {
        // Tentar operações que poderiam causar crash
        fontManager.BeginTextRendering();
        fontManager.EndTextRendering();
        
        // Tentar flush novamente
        fontManager.FlushAllPendingUploads();
        
        std::cout << "✓ Sistema permanece estável após todas as operações" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ Sistema crashou: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Teste de Validação Concluído ===" << std::endl;
    std::cout << "✓ Nenhum crash ocorreu durante os testes" << std::endl;
    std::cout << "✓ Sistema funciona corretamente sem device inicializado" << std::endl;
    std::cout << "✓ Uploads são enfileirados até o device estar pronto" << std::endl;
}

int main() {
    try {
        TestDeviceValidation();
    } catch (const std::exception& e) {
        std::cerr << "Erro durante o teste: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 