// Teste simples de carregamento de fontes
// Compile com: g++ -std=c++17 -I./src test_font_loading.cpp -o test_font_loading

#include "Drift/Core/Log.h"
#include "Drift/UI/FontSystem/Font.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include <iostream>

int main() {
    std::cout << "=== Teste de Carregamento de Fontes ===" << std::endl;
    
    // Configurar logging
    Drift::Core::SetLogLevel(Drift::Core::LogLevel::Debug);
    
    try {
        std::cout << "1. Testando criação de Font sem device..." << std::endl;
        auto font = std::make_shared<Drift::UI::Font>("test", 16.0f, Drift::UI::FontQuality::High);
        std::cout << "   ✓ Font criada com sucesso" << std::endl;
        
        std::cout << "2. Testando carregamento de arquivo..." << std::endl;
        std::string fontPath = "../../../fonts/Arial-Regular.ttf";
        std::ifstream testFile(fontPath);
        if (testFile.good()) {
            std::cout << "   ✓ Arquivo de fonte encontrado: " << fontPath << std::endl;
            testFile.close();
        } else {
            std::cout << "   ✗ Arquivo de fonte não encontrado: " << fontPath << std::endl;
            return -1;
        }
        
        std::cout << "3. Testando carregamento sem device..." << std::endl;
        bool loaded = font->LoadFromFile(fontPath, nullptr);
        if (loaded) {
            std::cout << "   ✓ Fonte carregada sem device" << std::endl;
        } else {
            std::cout << "   ✗ Falha ao carregar fonte sem device" << std::endl;
        }
        
        std::cout << "4. Testando FontManager..." << std::endl;
        auto& fontManager = Drift::UI::FontManager::GetInstance();
        std::cout << "   ✓ FontManager obtido" << std::endl;
        
        std::cout << "=== Teste concluído ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "ERRO: " << e.what() << std::endl;
        return -1;
    }
} 