#pragma once

#include "FontMetrics.h"
#include "FontAtlas.h"
#include "Drift/RHI/UIBatcher.h"
#include "Drift/RHI/Shader.h"
#include "Drift/RHI/PipelineState.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace Drift::UI {

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
    bool Initialize();
    void Shutdown();
    
    // Configuração
    void SetDevice(Drift::RHI::IDevice* device) { m_Device = device; }
    void SetBatcher(Drift::RHI::IUIBatcher* batcher) { m_Batcher = batcher; }
    void SetScreenSize(int width, int height);
    
    // Renderização de texto
    void BeginTextRendering();
    void EndTextRendering();
    
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
    
    // Batching e otimizações
    void FlushBatch();
    void ClearBatch();
    size_t GetBatchSize() const { return m_CurrentBatchSize; }
    
    // Estatísticas
    struct RenderStats {
        size_t drawCalls = 0;
        size_t verticesRendered = 0;
        size_t charactersRendered = 0;
        size_t batchesFlushed = 0;
        double renderTime = 0.0;
    };
    
    RenderStats GetStats() const { return m_Stats; }
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
    bool CompileShader(const std::string& name, Drift::RHI::ShaderType type);
    
    // Constantes
    static constexpr size_t MAX_VERTICES_PER_BATCH = 10000;
    static constexpr size_t MAX_INDICES_PER_BATCH = 15000;
    static constexpr size_t VERTEX_BUFFER_SIZE = MAX_VERTICES_PER_BATCH * sizeof(TextVertex);
    static constexpr size_t INDEX_BUFFER_SIZE = MAX_INDICES_PER_BATCH * sizeof(uint32_t);
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
};

} // namespace Drift::UI 