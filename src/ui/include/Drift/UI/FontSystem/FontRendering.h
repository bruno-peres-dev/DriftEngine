#pragma once

#include "FontMetrics.h"
#include "FontAtlas.h"
#include "FontManager.h"
#include "Drift/RHI/UIBatcher.h"
#include "Drift/RHI/Shader.h"
#include "Drift/RHI/PipelineState.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace Drift::UI {

/**
 * @brief Configuração do sistema de renderização de fontes
 */
struct FontRenderingConfig {
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
 * @brief Tipos de efeitos de texto
 */
enum class TextEffect {
    None,
    Outline,
    Shadow,
    Glow,
    Gradient,
    Emboss,
    Bevel
};

/**
 * @brief Configuração de efeitos de texto
 */
struct TextEffectConfig {
    TextEffect type = TextEffect::None;
    
    // Outline
    float outlineWidth = 2.0f;
    glm::vec4 outlineColor{0.0f, 0.0f, 0.0f, 1.0f};
    
    // Shadow
    glm::vec2 shadowOffset{2.0f, 2.0f};
    glm::vec4 shadowColor{0.0f, 0.0f, 0.0f, 0.5f};
    float shadowBlur = 1.0f;
    
    // Glow
    float glowRadius = 3.0f;
    glm::vec4 glowColor{1.0f, 1.0f, 0.0f, 0.8f};
    
    // Gradient
    glm::vec4 gradientStart{1.0f, 0.0f, 0.0f, 1.0f};
    glm::vec4 gradientEnd{0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec2 gradientDirection{0.0f, 1.0f};
    
    // Emboss/Bevel
    float embossDepth = 1.0f;
    glm::vec4 embossLight{1.0f, 1.0f, 1.0f, 0.5f};
    glm::vec4 embossDark{0.0f, 0.0f, 0.0f, 0.5f};
};

/**
 * @brief Configuração de renderização de texto
 */
struct TextRenderConfig {
    // Configurações básicas
    glm::vec4 color{1.0f};                    // Cor do texto
    float alpha = 1.0f;                       // Transparência
    glm::vec2 position{0.0f};                 // Posição na tela
    glm::vec2 scale{1.0f};                    // Escala do texto
    float rotation = 0.0f;                    // Rotação em radianos
    
    // Configurações de qualidade
    bool enableSubpixelRendering = true;      // Renderização subpixel
    bool enableAntiAliasing = true;           // Anti-aliasing
    bool enableGammaCorrection = true;        // Correção gamma
    float gamma = 2.2f;                       // Valor gamma
    
    // Configurações de efeitos
    std::vector<TextEffectConfig> effects;    // Lista de efeitos
    
    // Configurações de clipping
    glm::vec4 clipRect{0.0f};                 // Retângulo de clipping (x, y, w, h)
    bool enableClipping = false;              // Habilita clipping
    
    // Configurações de performance
    bool enableBatching = true;               // Batching de draw calls
    bool enableInstancing = true;             // Instancing para performance
    size_t maxBatchSize = 1000;               // Tamanho máximo do batch
};

/**
 * @brief Vértice para renderização de texto
 */
struct TextVertex {
    glm::vec2 position;       // Posição na tela
    glm::vec2 texCoord;       // Coordenadas de textura
    glm::vec4 color;          // Cor do vértice
    float effectData[4];      // Dados para efeitos
};

/**
 * @brief Sistema de Renderização de Fontes Profissional
 * 
 * Fornece renderização de alta qualidade para texto, incluindo:
 * - Renderização com anti-aliasing
 * - Efeitos de texto avançados
 * - Batching otimizado
 * - Suporte a múltiplas fontes
 * - Renderização em tempo real
 * - Cache de medidas de texto
 * - Integração com FontManager e FontMetrics
 */
class FontRendering {
public:
    /**
     * @brief Construtor
     * @param device Dispositivo RHI
     */
    FontRendering(Drift::RHI::IDevice* device);
    
    /**
     * @brief Destrutor
     */
    ~FontRendering();

    // Inicialização e configuração
    bool Initialize(const FontRenderingConfig& config = {});
    void Shutdown();
    
    // Configuração
    void SetDevice(Drift::RHI::IDevice* device) { m_Device = device; }
    void SetBatcher(Drift::RHI::IUIBatcher* batcher) { m_Batcher = batcher; }
    void SetScreenSize(int width, int height);
    void SetConfig(const FontRenderingConfig& config);
    const FontRenderingConfig& GetConfig() const { return m_Config; }
    
    // Renderização de texto
    void BeginTextRendering();
    void EndTextRendering();
    
    // Renderização básica
    void RenderText(const std::string& text, const glm::vec2& position,
                    const std::string& fontName, float fontSize, const glm::vec4& color);
    
    void RenderText(const std::string& text,
                    const std::shared_ptr<Font>& font,
                    const TextRenderConfig& config = {});
    
    void RenderText(const TextLayoutResult& layout,
                    const std::shared_ptr<Font>& font,
                    const TextRenderConfig& config = {});
    
    // Renderização de efeitos
    void RenderTextWithEffects(const std::string& text,
                               const std::shared_ptr<Font>& font,
                               const TextRenderConfig& config);
    
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
    
    void RenderOutline(const std::string& text,
                       const std::shared_ptr<Font>& font,
                       const TextEffectConfig& effect,
                       const TextRenderConfig& config);
    
    void RenderShadow(const std::string& text,
                      const std::shared_ptr<Font>& font,
                      const TextEffectConfig& effect,
                      const TextRenderConfig& config);
    
    void RenderGlow(const std::string& text,
                    const std::shared_ptr<Font>& font,
                    const TextEffectConfig& effect,
                    const TextRenderConfig& config);
    
    // Medidas de texto
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName, float fontSize);
    glm::vec2 MeasureText(const std::string& text, const std::shared_ptr<Font>& font);
    glm::vec2 MeasureText(const std::string& text, const std::shared_ptr<Font>& font,
                          const TextLayoutConfig& layoutConfig);
    
    // Carregamento de fontes
    std::shared_ptr<Font> GetOrLoadFont(const std::string& fontName, float fontSize);
    
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
    
    // Batching e otimizações
    void FlushBatch();
    void ClearBatch();
    size_t GetBatchSize() const { return m_CurrentBatchSize; }
    
    // Cache e otimizações
    void ClearTextCache();
    size_t GetTextCacheSize() const;
    size_t GetCacheMemoryUsage() const;
    
    // Estatísticas
    struct RenderStats {
        size_t textRenderCalls = 0;
        size_t drawCalls = 0;
        size_t verticesRendered = 0;
        size_t charactersRendered = 0;
        size_t batchesFlushed = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
        double renderTime = 0.0;
        double layoutTime = 0.0;
        
        // Estatísticas de fontes
        size_t fontsUsed = 0;
        size_t fallbackUsage = 0;
    };
    
    RenderStats GetStats() const { return m_Stats; }
    void LogStats() const;
    void ResetStats();

private:
    // Estruturas de dados
    struct TextBatch {
        std::shared_ptr<Font> font;
        std::shared_ptr<Drift::RHI::ITexture> texture;
        std::vector<TextVertex> vertices;
        std::vector<uint32_t> indices;
        TextRenderConfig config;
        size_t vertexCount = 0;
        size_t indexCount = 0;
    };
    
    struct ShaderData {
        std::shared_ptr<Drift::RHI::IShader> vertexShader;
        std::shared_ptr<Drift::RHI::IShader> pixelShader;
        std::shared_ptr<Drift::RHI::IPipelineState> pipelineState;
    };
    
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

    // Componentes do sistema
    std::unique_ptr<FontMetrics> m_FontMetrics;
    
    // Configuração
    FontRenderingConfig m_Config;
    Drift::RHI::IDevice* m_Device{nullptr};
    Drift::RHI::IUIBatcher* m_Batcher{nullptr};
    int m_ScreenWidth{0};
    int m_ScreenHeight{0};
    
    // Shaders e pipelines
    std::unordered_map<std::string, ShaderData> m_Shaders;
    std::shared_ptr<Drift::RHI::IPipelineState> m_DefaultPipeline;
    
    // Batching
    std::vector<TextBatch> m_Batches;
    TextBatch* m_CurrentBatch{nullptr};
    size_t m_CurrentBatchSize{0};
    size_t m_MaxBatchSize{1000};
    
    // Buffers
    std::shared_ptr<Drift::RHI::IBuffer> m_VertexBuffer;
    std::shared_ptr<Drift::RHI::IBuffer> m_IndexBuffer;
    std::shared_ptr<Drift::RHI::IBuffer> m_ConstantBuffer;
    
    // Cache de medidas de texto
    std::unordered_map<TextCacheKey, TextMeasureCache, TextCacheKeyHash> m_TextCache;
    
    // Estatísticas
    RenderStats m_Stats;
    
    // Métodos auxiliares
    bool CreateShaders();
    bool CreateBuffers();
    bool CreatePipelines();
    
    void AddVertexToBatch(const TextVertex& vertex);
    void AddQuadToBatch(const glm::vec2& pos, const glm::vec2& size,
                        const glm::vec2& uv0, const glm::vec2& uv1,
                        const glm::vec4& color);
    
    void RenderBatch(const TextBatch& batch);
    void RenderBatchInstanced(const TextBatch& batch);
    
    void ApplyEffect(const TextEffectConfig& effect,
                     const std::vector<TextVertex>& baseVertices,
                     std::vector<TextVertex>& effectVertices);
    
    glm::vec4 ApplyGammaCorrection(const glm::vec4& color) const;
    glm::vec2 ApplySubpixelOffset(const glm::vec2& position) const;
    
    // Utilitários de shader
    std::string GetShaderSource(const std::string& name) const;
    bool CompileShader(const std::string& name, const std::string& shaderType);
    
    // Cache de medidas
    glm::vec2 GetCachedTextMeasure(const std::string& text, const std::string& fontName, 
                                   float fontSize, const TextLayoutConfig& layoutConfig = {});
    
    void CacheTextMeasure(const std::string& text, const std::string& fontName,
                          float fontSize, const glm::vec2& size,
                          const TextLayoutConfig& layoutConfig = {});
    
    void TrimTextCache();
    size_t CalculateTextCacheMemoryUsage() const;
    
    // Utilitários
    void UpdateStats(bool cacheHit);
    std::vector<uint32_t> DecodeUTF8(const std::string& utf8_string);
    
    // Cálculo de medidas
    glm::vec2 CalculateTextMeasure(const std::string& text, const std::shared_ptr<Font>& font);
    
    // Constantes
    static constexpr size_t MAX_VERTICES_PER_BATCH = 10000;
    static constexpr size_t MAX_INDICES_PER_BATCH = 15000;
    static constexpr size_t VERTEX_BUFFER_SIZE = MAX_VERTICES_PER_BATCH * sizeof(TextVertex);
    static constexpr size_t INDEX_BUFFER_SIZE = MAX_INDICES_PER_BATCH * sizeof(uint32_t);
    static constexpr size_t MAX_CACHE_ENTRIES = 10000;
    static constexpr size_t MAX_CACHE_MEMORY = 50 * 1024 * 1024; // 50MB
};

/**
 * @brief Renderizador de texto especializado para UIBatcher
 */
class UIBatcherFontRenderer : public FontRendering {
public:
    /**
     * @brief Construtor
     * @param device Dispositivo RHI
     * @param batcher Batcher UI
     */
    UIBatcherFontRenderer(Drift::RHI::IDevice* device, Drift::RHI::IUIBatcher* batcher)
        : FontRendering(device) {
        SetBatcher(batcher);
    }
    
    /**
     * @brief Renderiza texto usando o UIBatcher
     */
    void RenderTextUIBatch(const std::string& text,
                           const std::shared_ptr<Font>& font,
                           const glm::vec2& position,
                           const glm::vec4& color,
                           float scale = 1.0f);
    
    /**
     * @brief Renderiza texto com fonte específica usando UIBatcher
     */
    void RenderTextUIBatch(const std::string& text,
                           const glm::vec2& position,
                           const std::string& fontName,
                           float fontSize,
                           const glm::vec4& color,
                           float scale = 1.0f);
};

// Macros para facilitar o uso
#define DRIFT_FONT_RENDERER() Drift::UI::FontRendering::GetInstance()

#define DRIFT_RENDER_TEXT(text, pos, font, size, color) \
    DRIFT_FONT_RENDERER().RenderText(text, pos, font, size, color)

#define DRIFT_MEASURE_TEXT(text, font, size) \
    DRIFT_FONT_RENDERER().MeasureText(text, font, size)

} // namespace Drift::UI 