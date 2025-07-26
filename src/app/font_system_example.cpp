#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

#include "Drift/UI/FontSystem/FontSystem.h"
#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/UI/FontSystem/MSDFGenerator.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

/**
 * @brief Exemplo completo de uso do sistema de fontes refatorado
 */
class FontSystemExample {
public:
    FontSystemExample() {
        LOG_INFO("=== Sistema de Fontes Refatorado - Exemplo de Uso ===");
    }
    
    ~FontSystemExample() {
        LOG_INFO("=== Exemplo Finalizado ===");
    }

    /**
     * @brief Executa o exemplo completo
     */
    void Run() {
        LOG_INFO("Iniciando exemplo do sistema de fontes...");
        
        // 1. Configuração básica
        SetupBasicConfiguration();
        
        // 2. Carregamento de fontes
        LoadFonts();
        
        // 3. Exemplo de renderização simples
        SimpleRenderingExample();
        
        // 4. Exemplo de renderização avançada
        AdvancedRenderingExample();
        
        // 5. Exemplo de MSDF
        MSDFExample();
        
        // 6. Exemplo de performance
        PerformanceExample();
        
        // 7. Exemplo de layout
        LayoutExample();
        
        // 8. Estatísticas finais
        PrintFinalStats();
    }

private:
    std::shared_ptr<Font> m_DefaultFont;
    std::shared_ptr<Font> m_TitleFont;
    std::shared_ptr<Font> m_CodeFont;
    std::unique_ptr<TextRenderer> m_TextRenderer;
    std::unique_ptr<TextLayoutEngine> m_LayoutEngine;
    
    /**
     * @brief Configuração básica do sistema
     */
    void SetupBasicConfiguration() {
        LOG_INFO("1. Configurando sistema básico...");
        
        // Obter instância do FontManager
        auto& fontManager = FontManager::GetInstance();
        
        // Configurar cache otimizado
        FontCacheConfig cacheConfig;
        cacheConfig.maxFonts = 32;
        cacheConfig.maxGlyphsPerFont = 2048;
        cacheConfig.maxAtlasSize = 2048;
        cacheConfig.enablePreloading = true;
        cacheConfig.enableLazyLoading = true;
        cacheConfig.memoryBudgetMB = 128.0f;
        fontManager.SetCacheConfig(cacheConfig);
        
        // Configurar qualidade padrão
        fontManager.SetDefaultQuality(FontQuality::High);
        fontManager.SetDefaultSize(16.0f);
        fontManager.SetDefaultFontName("Arial");
        
        // Criar renderizador de texto
        TextRenderConfig renderConfig;
        renderConfig.maxCommands = 512;
        renderConfig.maxBatches = 32;
        renderConfig.enableBatching = true;
        renderConfig.enableFrustumCulling = true;
        m_TextRenderer = std::make_unique<TextRenderer>(renderConfig);
        
        // Criar motor de layout
        m_LayoutEngine = std::make_unique<TextLayoutEngine>();
        
        LOG_INFO("   ✓ Sistema configurado com sucesso");
    }
    
    /**
     * @brief Carregamento de fontes
     */
    void LoadFonts() {
        LOG_INFO("2. Carregando fontes...");
        
        auto& fontManager = FontManager::GetInstance();
        
        // Carregar fonte padrão
        m_DefaultFont = fontManager.LoadFont("Arial", "fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);
        if (m_DefaultFont) {
            LOG_INFO("   ✓ Fonte padrão carregada: " + m_DefaultFont->GetName());
        } else {
            LOG_WARNING("   ⚠ Fonte padrão não encontrada, criando fonte embutida");
            m_DefaultFont = fontManager.CreateEmbeddedDefaultFont(16.0f, FontQuality::High);
        }
        
        // Carregar fonte de título
        m_TitleFont = fontManager.LoadFont("Title", "fonts/Arial-Bold.ttf", 32.0f, FontQuality::Ultra);
        if (m_TitleFont) {
            LOG_INFO("   ✓ Fonte de título carregada: " + m_TitleFont->GetName());
        }
        
        // Carregar fonte de código
        m_CodeFont = fontManager.LoadFont("Code", "fonts/Consolas.ttf", 14.0f, FontQuality::Medium);
        if (m_CodeFont) {
            LOG_INFO("   ✓ Fonte de código carregada: " + m_CodeFont->GetName());
        }
        
        // Pré-carregar caracteres essenciais
        std::vector<uint32_t> essentialChars = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            ' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_',
            '=', '+', '[', ']', '{', '}', '|', '\\', ';', ':', '"', '\'', ',',
            '.', '<', '>', '/', '?', '~', '`'
        };
        
        if (m_DefaultFont) {
            fontManager.PreloadCharacters("Arial", essentialChars, 16.0f, FontQuality::High);
        }
        if (m_TitleFont) {
            fontManager.PreloadCharacters("Title", essentialChars, 32.0f, FontQuality::Ultra);
        }
        if (m_CodeFont) {
            fontManager.PreloadCharacters("Code", essentialChars, 14.0f, FontQuality::Medium);
        }
        
        LOG_INFO("   ✓ Carregamento de fontes concluído");
    }
    
    /**
     * @brief Exemplo de renderização simples
     */
    void SimpleRenderingExample() {
        LOG_INFO("3. Exemplo de renderização simples...");
        
        if (!m_TextRenderer || !m_DefaultFont) {
            LOG_ERROR("   ✗ Renderizador ou fonte não disponível");
            return;
        }
        
        // Iniciar ciclo de renderização
        m_TextRenderer->BeginTextRendering();
        
        // Adicionar textos simples
        m_TextRenderer->AddText("Hello World!", glm::vec2(100, 100), "Arial", 16.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        m_TextRenderer->AddText("Sistema de Fontes Refatorado", glm::vec2(100, 130), "Arial", 16.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        m_TextRenderer->AddText("Com MSDF e Otimizações", glm::vec2(100, 160), "Arial", 16.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        
        // Medir texto
        glm::vec2 textSize = m_TextRenderer->MeasureText("Hello World!", "Arial", 16.0f);
        LOG_INFO("   ✓ Tamanho do texto 'Hello World!': " + std::to_string(textSize.x) + "x" + std::to_string(textSize.y));
        
        // Finalizar ciclo de renderização
        m_TextRenderer->EndTextRendering();
        
        LOG_INFO("   ✓ Renderização simples concluída");
    }
    
    /**
     * @brief Exemplo de renderização avançada
     */
    void AdvancedRenderingExample() {
        LOG_INFO("4. Exemplo de renderização avançada...");
        
        if (!m_TextRenderer || !m_TitleFont || !m_CodeFont) {
            LOG_ERROR("   ✗ Fontes não disponíveis");
            return;
        }
        
        // Configurações avançadas
        TextRenderSettings titleSettings;
        titleSettings.quality = FontQuality::Ultra;
        titleSettings.enableKerning = true;
        titleSettings.enableSubpixel = true;
        titleSettings.gamma = 2.2f;
        titleSettings.contrast = 0.1f;
        titleSettings.smoothing = 0.1f;
        titleSettings.outlineWidth = 2.0f;
        titleSettings.outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        
        TextRenderSettings codeSettings;
        codeSettings.quality = FontQuality::Medium;
        codeSettings.enableKerning = false;
        codeSettings.enableSubpixel = false;
        codeSettings.letterSpacing = 1.0f;
        
        // Iniciar ciclo de renderização
        m_TextRenderer->BeginTextRendering();
        
        // Título com configurações avançadas
        m_TextRenderer->AddText("TÍTULO PRINCIPAL", glm::vec2(200, 200), "Title", 32.0f, 
                               glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), titleSettings);
        
        // Código com configurações específicas
        std::string codeText = "void RenderText(const std::string& text) {\n"
                              "    m_TextRenderer->AddText(text, position);\n"
                              "}";
        
        m_TextRenderer->AddText(codeText, glm::vec2(200, 300), "Code", 14.0f, 
                               glm::vec4(0.8f, 0.8f, 0.8f, 1.0f), codeSettings);
        
        // Texto com transformação
        glm::mat4 transform = glm::rotate(glm::mat4(1.0f), 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
        m_TextRenderer->AddText("Texto Rotacionado", glm::vec2(400, 400), transform, 
                               "Arial", 16.0f, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        
        // Finalizar ciclo de renderização
        m_TextRenderer->EndTextRendering();
        
        LOG_INFO("   ✓ Renderização avançada concluída");
    }
    
    /**
     * @brief Exemplo de MSDF
     */
    void MSDFExample() {
        LOG_INFO("5. Exemplo de MSDF...");
        
        // Criar gerador MSDF
        MSDFConfig msdfConfig;
        msdfConfig.width = 64;
        msdfConfig.height = 64;
        msdfConfig.range = 4.0f;
        msdfConfig.enableSubpixel = true;
        msdfConfig.enableSupersampling = true;
        msdfConfig.supersampleFactor = 4;
        
        MSDFGenerator msdfGenerator(msdfConfig);
        
        // Criar processador de fonte
        FontProcessor fontProcessor;
        if (fontProcessor.LoadFont("fonts/Arial-Regular.ttf")) {
            LOG_INFO("   ✓ Fonte carregada para processamento MSDF");
            
            // Extrair contornos de um glyph
            std::vector<Contour> contours;
            if (fontProcessor.ExtractGlyphContours('A', contours)) {
                LOG_INFO("   ✓ Contornos extraídos para o caractere 'A'");
                
                // Gerar MSDF
                MSDFData msdfData;
                if (msdfGenerator.GenerateFromContours(contours, msdfData)) {
                    LOG_INFO("   ✓ MSDF gerado com sucesso");
                    LOG_INFO("     - Dimensões: " + std::to_string(msdfData.width) + "x" + std::to_string(msdfData.height));
                    LOG_INFO("     - Range: " + std::to_string(msdfData.range));
                    
                    // Aplicar filtros de qualidade
                    msdfGenerator.ApplyQualityFilters(msdfData, TextRenderSettings{});
                    
                    // Converter para RGBA8
                    std::vector<uint8_t> rgba8Data;
                    if (msdfGenerator.ConvertToRGBA8(msdfData, rgba8Data)) {
                        LOG_INFO("   ✓ Conversão para RGBA8 concluída");
                        LOG_INFO("     - Tamanho dos dados: " + std::to_string(rgba8Data.size()) + " bytes");
                    }
                } else {
                    LOG_ERROR("   ✗ Falha ao gerar MSDF");
                }
            } else {
                LOG_ERROR("   ✗ Falha ao extrair contornos");
            }
        } else {
            LOG_ERROR("   ✗ Falha ao carregar fonte para MSDF");
        }
        
        LOG_INFO("   ✓ Exemplo de MSDF concluído");
    }
    
    /**
     * @brief Exemplo de performance
     */
    void PerformanceExample() {
        LOG_INFO("6. Exemplo de performance...");
        
        if (!m_TextRenderer || !m_DefaultFont) {
            LOG_ERROR("   ✗ Renderizador ou fonte não disponível");
            return;
        }
        
        // Teste de performance com muitos textos
        auto startTime = std::chrono::high_resolution_clock::now();
        
        m_TextRenderer->BeginTextRendering();
        
        // Adicionar 1000 textos
        for (int i = 0; i < 1000; ++i) {
            std::string text = "Texto " + std::to_string(i);
            float x = 100.0f + (i % 20) * 150.0f;
            float y = 100.0f + (i / 20) * 30.0f;
            
            m_TextRenderer->AddText(text, glm::vec2(x, y), "Arial", 12.0f, 
                                   glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        
        m_TextRenderer->EndTextRendering();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        LOG_INFO("   ✓ Performance test concluído");
        LOG_INFO("     - 1000 textos renderizados em " + std::to_string(duration.count()) + " μs");
        LOG_INFO("     - Média: " + std::to_string(duration.count() / 1000.0) + " μs por texto");
        
        // Estatísticas do renderizador
        TextRenderStats stats = m_TextRenderer->GetStats();
        LOG_INFO("     - Comandos renderizados: " + std::to_string(stats.commandsRendered));
        LOG_INFO("     - Batches renderizados: " + std::to_string(stats.batchesRendered));
        LOG_INFO("     - Vértices renderizados: " + std::to_string(stats.verticesRendered));
        LOG_INFO("     - Draw calls: " + std::to_string(stats.drawCalls));
    }
    
    /**
     * @brief Exemplo de layout
     */
    void LayoutExample() {
        LOG_INFO("7. Exemplo de layout...");
        
        if (!m_LayoutEngine || !m_DefaultFont) {
            LOG_ERROR("   ✗ Layout engine ou fonte não disponível");
            return;
        }
        
        // Texto longo para teste de layout
        std::string longText = "Este é um texto muito longo que será usado para testar o sistema de layout "
                              "do motor de fontes refatorado. O texto deve ser quebrado em múltiplas linhas "
                              "quando exceder a largura máxima especificada.";
        
        // Layout simples
        TextRenderInfo simpleLayout = m_LayoutEngine->CalculateLayout(longText, m_DefaultFont, 400.0f);
        LOG_INFO("   ✓ Layout simples calculado");
        LOG_INFO("     - Tamanho: " + std::to_string(simpleLayout.size.x) + "x" + std::to_string(simpleLayout.size.y));
        
        // Layout com quebra de linha
        std::vector<TextRenderInfo> multiLineLayout = m_LayoutEngine->CalculateMultiLineLayout(longText, m_DefaultFont, 400.0f);
        LOG_INFO("   ✓ Layout multi-linha calculado");
        LOG_INFO("     - Número de linhas: " + std::to_string(multiLineLayout.size()));
        
        // Layout justificado
        std::vector<TextRenderInfo> justifiedLayout = m_LayoutEngine->CalculateJustifiedLayout(longText, m_DefaultFont, 400.0f);
        LOG_INFO("   ✓ Layout justificado calculado");
        LOG_INFO("     - Número de linhas: " + std::to_string(justifiedLayout.size()));
        
        // Word wrap
        std::vector<std::string> wrappedLines = m_LayoutEngine->WordWrap(longText, m_DefaultFont, 400.0f);
        LOG_INFO("   ✓ Word wrap concluído");
        LOG_INFO("     - Linhas resultantes: " + std::to_string(wrappedLines.size()));
        
        // Truncate
        std::string truncatedText = m_LayoutEngine->TruncateText(longText, m_DefaultFont, 200.0f);
        LOG_INFO("   ✓ Texto truncado");
        LOG_INFO("     - Texto original: " + std::to_string(longText.length()) + " caracteres");
        LOG_INFO("     - Texto truncado: " + std::to_string(truncatedText.length()) + " caracteres");
        
        LOG_INFO("   ✓ Exemplo de layout concluído");
    }
    
    /**
     * @brief Imprime estatísticas finais
     */
    void PrintFinalStats() {
        LOG_INFO("8. Estatísticas finais...");
        
        auto& fontManager = FontManager::GetInstance();
        FontStats stats = fontManager.GetStats();
        
        LOG_INFO("   === Estatísticas do Sistema de Fontes ===");
        LOG_INFO("   - Fontes carregadas: " + std::to_string(stats.totalFonts));
        LOG_INFO("   - Glyphs carregados: " + std::to_string(stats.totalGlyphs));
        LOG_INFO("   - Atlases criados: " + std::to_string(stats.totalAtlases));
        LOG_INFO("   - Uso de memória: " + std::to_string(stats.memoryUsageBytes / 1024 / 1024) + " MB");
        LOG_INFO("   - Acertos no cache: " + std::to_string(stats.cacheHits));
        LOG_INFO("   - Falhas no cache: " + std::to_string(stats.cacheMisses));
        LOG_INFO("   - Taxa de acerto: " + std::to_string(stats.cacheHitRate * 100.0f) + "%");
        
        if (m_TextRenderer) {
            TextRenderStats renderStats = m_TextRenderer->GetStats();
            LOG_INFO("   === Estatísticas de Renderização ===");
            LOG_INFO("   - Comandos renderizados: " + std::to_string(renderStats.commandsRendered));
            LOG_INFO("   - Batches renderizados: " + std::to_string(renderStats.batchesRendered));
            LOG_INFO("   - Vértices renderizados: " + std::to_string(renderStats.verticesRendered));
            LOG_INFO("   - Índices renderizados: " + std::to_string(renderStats.indicesRendered));
            LOG_INFO("   - Draw calls: " + std::to_string(renderStats.drawCalls));
            LOG_INFO("   - Mudanças de estado: " + std::to_string(renderStats.stateChanges));
            LOG_INFO("   - Bindings de textura: " + std::to_string(renderStats.textureBinds));
            LOG_INFO("   - Tempo de renderização: " + std::to_string(renderStats.renderTime) + " ms");
            LOG_INFO("   - Comandos cullados: " + std::to_string(renderStats.culledCommands));
            LOG_INFO("   - Batches cullados: " + std::to_string(renderStats.culledBatches));
        }
        
        LOG_INFO("   === Exemplo Concluído com Sucesso ===");
    }
};

/**
 * @brief Função principal do exemplo
 */
int main() {
    try {
        FontSystemExample example;
        example.Run();
        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Erro no exemplo: " + std::string(e.what()));
        return 1;
    }
} 