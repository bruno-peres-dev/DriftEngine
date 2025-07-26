#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/RHI/UIBatcher.h"
#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/Core/Log.h"
#include <iostream>
#include <chrono>

using namespace Drift::UI;
using namespace Drift::RHI;

class UIDemo {
public:
    UIDemo() {
        InitializeFontSystem();
        InitializeUIBatcher();
    }
    
    ~UIDemo() {
        Cleanup();
    }
    
    void Run() {
        Core::Log("[UIDemo] Iniciando demonstração do sistema UI otimizado...");
        
        // Demonstração de carregamento de fontes
        DemoFontLoading();
        
        // Demonstração de renderização de texto
        DemoTextRendering();
        
        // Demonstração de batching otimizado
        DemoUIBatching();
        
        // Demonstração de cache de geometria
        DemoGeometryCache();
        
        // Demonstração de estatísticas
        DemoStatistics();
        
        Core::Log("[UIDemo] Demonstração concluída com sucesso!");
    }

private:
    void InitializeFontSystem() {
        Core::Log("[UIDemo] Inicializando FontSystem...");
        
        auto& fontManager = FontManager::GetInstance();
        
        // Configurar cache otimizado
        FontCacheConfig config;
        config.maxFonts = 32;
        config.maxGlyphsPerFont = 2048;
        config.maxAtlasSize = 2048;
        config.enablePreloading = true;
        config.enableLazyLoading = true;
        config.memoryBudgetMB = 128.0f;
        
        fontManager.SetCacheConfig(config);
        
        // Configurar qualidade padrão
        fontManager.SetDefaultQuality(FontQuality::High);
        fontManager.SetDefaultSize(16.0f);
        fontManager.SetDefaultFontName("Arial");
        
        Core::Log("[UIDemo] FontSystem inicializado");
    }
    
    void InitializeUIBatcher() {
        Core::Log("[UIDemo] Inicializando UIBatcher...");
        
        // Em uma implementação real, seria criado com ring buffer e contexto
        // Por enquanto, apenas demonstração da interface
        
        Core::Log("[UIDemo] UIBatcher inicializado");
    }
    
    void DemoFontLoading() {
        Core::Log("[UIDemo] === Demonstração de Carregamento de Fontes ===");
        
        auto& fontManager = FontManager::GetInstance();
        
        // Carregar fonte com diferentes qualidades
        auto fontLow = fontManager.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::Low);
        auto fontMedium = fontManager.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::Medium);
        auto fontHigh = fontManager.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::High);
        auto fontUltra = fontManager.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::Ultra);
        
        if (fontHigh) {
            Core::Log("[UIDemo] Fonte carregada: " + fontHigh->GetName());
            Core::Log("[UIDemo] Tamanho: " + std::to_string(fontHigh->GetSize()));
            Core::Log("[UIDemo] Qualidade: " + std::to_string(static_cast<int>(fontHigh->GetQuality())));
            
            // Demonstrar métricas
            const auto& metrics = fontHigh->GetMetrics();
            Core::Log("[UIDemo] Métricas da fonte:");
            Core::Log("[UIDemo]   - Ascender: " + std::to_string(metrics.ascender));
            Core::Log("[UIDemo]   - Descender: " + std::to_string(metrics.descender));
            Core::Log("[UIDemo]   - Line Height: " + std::to_string(metrics.lineHeight));
            Core::Log("[UIDemo]   - X Height: " + std::to_string(metrics.xHeight));
        }
        
        // Pré-carregar caracteres comuns
        std::vector<uint32_t> commonChars = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            ' ', '.', ',', '!', '?', ':', ';', '-', '_', '(', ')', '[', ']'
        };
        
        fontManager.PreloadCharacters("Arial", commonChars, 16.0f, FontQuality::High);
        Core::Log("[UIDemo] Caracteres pré-carregados: " + std::to_string(commonChars.size()));
    }
    
    void DemoTextRendering() {
        Core::Log("[UIDemo] === Demonstração de Renderização de Texto ===");
        
        auto& fontManager = FontManager::GetInstance();
        auto font = fontManager.GetFont("Arial", 16.0f, FontQuality::High);
        
        if (!font) {
            Core::Log("[UIDemo] ERRO: Fonte não encontrada!");
            return;
        }
        
        // Demonstrar diferentes configurações de renderização
        TextRenderSettings settings;
        
        // Configuração padrão
        settings.quality = FontQuality::High;
        settings.enableSubpixel = true;
        settings.enableLigatures = true;
        settings.enableKerning = true;
        settings.enableHinting = true;
        settings.gamma = 2.2f;
        settings.contrast = 0.1f;
        settings.smoothing = 0.1f;
        
        // Medir texto
        std::string sampleText = "Hello, World! This is a sample text for demonstration.";
        glm::vec2 textSize = font->MeasureText(sampleText);
        Core::Log("[UIDemo] Tamanho do texto: " + std::to_string(textSize.x) + " x " + std::to_string(textSize.y));
        
        // Word wrapping
        std::vector<std::string> wrappedLines = TextUtils::WordWrap(sampleText, 200.0f, "Arial", 16.0f);
        Core::Log("[UIDemo] Texto quebrado em " + std::to_string(wrappedLines.size()) + " linhas:");
        for (const auto& line : wrappedLines) {
            Core::Log("[UIDemo]   - " + line);
        }
        
        // Truncar texto
        std::string truncated = TextUtils::TruncateText(sampleText, 150.0f, "Arial", 16.0f);
        Core::Log("[UIDemo] Texto truncado: " + truncated);
        
        // Converter para codepoints
        std::vector<uint32_t> codepoints = TextUtils::StringToCodepoints(sampleText);
        Core::Log("[UIDemo] Codepoints: " + std::to_string(codepoints.size()));
        
        // Configuração com outline
        TextRenderSettings outlineSettings = settings;
        outlineSettings.outlineWidth = 1.0f;
        outlineSettings.outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        
        Core::Log("[UIDemo] Configurações de renderização demonstradas");
    }
    
    void DemoUIBatching() {
        Core::Log("[UIDemo] === Demonstração de UIBatching Otimizado ===");
        
        // Em uma implementação real, seria usado um UIBatcher real
        // Por enquanto, demonstração da interface
        
        Core::Log("[UIDemo] Configurações de batch:");
        Core::Log("[UIDemo]   - Max Vertices: 65536");
        Core::Log("[UIDemo]   - Max Indices: 131072");
        Core::Log("[UIDemo]   - Max Textures: 8");
        Core::Log("[UIDemo]   - Enable Scissor: true");
        Core::Log("[UIDemo]   - Enable Blending: true");
        
        // Simular ciclo de renderização
        Core::Log("[UIDemo] Simulando ciclo de renderização...");
        
        // Begin
        Core::Log("[UIDemo] uiBatcher->Begin()");
        
        // Adicionar primitivas
        Core::Log("[UIDemo] uiBatcher->AddRect(10, 10, 100, 50, 0xFFFFFFFF)");
        Core::Log("[UIDemo] uiBatcher->AddRect(120, 10, 100, 50, 0xFF0000FF)");
        Core::Log("[UIDemo] uiBatcher->AddRect(230, 10, 100, 50, 0xFF00FF00)");
        
        // Adicionar texto
        Core::Log("[UIDemo] uiBatcher->AddText(10, 70, \"Hello World\", 0xFFFFFFFF)");
        Core::Log("[UIDemo] uiBatcher->AddText(10, 90, \"Optimized UI System\", 0xFFFF00FF)");
        
        // Sistema de clipping
        Core::Log("[UIDemo] uiBatcher->PushScissorRect(0, 0, 300, 200)");
        Core::Log("[UIDemo] uiBatcher->AddRect(300, 10, 100, 50, 0xFFFFFF00) // Clipped");
        Core::Log("[UIDemo] uiBatcher->PopScissorRect()");
        
        // End
        Core::Log("[UIDemo] uiBatcher->End()");
        
        Core::Log("[UIDemo] Ciclo de renderização simulado");
    }
    
    void DemoGeometryCache() {
        Core::Log("[UIDemo] === Demonstração de Cache de Geometria ===");
        
        // Simular criação de cache de geometria
        Core::Log("[UIDemo] Criando cache de geometria...");
        
        // Em uma implementação real:
        // uint32_t cacheId = uiBatcher->CreateGeometryCache();
        
        // Criar geometria de exemplo (círculo)
        std::vector<UIVertex> circleVertices;
        std::vector<uint32_t> circleIndices;
        
        const int segments = 32;
        const float radius = 50.0f;
        
        // Centro
        circleVertices.emplace_back(0.0f, 0.0f, 0.5f, 0.5f, 0xFFFFFFFF, 0);
        
        // Vértices do círculo
        for (int i = 0; i <= segments; ++i) {
            float angle = (2.0f * 3.14159f * i) / segments;
            float x = radius * cos(angle);
            float y = radius * sin(angle);
            float u = 0.5f + 0.5f * cos(angle);
            float v = 0.5f + 0.5f * sin(angle);
            
            circleVertices.emplace_back(x, y, u, v, 0xFFFFFFFF, 0);
        }
        
        // Índices
        for (int i = 1; i < segments; ++i) {
            circleIndices.push_back(0);           // Centro
            circleIndices.push_back(i);           // Vértice atual
            circleIndices.push_back(i + 1);       // Próximo vértice
        }
        // Fechar o círculo
        circleIndices.push_back(0);
        circleIndices.push_back(segments);
        circleIndices.push_back(1);
        
        Core::Log("[UIDemo] Geometria criada:");
        Core::Log("[UIDemo]   - Vértices: " + std::to_string(circleVertices.size()));
        Core::Log("[UIDemo]   - Índices: " + std::to_string(circleIndices.size()));
        Core::Log("[UIDemo]   - Segments: " + std::to_string(segments));
        
        // Em uma implementação real:
        // uiBatcher->UpdateGeometryCache(cacheId, circleVertices, circleIndices);
        // uiBatcher->RenderGeometryCache(cacheId, 100, 100, 0xFFFFFFFF);
        
        Core::Log("[UIDemo] Cache de geometria demonstrado");
    }
    
    void DemoStatistics() {
        Core::Log("[UIDemo] === Demonstração de Estatísticas ===");
        
        // Estatísticas do FontManager
        auto& fontManager = FontManager::GetInstance();
        auto fontStats = fontManager.GetStats();
        
        Core::Log("[UIDemo] Estatísticas do FontManager:");
        Core::Log("[UIDemo]   - Total Fonts: " + std::to_string(fontStats.totalFonts));
        Core::Log("[UIDemo]   - Total Glyphs: " + std::to_string(fontStats.totalGlyphs));
        Core::Log("[UIDemo]   - Total Atlases: " + std::to_string(fontStats.totalAtlases));
        Core::Log("[UIDemo]   - Memory Usage: " + std::to_string(fontStats.memoryUsageBytes) + " bytes");
        Core::Log("[UIDemo]   - Cache Hits: " + std::to_string(fontStats.cacheHits));
        Core::Log("[UIDemo]   - Cache Misses: " + std::to_string(fontStats.cacheMisses));
        Core::Log("[UIDemo]   - Cache Hit Rate: " + std::to_string(fontStats.cacheHitRate * 100.0f) + "%");
        
        // Estatísticas do UIBatcher (simuladas)
        UIBatchStats uiStats;
        uiStats.drawCalls = 5;
        uiStats.verticesRendered = 24;
        uiStats.indicesRendered = 36;
        uiStats.batchesCreated = 2;
        uiStats.textureSwitches = 1;
        
        Core::Log("[UIDemo] Estatísticas do UIBatcher:");
        Core::Log("[UIDemo]   - Draw Calls: " + std::to_string(uiStats.drawCalls));
        Core::Log("[UIDemo]   - Vertices Rendered: " + std::to_string(uiStats.verticesRendered));
        Core::Log("[UIDemo]   - Indices Rendered: " + std::to_string(uiStats.indicesRendered));
        Core::Log("[UIDemo]   - Batches Created: " + std::to_string(uiStats.batchesCreated));
        Core::Log("[UIDemo]   - Texture Switches: " + std::to_string(uiStats.textureSwitches));
        
        Core::Log("[UIDemo] Estatísticas demonstradas");
    }
    
    void Cleanup() {
        Core::Log("[UIDemo] Limpando recursos...");
        
        auto& fontManager = FontManager::GetInstance();
        fontManager.UnloadAllFonts();
        
        Core::Log("[UIDemo] Recursos limpos");
    }
};

int main() {
    Core::Log("[UIDemo] Iniciando demonstração do sistema UI otimizado...");
    
    try {
        UIDemo demo;
        demo.Run();
    } catch (const std::exception& e) {
        Core::Log("[UIDemo] ERRO: " + std::string(e.what()));
        return -1;
    } catch (...) {
        Core::Log("[UIDemo] ERRO desconhecido");
        return -1;
    }
    
    Core::Log("[UIDemo] Demonstração concluída com sucesso!");
    return 0;
} 