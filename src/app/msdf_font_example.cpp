#include "Drift/UI/FontSystem/MSDFFont.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/UIBatcher.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Color.h"
#include <iostream>
#include <chrono>

using namespace Drift::UI::FontSystem;

// ============================================================================
// EXEMPLO DE USO DO SISTEMA MSDF
// ============================================================================

class MSDFFontExample {
private:
    std::unique_ptr<MSDFFontSystem> fontSystem;
    Drift::RHI::IDevice* device;
    Drift::RHI::IUIBatcher* uiBatcher;
    
    // Configurações de exemplo
    TextRenderSettings defaultSettings;
    TextRenderSettings titleSettings;
    TextRenderSettings subtitleSettings;
    TextRenderSettings bodySettings;
    
    // Textos de exemplo
    std::vector<std::string> sampleTexts;
    std::vector<std::wstring> sampleWTexts;
    
    // Estatísticas
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    float deltaTime = 0.0f;
    
public:
    MSDFFontExample() : device(nullptr), uiBatcher(nullptr) {
        InitializeSettings();
        InitializeSampleTexts();
    }
    
    ~MSDFFontExample() = default;
    
    bool Initialize(Drift::RHI::IDevice* dev, Drift::RHI::IUIBatcher* batcher) {
        device = dev;
        uiBatcher = batcher;
        
        if (!device || !uiBatcher) {
            DRIFT_LOG_ERROR("MSDFFontExample: Device ou UIBatcher inválidos");
            return false;
        }
        
        // Inicializar sistema de fontes
        fontSystem = std::make_unique<MSDFFontSystem>();
        if (!fontSystem->Initialize(device)) {
            DRIFT_LOG_ERROR("MSDFFontExample: Falha ao inicializar sistema de fontes");
            return false;
        }
        
        // Carregar fontes
        LoadFonts();
        
        // Configurar fontes padrão
        fontSystem->SetDefaultFont("Arial");
        fontSystem->SetFallbackFont("Consolas");
        
        // Pré-carregar glyphs comuns
        PreloadCommonGlyphs();
        
        lastFrameTime = std::chrono::high_resolution_clock::now();
        
        DRIFT_LOG_INFO("MSDFFontExample inicializado com sucesso");
        return true;
    }
    
    void Shutdown() {
        fontSystem.reset();
        device = nullptr;
        uiBatcher = nullptr;
    }
    
    void Update() {
        // Calcular delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        lastFrameTime = currentTime;
    }
    
    void Render() {
        if (!fontSystem || !uiBatcher) return;
        
        // Configurar UIBatcher
        uiBatcher->Begin();
        uiBatcher->SetScreenSize(1920.0f, 1080.0f);
        
        // Renderizar exemplos
        RenderBasicText();
        RenderStyledText();
        RenderMultilineText();
        RenderAnimatedText();
        RenderTextEffects();
        RenderTextMetrics();
        RenderDebugInfo();
        
        // Finalizar renderização
        uiBatcher->End();
    }
    
private:
    void InitializeSettings() {
        // Configurações padrão
        defaultSettings.fontSize = 16.0f;
        defaultSettings.lineHeight = 1.2f;
        defaultSettings.color = 0xFFFFFFFF;
        defaultSettings.enableKerning = true;
        defaultSettings.enableSubpixelRendering = true;
        
        // Configurações para títulos
        titleSettings = defaultSettings;
        titleSettings.fontSize = 32.0f;
        titleSettings.lineHeight = 1.1f;
        titleSettings.color = 0xFFFFD700; // Dourado
        titleSettings.outlineWidth = 2.0f;
        titleSettings.outlineColor = 0xFF000000;
        titleSettings.shadowOffsetX = 2.0f;
        titleSettings.shadowOffsetY = 2.0f;
        titleSettings.shadowBlur = 3.0f;
        titleSettings.shadowColor = 0x80000000;
        
        // Configurações para subtítulos
        subtitleSettings = defaultSettings;
        subtitleSettings.fontSize = 24.0f;
        subtitleSettings.lineHeight = 1.15f;
        subtitleSettings.color = 0xFF87CEEB; // Azul claro
        titleSettings.shadowOffsetX = 1.0f;
        titleSettings.shadowOffsetY = 1.0f;
        titleSettings.shadowBlur = 2.0f;
        
        // Configurações para corpo de texto
        bodySettings = defaultSettings;
        bodySettings.fontSize = 14.0f;
        bodySettings.lineHeight = 1.4f;
        bodySettings.color = 0xFFE0E0E0; // Cinza claro
        bodySettings.wordSpacing = 2.0f;
    }
    
    void InitializeSampleTexts() {
        // Textos de exemplo em português
        sampleTexts = {
            "Sistema de Fontes MSDF Avançado",
            "Renderização de Alta Qualidade",
            "Suporte a Unicode e Emojis",
            "Efeitos Visuais Profissionais",
            "Otimização de Performance",
            "DriftEngine - Motor de Jogos"
        };
        
        // Textos Unicode de exemplo
        sampleWTexts = {
            L"Texto com Acentos: áéíóúâêîôûãõç",
            L"Emojis: 🎮🚀⚡🎯🎨",
            L"Caracteres Especiais: ©®™€¥£",
            L"Matemática: αβγδεθλμπσφω",
            L"Cirílico: абвгдеёжзийклмнопрстуфхцчшщъыьэюя",
            L"Japonês: こんにちは世界"
        };
    }
    
    void LoadFonts() {
        // Carregar fontes do sistema
        fontSystem->LoadFont("fonts/Arial-Regular.ttf", "Arial", 16.0f);
        fontSystem->LoadFont("fonts/Arial-Bold.ttf", "Arial-Bold", 16.0f);
        fontSystem->LoadFont("fonts/Consolas-Regular.ttf", "Consolas", 16.0f);
        
        // Carregar fontes de fallback
        fontSystem->LoadFont("fonts/NotoSans-Regular.ttf", "NotoSans", 16.0f);
        fontSystem->LoadFont("fonts/NotoEmoji-Regular.ttf", "NotoEmoji", 16.0f);
        
        DRIFT_LOG_INFO("Fontes carregadas: {}", fontSystem->GetStats().loadedFonts);
    }
    
    void PreloadCommonGlyphs() {
        // Pré-carregar caracteres comuns
        std::string commonChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?;:()[]{}\"'`~@#$%^&*+-=_|\\/<>";
        fontSystem->PreloadGlyphs(commonChars, "Arial");
        
        // Pré-carregar caracteres especiais
        std::wstring specialChars = L"áéíóúâêîôûãõçÁÉÍÓÚÂÊÎÔÛÃÕÇ";
        fontSystem->PreloadGlyphs(specialChars, "Arial");
        
        DRIFT_LOG_INFO("Glyphs pré-carregados: {}", fontSystem->GetStats().cachedGlyphs);
    }
    
    void RenderBasicText() {
        // Renderizar título principal
        TextLayoutInfo titleLayout;
        titleLayout.position = glm::vec2(50.0f, 50.0f);
        titleLayout.alignment = glm::vec2(0.0f, 0.0f); // Centralizado
        
        auto result = fontSystem->RenderText(sampleTexts[0], titleLayout.position, titleSettings, titleLayout);
        
        // Renderizar subtítulo
        TextLayoutInfo subtitleLayout;
        subtitleLayout.position = glm::vec2(50.0f, 100.0f);
        subtitleLayout.alignment = glm::vec2(0.0f, 0.0f);
        
        fontSystem->RenderText(sampleTexts[1], subtitleLayout.position, subtitleSettings, subtitleLayout);
    }
    
    void RenderStyledText() {
        float yPos = 150.0f;
        
        // Renderizar diferentes estilos de texto
        for (size_t i = 2; i < sampleTexts.size(); ++i) {
            TextRenderSettings styleSettings = bodySettings;
            
            // Variações de cor
            switch (i % 4) {
                case 0: styleSettings.color = 0xFFFF6B6B; break; // Vermelho
                case 1: styleSettings.color = 0xFF4ECDC4; break; // Verde-azulado
                case 2: styleSettings.color = 0xFFFFE66D; break; // Amarelo
                case 3: styleSettings.color = 0xFF95E1D3; break; // Verde claro
            }
            
            // Adicionar outline para alguns textos
            if (i % 3 == 0) {
                styleSettings.outlineWidth = 1.0f;
                styleSettings.outlineColor = 0xFF000000;
            }
            
            TextLayoutInfo layout;
            layout.position = glm::vec2(50.0f, yPos);
            
            fontSystem->RenderText(sampleTexts[i], layout.position, styleSettings, layout);
            yPos += 30.0f;
        }
    }
    
    void RenderMultilineText() {
        // Texto longo com quebra de linha
        std::string longText = "Este é um exemplo de texto longo que será renderizado com quebra de linha automática. "
                              "O sistema MSDF suporta quebra de palavras e alinhamento de texto de forma profissional. "
                              "A qualidade de renderização é mantida em qualquer resolução.";
        
        TextLayoutInfo layout;
        layout.position = glm::vec2(50.0f, 350.0f);
        layout.size = glm::vec2(600.0f, 200.0f);
        layout.maxWidth = 600.0f;
        layout.wordWrap = true;
        layout.alignment = glm::vec2(0.0f, 0.0f); // Centralizado
        
        TextRenderSettings settings = bodySettings;
        settings.fontSize = 16.0f;
        settings.lineHeight = 1.3f;
        
        fontSystem->RenderText(longText, layout.position, settings, layout);
    }
    
    void RenderAnimatedText() {
        // Texto com animação baseada no tempo
        static float animationTime = 0.0f;
        animationTime += deltaTime;
        
        // Efeito de cor pulsante
        float pulse = (sin(animationTime * 2.0f) + 1.0f) * 0.5f;
        uint32_t animatedColor = 0xFF0000FF | (uint32_t(pulse * 255) << 16) | (uint32_t(pulse * 255) << 8);
        
        TextRenderSettings animatedSettings = titleSettings;
        animatedSettings.color = animatedColor;
        animatedSettings.fontSize = 28.0f + sin(animationTime * 3.0f) * 4.0f; // Escala pulsante
        
        TextLayoutInfo layout;
        layout.position = glm::vec2(50.0f, 600.0f);
        layout.alignment = glm::vec2(0.0f, 0.0f);
        
        fontSystem->RenderText("Texto Animado", layout.position, animatedSettings, layout);
    }
    
    void RenderTextEffects() {
        float yPos = 700.0f;
        
        // Renderizar textos com diferentes efeitos
        for (size_t i = 0; i < sampleWTexts.size(); ++i) {
            TextRenderSettings effectSettings = bodySettings;
            effectSettings.fontSize = 18.0f;
            
            // Aplicar diferentes efeitos
            switch (i % 3) {
                case 0: // Sombra
                    effectSettings.shadowOffsetX = 3.0f;
                    effectSettings.shadowOffsetY = 3.0f;
                    effectSettings.shadowBlur = 4.0f;
                    effectSettings.shadowColor = 0x60000000;
                    break;
                    
                case 1: // Outline
                    effectSettings.outlineWidth = 2.0f;
                    effectSettings.outlineColor = 0xFF0000FF;
                    break;
                    
                case 2: // Combinação
                    effectSettings.outlineWidth = 1.5f;
                    effectSettings.outlineColor = 0xFF000000;
                    effectSettings.shadowOffsetX = 2.0f;
                    effectSettings.shadowOffsetY = 2.0f;
                    effectSettings.shadowBlur = 3.0f;
                    effectSettings.shadowColor = 0x40000000;
                    break;
            }
            
            TextLayoutInfo layout;
            layout.position = glm::vec2(50.0f, yPos);
            
            fontSystem->RenderText(sampleWTexts[i], layout.position, effectSettings, layout);
            yPos += 35.0f;
        }
    }
    
    void RenderTextMetrics() {
        // Demonstrar medição de texto
        std::string testText = "Texto para medição";
        glm::vec2 textSize = fontSystem->MeasureText(testText, bodySettings);
        
        // Renderizar texto com bounds visíveis
        TextLayoutInfo layout;
        layout.position = glm::vec2(800.0f, 50.0f);
        layout.size = textSize;
        
        // Renderizar bounds (retângulo)
        uiBatcher->AddRect(layout.position.x, layout.position.y, 
                          textSize.x, textSize.y, 0x40FF0000);
        
        // Renderizar texto
        fontSystem->RenderText(testText, layout.position, bodySettings, layout);
        
        // Informações de métricas
        std::string metricsText = "Largura: " + std::to_string(int(textSize.x)) + 
                                 "px, Altura: " + std::to_string(int(textSize.y)) + "px";
        
        TextLayoutInfo metricsLayout;
        metricsLayout.position = glm::vec2(800.0f, 100.0f);
        fontSystem->RenderText(metricsText, metricsLayout.position, bodySettings, metricsLayout);
    }
    
    void RenderDebugInfo() {
        // Mostrar estatísticas do sistema
        auto stats = fontSystem->GetStats();
        
        std::string debugInfo = "Fontes: " + std::to_string(stats.loadedFonts) +
                               " | Glyphs: " + std::to_string(stats.cachedGlyphs) +
                               " | Atlas: " + std::to_string(int(stats.atlasUsageRatio * 100)) + "%" +
                               " | FPS: " + std::to_string(int(1.0f / deltaTime));
        
        TextRenderSettings debugSettings = bodySettings;
        debugSettings.fontSize = 12.0f;
        debugSettings.color = 0xFF00FF00;
        
        TextLayoutInfo layout;
        layout.position = glm::vec2(10.0f, 1050.0f);
        
        fontSystem->RenderText(debugInfo, layout.position, debugSettings, layout);
    }
};

// ============================================================================
// FUNÇÃO PRINCIPAL DO EXEMPLO
// ============================================================================

int main() {
    DRIFT_LOG_INFO("Iniciando exemplo do sistema MSDF");
    
    // TODO: Inicializar device e UIBatcher
    // Drift::RHI::IDevice* device = CreateDevice();
    // Drift::RHI::IUIBatcher* batcher = CreateUIBatcher(device);
    
    MSDFFontExample example;
    
    // if (!example.Initialize(device, batcher)) {
    //     DRIFT_LOG_ERROR("Falha ao inicializar exemplo MSDF");
    //     return -1;
    // }
    
    // Loop principal
    // while (IsRunning()) {
    //     example.Update();
    //     example.Render();
    //     Present();
    // }
    
    example.Shutdown();
    
    DRIFT_LOG_INFO("Exemplo MSDF finalizado");
    return 0;
} 