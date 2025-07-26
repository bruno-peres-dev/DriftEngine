#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <iostream>
#include <chrono>

using namespace Drift::UI;

void TestFontBatching() {
    std::cout << "=== Teste do Sistema de Batching de Fontes ===" << std::endl;
    
    auto& fontManager = FontManager::GetInstance();
    
    // Configurar device (simulado)
    fontManager.SetDevice(nullptr); // Em um caso real, seria um device real
    
    // Medir tempo de carregamento sem batching (simulado)
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Carregar uma fonte - isso agora usa batching automaticamente
    auto font = fontManager.LoadFont("Arial", "fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (font) {
        std::cout << "✓ Fonte carregada com sucesso em " << duration.count() << "ms" << std::endl;
        std::cout << "  - Nome: " << font->GetName() << std::endl;
        std::cout << "  - Tamanho: " << font->GetSize() << std::endl;
        std::cout << "  - Qualidade: " << static_cast<int>(font->GetQuality()) << std::endl;
        
        // Verificar se há uploads pendentes
        if (fontManager.HasPendingUploads()) {
            std::cout << "  - Uploads pendentes detectados" << std::endl;
            
            // Fazer flush manual
            startTime = std::chrono::high_resolution_clock::now();
            fontManager.FlushAllPendingUploads();
            endTime = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "  - Flush concluído em " << duration.count() << "ms" << std::endl;
        } else {
            std::cout << "  - Nenhum upload pendente" << std::endl;
        }
        
        // Testar carregamento de glyphs adicionais
        std::cout << "\n--- Testando carregamento de glyphs adicionais ---" << std::endl;
        
        std::vector<uint32_t> additionalChars = {
            0x00E7, // ç
            0x00E1, // á
            0x00E9, // é
            0x00ED, // í
            0x00F3, // ó
            0x00FA, // ú
            0x00E0, // à
            0x00E8, // è
            0x00EC, // ì
            0x00F2, // ò
            0x00F9, // ù
            0x00C7, // Ç
            0x00C1, // Á
            0x00C9, // É
            0x00CD, // Í
            0x00D3, // Ó
            0x00DA, // Ú
            0x00C0, // À
            0x00C8, // È
            0x00CC, // Ì
            0x00D2, // Ò
            0x00D9  // Ù
        };
        
        startTime = std::chrono::high_resolution_clock::now();
        
        for (uint32_t cp : additionalChars) {
            const auto* glyph = font->GetGlyph(cp);
            if (glyph && glyph->isValid) {
                std::cout << "  ✓ Glyph " << std::hex << cp << std::dec << " carregado" << std::endl;
            } else {
                std::cout << "  ✗ Glyph " << std::hex << cp << std::dec << " falhou" << std::endl;
            }
        }
        
        endTime = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "  - Carregamento de glyphs adicionais: " << duration.count() << "ms" << std::endl;
        
        // Fazer flush final
        if (fontManager.HasPendingUploads()) {
            startTime = std::chrono::high_resolution_clock::now();
            fontManager.FlushAllPendingUploads();
            endTime = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "  - Flush final: " << duration.count() << "ms" << std::endl;
        }
        
        // Mostrar estatísticas
        auto stats = fontManager.GetStats();
        std::cout << "\n--- Estatísticas do Sistema ---" << std::endl;
        std::cout << "  - Fontes carregadas: " << stats.totalFonts << std::endl;
        std::cout << "  - Glyphs totais: " << stats.totalGlyphs << std::endl;
        std::cout << "  - Atlases criados: " << stats.totalAtlases << std::endl;
        std::cout << "  - Uso de memória: " << (stats.memoryUsageBytes / 1024) << " KB" << std::endl;
        std::cout << "  - Taxa de acerto do cache: " << (stats.cacheHitRate * 100.0f) << "%" << std::endl;
        
    } else {
        std::cout << "✗ Falha ao carregar fonte" << std::endl;
    }
    
    std::cout << "\n=== Teste Concluído ===" << std::endl;
}

int main() {
    try {
        TestFontBatching();
    } catch (const std::exception& e) {
        std::cerr << "Erro durante o teste: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 