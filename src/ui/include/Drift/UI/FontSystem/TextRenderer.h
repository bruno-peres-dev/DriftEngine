#pragma once

#include "FontManager.h"
#include "FontMetrics.h"
#include "FontRendering.h"
#include "Drift/RHI/UIBatcher.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <memory>

namespace Drift::UI {

/**
 * @brief Configuração de renderização de texto
 */
struct TextRendererConfig {
    // Configurações de qualidade
    bool enableSubpixelRendering = true;      // Renderização subpixel
    bool enableAntiAliasing = true;           // Anti-aliasing
    bool enableGammaCorrection = true;        // Correção gamma
    float gamma = 2.2f;                       // Valor gamma
    
    // Configurações de cache
    bool enableTextCache = true;              // Cache de medidas de texto
    size_t maxCachedStrings = 1000;           // Máximo de strings em cache
    size_t maxCacheMemory = 10 * 1024 * 1024; // Máximo de memória para cache (10MB)
    
    // Configurações de performance
    bool enableBatching = true;               // Batching de draw calls
    bool enableInstancing = true;             // Instancing para performance
    size_t maxBatchSize = 1000;               // Tamanho máximo do batch
    
    // Configurações de efeitos
    bool enableEffects = true;                // Habilita efeitos de texto
    bool enableShadows = true;                // Habilita sombras
    bool enableOutlines = true;               // Habilita outlines
    bool enableGradients = true;              // Habilita gradientes
};

/**
 * @brief Renderizador de Texto Profissional
 * 
 * Interface de alto nível para renderização de texto, integrando todos os
 * componentes do sistema de fontes profissional:
 * 
 * - FontManager para gerenciamento de fontes
 * - FontMetrics para cálculos de layout
 * - FontRendering para renderização de alta qualidade
 * - Sistema de cache otimizado
 * - Suporte a efeitos avançados
 * - Batching automático
 */
class TextRenderer {
public:
    /**
     * @brief Construtor
     * @param device Dispositivo RHI
     */
    TextRenderer(Drift::RHI::IDevice* device);
    
    /**
     * @brief Destrutor
     */
    ~TextRenderer();

    // Inicialização e configuração
    bool Initialize(const TextRendererConfig& config = {});
    void Shutdown();
    
    // Configuração
    void SetConfig(const TextRendererConfig& config);
    const TextRendererConfig& GetConfig() const { return m_Config; }
    void SetBatcher(Drift::RHI::IUIBatcher* batcher);
    void SetScreenSize(int width, int height);

    // Renderização de texto
    void BeginTextRendering();
    void EndTextRendering();

    /**
     * @brief Renderiza texto simples
     * @param text Texto a ser renderizado
     * @param position Posição na tela
     * @param fontName Nome da fonte
     * @param fontSize Tamanho da fonte
     * @param color Cor do texto
     */
    void RenderText(const std::string& text, const glm::vec2& position,
                    const std::string& fontName, float fontSize, const glm::vec4& color);

    /**
     * @brief Renderiza texto com configuração completa
     * @param text Texto a ser renderizado
     * @param position Posição na tela
     * @param font Fonte a ser usada
     * @param config Configuração de renderização
     */
    void RenderText(const std::string& text, const glm::vec2& position,
                    const std::shared_ptr<Font>& font, const TextRenderConfig& config = {});

    /**
     * @brief Renderiza texto com layout pré-calculado
     * @param layout Layout do texto
     * @param position Posição na tela
     * @param font Fonte a ser usada
     * @param config Configuração de renderização
     */
    void RenderText(const TextLayoutResult& layout, const glm::vec2& position,
                    const std::shared_ptr<Font>& font, const TextRenderConfig& config = {});

    // Renderização com efeitos
    void RenderTextWithShadow(const std::string& text, const glm::vec2& position,
                              const std::shared_ptr<Font>& font, const glm::vec4& color,
                              const glm::vec2& shadowOffset, const glm::vec4& shadowColor);

    void RenderTextWithOutline(const std::string& text, const glm::vec2& position,
                               const std::shared_ptr<Font>& font, const glm::vec4& color,
                               float outlineWidth, const glm::vec4& outlineColor);

    void RenderTextWithGradient(const std::string& text, const glm::vec2& position,
                                const std::shared_ptr<Font>& font,
                                const glm::vec4& startColor, const glm::vec4& endColor,
                                const glm::vec2& gradientDirection);

    // Medidas de texto
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName, float fontSize);
    glm::vec2 MeasureText(const std::string& text, const std::shared_ptr<Font>& font);
    glm::vec2 MeasureText(const std::string& text, const std::shared_ptr<Font>& font,
                          const TextLayoutConfig& layoutConfig);

    float GetTextWidth(const std::string& text, const std::shared_ptr<Font>& font);
    float GetTextHeight(const std::string& text, const std::shared_ptr<Font>& font,
                        const TextLayoutConfig& layoutConfig = {});

    // Layout de texto
    TextLayoutResult CalculateLayout(const std::string& text, const std::shared_ptr<Font>& font,
                                    const TextLayoutConfig& layoutConfig = {});

    std::vector<std::string> BreakTextIntoLines(const std::string& text,
                                                const std::shared_ptr<Font>& font,
                                                float maxWidth,
                                                const TextLayoutConfig& layoutConfig = {});

    // Cache e otimizações
    void ClearTextCache();
    void TrimTextCache();
    size_t GetTextCacheSize() const;
    size_t GetCacheMemoryUsage() const;

    // Estatísticas
    struct TextRendererStats {
        size_t textRenderCalls = 0;
        size_t charactersRendered = 0;
        size_t drawCalls = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
        size_t batchesFlushed = 0;
        double renderTime = 0.0;
        double layoutTime = 0.0;
        
        // Estatísticas de fontes
        size_t fontsUsed = 0;
        size_t fallbackUsage = 0;
    };

    TextRendererStats GetStats() const;
    void LogStats() const;
    void ResetStats();

private:
    // Componentes do sistema
    std::unique_ptr<FontMetrics> m_FontMetrics;
    std::unique_ptr<FontRendering> m_FontRendering;
    
    // Configuração
    TextRendererConfig m_Config;
    Drift::RHI::IDevice* m_Device{nullptr};
    Drift::RHI::IUIBatcher* m_Batcher{nullptr};
    int m_ScreenWidth{0};
    int m_ScreenHeight{0};
    
    // Cache de medidas de texto
    struct TextMeasureCache {
        glm::vec2 size;
        size_t lastUsed = 0;
        size_t accessCount = 0;
        size_t memoryUsage = 0;
    };
    
    struct TextCacheKey {
        std::string text;
        std::string fontName;
        float fontSize;
        TextLayoutConfig layoutConfig;
        
        bool operator==(const TextCacheKey& other) const;
    };
    
    struct TextCacheKeyHash {
        size_t operator()(const TextCacheKey& key) const;
    };
    
    std::unordered_map<TextCacheKey, TextMeasureCache, TextCacheKeyHash> m_TextCache;
    
    // Estatísticas
    TextRendererStats m_Stats;
    
    // Métodos auxiliares
    glm::vec2 GetCachedTextMeasure(const std::string& text, const std::string& fontName, 
                                   float fontSize, const TextLayoutConfig& layoutConfig = {});
    
    void CacheTextMeasure(const std::string& text, const std::string& fontName,
                          float fontSize, const glm::vec2& size,
                          const TextLayoutConfig& layoutConfig = {});
    
    void TrimTextCache();
    size_t CalculateTextCacheMemoryUsage() const;
    
    // Utilitários
    std::shared_ptr<Font> GetOrLoadFont(const std::string& fontName, float fontSize);
    void UpdateStats(bool cacheHit);
    
    // Constantes
    static constexpr size_t MAX_CACHE_ENTRIES = 10000;
    static constexpr size_t MAX_CACHE_MEMORY = 50 * 1024 * 1024; // 50MB
};

/**
 * @brief Renderizador de texto especializado para UIBatcher
 */
class UIBatcherTextRenderer : public TextRenderer {
public:
    /**
     * @brief Construtor
     * @param device Dispositivo RHI
     * @param batcher Batcher UI
     */
    UIBatcherTextRenderer(Drift::RHI::IDevice* device, Drift::RHI::IUIBatcher* batcher)
        : TextRenderer(device) {
        SetBatcher(batcher);
    }
    
    /**
     * @brief Renderiza texto usando o UIBatcher
     */
    void RenderTextUIBatch(const std::string& text,
                           const glm::vec2& position,
                           const std::string& fontName,
                           float fontSize,
                           const glm::vec4& color,
                           float scale = 1.0f);
    
    /**
     * @brief Renderiza texto com fonte específica usando UIBatcher
     */
    void RenderTextUIBatch(const std::string& text,
                           const glm::vec2& position,
                           const std::shared_ptr<Font>& font,
                           const glm::vec4& color,
                           float scale = 1.0f);
};

// Macros para facilitar o uso
#define DRIFT_TEXT_RENDERER() Drift::UI::TextRenderer::GetInstance()

#define DRIFT_RENDER_TEXT(text, pos, font, size, color) \
    DRIFT_TEXT_RENDERER().RenderText(text, pos, font, size, color)

#define DRIFT_MEASURE_TEXT(text, font, size) \
    DRIFT_TEXT_RENDERER().MeasureText(text, font, size)

} // namespace Drift::UI
