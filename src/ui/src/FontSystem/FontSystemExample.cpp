#include "Drift/UI/FontSystem/FontSystem.h"
#include "Drift/Core/Assets/AssetsSystem.h"
#include "Drift/Core/Log.h"
#include <iostream>

using namespace Drift::UI;
using namespace Drift::Core;

/**
 * @brief Exemplo de uso do Sistema de Fontes Profissional
 * 
 * Este exemplo demonstra como usar o novo sistema de fontes
 * integrado ao sistema de assets do Drift Engine.
 */
class FontSystemExample {
public:
    /**
     * @brief Executa o exemplo completo
     */
    void RunExample() {
        Core::Log("[FontSystemExample] Iniciando exemplo do sistema de fontes profissional");
        
        // 1. Inicialização do sistema
        InitializeSystems();
        
        // 2. Configuração do sistema de fontes
        ConfigureFontSystem();
        
        // 3. Carregamento de fontes
        LoadFonts();
        
        // 4. Exemplos de renderização
        RenderTextExamples();
        
        // 5. Exemplos de layout
        LayoutExamples();
        
        // 6. Exemplos de efeitos
        EffectsExamples();
        
        // 7. Estatísticas e performance
        PerformanceExamples();
        
        // 8. Limpeza
        Cleanup();
        
        Core::Log("[FontSystemExample] Exemplo concluído com sucesso!");
    }

private:
    std::shared_ptr<Font> m_ArialFont;
    std::shared_ptr<Font> m_MinecraftFont;
    std::shared_ptr<TextRenderer> m_TextRenderer;
    
    /**
     * @brief Inicializa os sistemas necessários
     */
    void InitializeSystems() {
        Core::Log("[FontSystemExample] Inicializando sistemas...");
        
        // Inicializa o sistema de assets
        auto& assetsSystem = Assets::AssetsSystem::GetInstance();
        assetsSystem.Initialize();
        
        // Registra o loader de fontes
        auto fontLoader = std::make_unique<FontLoader>(nullptr); // Device será configurado depois
        assetsSystem.RegisterLoader<Font>(std::move(fontLoader));
        
        // Inicializa o sistema de fontes
        FontSystemConfig config;
        config.enableAsyncLoading = true;
        config.enablePreloading = true;
        config.defaultQuality = FontQuality::High;
        config.fallbackFonts = {"fonts/Arial-Regular.ttf", "fonts/Minecraft.ttf"};
        
        InitializeFontSystem(config);
        
        Core::Log("[FontSystemExample] Sistemas inicializados");
    }
    
    /**
     * @brief Configura o sistema de fontes
     */
    void ConfigureFontSystem() {
        Core::Log("[FontSystemExample] Configurando sistema de fontes...");
        
        auto& fontManager = FontManager::GetInstance();
        
        // Configura fontes de fallback
        fontManager.RegisterFallbackFont("fonts/Arial-Regular.ttf", "Arial");
        fontManager.RegisterFallbackFont("fonts/Minecraft.ttf", "Minecraft");
        
        // Configura fontes do sistema
        std::vector<std::string> systemFonts = {
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "C:/Windows/Fonts/consola.ttf"
        };
        fontManager.SetSystemFallbackFonts(systemFonts);
        
        Core::Log("[FontSystemExample] Sistema de fontes configurado");
    }
    
    /**
     * @brief Carrega as fontes necessárias
     */
    void LoadFonts() {
        Core::Log("[FontSystemExample] Carregando fontes...");
        
        auto& fontManager = FontManager::GetInstance();
        
        // Carrega fontes usando o sistema de assets
        FontLoadConfig arialConfig;
        arialConfig.size = 16.0f;
        arialConfig.quality = FontQuality::High;
        arialConfig.enableKerning = true;
        arialConfig.enableLigatures = true;
        
        m_ArialFont = fontManager.LoadFontAsset("fonts/Arial-Regular.ttf", arialConfig);
        
        FontLoadConfig minecraftConfig;
        minecraftConfig.size = 12.0f;
        minecraftConfig.quality = FontQuality::Medium;
        minecraftConfig.enableKerning = false; // Fontes pixeladas geralmente não têm kerning
        
        m_MinecraftFont = fontManager.LoadFontAsset("fonts/Minecraft.ttf", minecraftConfig);
        
        // Pré-carrega tamanhos comuns
        std::vector<float> commonSizes = {8.0f, 12.0f, 16.0f, 24.0f, 32.0f, 48.0f};
        fontManager.PreloadCommonSizes("fonts/Arial-Regular.ttf", commonSizes);
        
        // Pré-carrega conjunto de caracteres
        std::vector<uint32_t> latinChars;
        for (uint32_t i = 32; i <= 126; ++i) { // ASCII básico
            latinChars.push_back(i);
        }
        // Adiciona caracteres acentuados comuns
        std::vector<uint32_t> accentedChars = {0x00E1, 0x00E0, 0x00E2, 0x00E3, 0x00E7, 0x00E9, 0x00E8, 0x00EA, 0x00ED, 0x00F3, 0x00F2, 0x00F4, 0x00F5, 0x00FA, 0x00F9, 0x00FC, 0x00E7};
        latinChars.insert(latinChars.end(), accentedChars.begin(), accentedChars.end());
        
        fontManager.PreloadCharSet("fonts/Arial-Regular.ttf", latinChars);
        
        Core::Log("[FontSystemExample] Fontes carregadas");
    }
    
    /**
     * @brief Exemplos de renderização de texto
     */
    void RenderTextExamples() {
        Core::Log("[FontSystemExample] Exemplos de renderização...");
        
        // Renderização básica
        m_TextRenderer->RenderText("Olá, Mundo!", glm::vec2(100, 100), m_ArialFont, glm::vec4(1.0f));
        
        // Renderização com diferentes cores
        m_TextRenderer->RenderText("Texto colorido", glm::vec2(100, 150), m_ArialFont, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        m_TextRenderer->RenderText("Outra cor", glm::vec2(100, 180), m_ArialFont, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        
        // Renderização com fonte pixelada
        m_TextRenderer->RenderText("MINECRAFT FONT", glm::vec2(100, 220), m_MinecraftFont, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        
        // Renderização com caracteres especiais
        m_TextRenderer->RenderText("Caracteres especiais: áéíóú çãõ", glm::vec2(100, 260), m_ArialFont, glm::vec4(1.0f));
        
        // Renderização com números
        m_TextRenderer->RenderText("Números: 1234567890", glm::vec2(100, 300), m_ArialFont, glm::vec4(1.0f));
        
        // Renderização com símbolos
        m_TextRenderer->RenderText("Símbolos: @#$%&*()_+-=[]{}|;':\",./<>?", glm::vec2(100, 340), m_ArialFont, glm::vec4(1.0f));
        
        Core::Log("[FontSystemExample] Exemplos de renderização concluídos");
    }
    
    /**
     * @brief Exemplos de layout de texto
     */
    void LayoutExamples() {
        Core::Log("[FontSystemExample] Exemplos de layout...");
        
        // Layout com quebra de linha
        TextLayoutConfig layoutConfig;
        layoutConfig.maxWidth = 300.0f;
        layoutConfig.enableWordWrap = true;
        layoutConfig.horizontalAlign = TextAlign::Justify;
        layoutConfig.lineSpacing = 1.2f;
        
        std::string longText = "Este é um texto longo que será quebrado em múltiplas linhas para demonstrar o sistema de layout de texto do Drift Engine. O sistema suporta quebra de linha automática, alinhamento e espaçamento configurável.";
        
        auto layout = m_TextRenderer->CalculateLayout(longText, m_ArialFont, layoutConfig);
        m_TextRenderer->RenderText(layout, glm::vec2(100, 400), m_ArialFont);
        
        // Layout centralizado
        TextLayoutConfig centerConfig;
        centerConfig.horizontalAlign = TextAlign::Center;
        centerConfig.maxWidth = 400.0f;
        
        auto centerLayout = m_TextRenderer->CalculateLayout("Texto Centralizado", m_ArialFont, centerConfig);
        m_TextRenderer->RenderText(centerLayout, glm::vec2(200, 500), m_ArialFont, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        
        // Layout alinhado à direita
        TextLayoutConfig rightConfig;
        rightConfig.horizontalAlign = TextAlign::Right;
        rightConfig.maxWidth = 400.0f;
        
        auto rightLayout = m_TextRenderer->CalculateLayout("Texto Alinhado à Direita", m_ArialFont, rightConfig);
        m_TextRenderer->RenderText(rightLayout, glm::vec2(200, 540), m_ArialFont, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        
        Core::Log("[FontSystemExample] Exemplos de layout concluídos");
    }
    
    /**
     * @brief Exemplos de efeitos de texto
     */
    void EffectsExamples() {
        Core::Log("[FontSystemExample] Exemplos de efeitos...");
        
        // Texto com sombra
        m_TextRenderer->RenderTextWithShadow("Texto com Sombra", glm::vec2(100, 600), m_ArialFont, 
                                            glm::vec4(1.0f), glm::vec2(2.0f, 2.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
        
        // Texto com outline
        m_TextRenderer->RenderTextWithOutline("Texto com Outline", glm::vec2(100, 640), m_ArialFont,
                                             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 2.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        
        // Texto com gradiente
        m_TextRenderer->RenderTextWithGradient("Texto com Gradiente", glm::vec2(100, 680), m_ArialFont,
                                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                                              glm::vec2(0.0f, 1.0f));
        
        // Texto com múltiplos efeitos usando configuração avançada
        TextRenderConfig effectConfig;
        effectConfig.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        effectConfig.scale = glm::vec2(1.5f);
        
        TextEffectConfig shadowEffect;
        shadowEffect.type = TextEffect::Shadow;
        shadowEffect.shadowOffset = glm::vec2(3.0f, 3.0f);
        shadowEffect.shadowColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.7f);
        effectConfig.effects.push_back(shadowEffect);
        
        TextEffectConfig glowEffect;
        glowEffect.type = TextEffect::Glow;
        glowEffect.glowRadius = 5.0f;
        glowEffect.glowColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.5f);
        effectConfig.effects.push_back(glowEffect);
        
        m_TextRenderer->RenderText("Texto com Múltiplos Efeitos", glm::vec2(100, 720), m_ArialFont, effectConfig);
        
        Core::Log("[FontSystemExample] Exemplos de efeitos concluídos");
    }
    
    /**
     * @brief Exemplos de performance e otimização
     */
    void PerformanceExamples() {
        Core::Log("[FontSystemExample] Exemplos de performance...");
        
        // Renderização em lote
        m_TextRenderer->BeginTextRendering();
        
        for (int i = 0; i < 100; ++i) {
            std::string text = "Texto " + std::to_string(i);
            glm::vec2 pos(100 + (i % 10) * 80, 800 + (i / 10) * 30);
            glm::vec4 color(1.0f, 0.5f + (i % 5) * 0.1f, 0.5f, 1.0f);
            
            m_TextRenderer->RenderText(text, pos, m_ArialFont, color);
        }
        
        m_TextRenderer->EndTextRendering();
        
        // Medidas de texto em lote
        std::vector<std::string> texts = {
            "Texto curto",
            "Texto médio com algumas palavras",
            "Texto muito longo que vai ocupar bastante espaço na tela e ser usado para testar o sistema de medidas",
            "1234567890",
            "!@#$%^&*()",
            "áéíóú çãõ"
        };
        
        for (const auto& text : texts) {
            auto size = m_TextRenderer->MeasureText(text, m_ArialFont);
            Core::Log("[FontSystemExample] Medida de '" + text + "': " + 
                     std::to_string(size.x) + "x" + std::to_string(size.y));
        }
        
        // Estatísticas
        auto fontStats = FontManager::GetInstance().GetStats();
        auto textStats = m_TextRenderer->GetStats();
        
        Core::Log("[FontSystemExample] Estatísticas do sistema:");
        Core::Log("  - Fontes carregadas: " + std::to_string(fontStats.loadedFonts));
        Core::Log("  - Cache hits: " + std::to_string(fontStats.cacheHits));
        Core::Log("  - Cache misses: " + std::to_string(fontStats.cacheMisses));
        Core::Log("  - Uso de memória: " + std::to_string(fontStats.totalMemoryUsage / 1024) + " KB");
        Core::Log("  - Caracteres renderizados: " + std::to_string(textStats.charactersRendered));
        Core::Log("  - Draw calls: " + std::to_string(textStats.drawCalls));
        
        Core::Log("[FontSystemExample] Exemplos de performance concluídos");
    }
    
    /**
     * @brief Limpeza dos recursos
     */
    void Cleanup() {
        Core::Log("[FontSystemExample] Limpando recursos...");
        
        // Limpa caches
        FontManager::GetInstance().ClearCache();
        m_TextRenderer->ClearTextCache();
        
        // Finaliza sistemas
        ShutdownFontSystem();
        
        Core::Log("[FontSystemExample] Recursos limpos");
    }
};

/**
 * @brief Função principal do exemplo
 */
void RunFontSystemExample() {
    FontSystemExample example;
    example.RunExample();
}

// Exemplo de uso das macros
void MacroExamples() {
    // Carregamento de fontes
    auto font = DRIFT_LOAD_FONT("fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);
    
    // Renderização de texto
    DRIFT_RENDER_TEXT("Texto via macro", glm::vec2(100, 100), "Arial", 16.0f, glm::vec4(1.0f));
    
    // Medidas de texto
    auto size = DRIFT_MEASURE_TEXT("Texto para medir", "Arial", 16.0f);
    
    // Carregamento via assets
    auto fontAsset = DRIFT_LOAD_FONT_ASSET("fonts/Arial-Regular.ttf", {16.0f, FontQuality::High});
    
    // Pré-carregamento
    DRIFT_PRELOAD_FONT("fonts/Minecraft.ttf", {12.0f, FontQuality::Medium});
} 