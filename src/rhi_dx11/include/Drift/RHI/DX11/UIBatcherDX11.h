#pragma once
#include "Drift/RHI/UIBatcherBase.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Texture.h"
#include <memory>
#include <vector>
#include <array>

// Forward declarations

namespace Drift::RHI { class IPipelineState; }
namespace Drift::UI { class TextRenderer; }

namespace Drift::RHI::DX11 {

/**
 * @brief Estrutura para constantes do shader UI
 */
struct UIConstants {
    float screenSize[2];    // Largura e altura da tela
    float atlasSize[2];     // Tamanho do atlas de fontes
    float padding[2];       // Padding para otimizações
    float time;             // Tempo para animações
    float debugColor[4];    // Cor de debug
};

/**
 * @brief Estrutura para batch de renderização específica do DX11
 */
struct UIBatch {
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
    std::array<uint32_t, 16> textureIds;  // Aumentado para 16 texturas
    uint32_t textureCount;
    bool hasTexture;
    bool isText;
    bool isInstanced;
    size_t vertexCount;
    size_t indexCount;
    size_t instanceCount;
    float boundingBox[4];  // Para frustum culling
    
    UIBatch() : textureCount(0), hasTexture(false), isText(false), 
                isInstanced(false), vertexCount(0), indexCount(0), instanceCount(1) {
        std::fill(textureIds.begin(), textureIds.end(), 8);
        std::fill(boundingBox, boundingBox + 4, 0.0f);
    }
    
    void Clear() {
        vertices.clear();
        indices.clear();
        std::fill(textureIds.begin(), textureIds.end(), 8);
        textureCount = 0;
        hasTexture = false;
        isText = false;
        isInstanced = false;
        vertexCount = 0;
        indexCount = 0;
        instanceCount = 1;
        std::fill(boundingBox, boundingBox + 4, 0.0f);
    }
    
    bool IsEmpty() const { return vertexCount == 0; }
    
    void CalculateBoundingBox() {
        if (vertices.empty()) return;
        
        float minX = vertices[0].x, maxX = vertices[0].x;
        float minY = vertices[0].y, maxY = vertices[0].y;
        
        for (const auto& v : vertices) {
            minX = std::min(minX, v.x);
            maxX = std::max(maxX, v.x);
            minY = std::min(minY, v.y);
            maxY = std::max(maxY, v.y);
        }
        
        boundingBox[0] = minX;
        boundingBox[1] = minY;
        boundingBox[2] = maxX - minX;
        boundingBox[3] = maxY - minY;
    }
};

/**
 * @brief Estrutura para comando de renderização
 */
struct UIRenderCommand {
    enum class Type {
        Rect,
        Quad,
        TexturedRect,
        Text,
        GeometryCache,
        Instanced
    };
    
    Type type;
    float x, y, w, h;
    Drift::Color color;
    uint32_t textureId;
    uint32_t geometryCacheId;
    size_t instanceCount;
    glm::vec2 uvMin, uvMax;
    
    UIRenderCommand() : type(Type::Rect), x(0), y(0), w(0), h(0), 
                       color(0xFFFFFFFF), textureId(8), geometryCacheId(0), 
                       instanceCount(1), uvMin(0,0), uvMax(1,1) {}
};

/**
 * @brief Pool de vértices otimizado
 */
class VertexPool {
public:
    explicit VertexPool(size_t capacity);
    ~VertexPool();
    
    UIVertex* Allocate(size_t count);
    void Reset();
    size_t GetCapacity() const { return m_Capacity; }
    size_t GetUsed() const { return m_Used; }
    
private:
    std::vector<UIVertex> m_Vertices;
    size_t m_Capacity;
    size_t m_Used;
};

/**
 * @brief Sistema de culling otimizado
 */
class UICullingSystem {
public:
    void SetViewport(float x, float y, float w, float h);
    bool IsVisible(const float* boundingBox) const;
    void EnableFrustumCulling(bool enable) { m_EnableFrustumCulling = enable; }
    void EnableOcclusionCulling(bool enable) { m_EnableOcclusionCulling = enable; }
    
private:
    float m_Viewport[4];  // x, y, w, h
    bool m_EnableFrustumCulling = true;
    bool m_EnableOcclusionCulling = true;
};

/**
 * @brief Implementação DX11 otimizada de IUIBatcher para nível AAA
 */
class UIBatcherDX11 : public UIBatcherBase {
public:
    UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx);
    ~UIBatcherDX11();
    
    // === Implementação da interface ===
    void Begin() override;
    void End() override;
    void AddRect(float x, float y, float w, float h, Drift::Color color) override;
    void AddQuad(float x0, float y0, float x1, float y1,
                 float x2, float y2, float x3, float y3,
                 Drift::Color color) override;
    void AddTexturedRect(float x, float y, float w, float h,
                         const glm::vec2& uvMin, const glm::vec2& uvMax,
                         Drift::Color color, uint32_t textureId) override;
    void AddText(float x, float y, const char* text, Drift::Color color) override;
    void BeginText() override;
    void EndText() override;
    void FlushBatch() override;
    void SetBlendMode(uint32_t srcFactor, uint32_t dstFactor) override;
    void SetDepthTest(bool enabled) override;
    void SetViewport(float x, float y, float w, float h) override;
    
    // === Informações do backend ===
    std::string GetBackendName() const override { return "DirectX 11"; }

protected:
    // === Implementação dos métodos virtuais da classe base ===
    void OnBegin() override;
    void OnEnd() override;
    void OnAddRect(float x, float y, float w, float h, Drift::Color color) override;
    void OnAddQuad(float x0, float y0, float x1, float y1,
                   float x2, float y2, float x3, float y3,
                   Drift::Color color) override;
    void OnAddTexturedRect(float x, float y, float w, float h,
                           const glm::vec2& uvMin, const glm::vec2& uvMax,
                           Drift::Color color, uint32_t textureId) override;
    void OnAddText(float x, float y, const char* text, Drift::Color color) override;
    void OnBeginText() override;
    void OnEndText() override;
    void OnFlushBatch() override;
    void OnSetBlendMode(uint32_t srcFactor, uint32_t dstFactor) override;
    void OnSetDepthTest(bool enabled) override;
    void OnSetViewport(float x, float y, float w, float h) override;
    
    // === Detecção de recursos específica do DX11 ===
    bool DetectAnisotropicFiltering() const override;
    bool DetectMSAA() const override;
    uint32_t DetectMaxAnisotropy() const override;
    size_t DetectMaxTextureUnits() const override;
    size_t DetectMaxVertexAttributes() const override;

private:
    // === Recursos específicos do DX11 ===
    std::shared_ptr<Drift::RHI::IPipelineState> m_Pipeline;
    std::shared_ptr<Drift::RHI::IPipelineState> m_TextPipeline;
    std::shared_ptr<Drift::RHI::IPipelineState> m_InstancedPipeline;
    std::shared_ptr<IRingBuffer> m_RingBuffer;
    IContext* m_Context;
    
    // === Sistema de renderização de texto ===
    std::unique_ptr<Drift::UI::TextRenderer> m_TextRenderer;
    std::shared_ptr<ISampler> m_DefaultSampler;
    std::shared_ptr<IBuffer> m_TextCB;
    bool m_AddingText{false};
    
    // === Constant buffer para UI ===
    std::shared_ptr<IBuffer> m_UIConstantsBuffer;
    UIConstants m_UIConstants;
    
    // === Otimizações específicas do DX11 ===
    std::unique_ptr<VertexPool> m_VertexPool;
    std::unique_ptr<UICullingSystem> m_CullingSystem;
    std::vector<UIRenderCommand> m_CommandBuffer;
    std::vector<UIVertex> m_VertexBuffer;
    std::vector<uint32_t> m_IndexBuffer;
    bool m_BatchDirty{false};
    
    // === Métodos auxiliares específicos do DX11 ===
    void CreateDefaultSampler();
    void EnsurePipeline();
    void CreateTextPipeline();
    void CreateInstancedPipeline();
    void RenderBatch(const UIBatch& batch);
    void ProcessCommandBuffer();
    void SortCommandsByTexture();
    void RenderVertices(const UIVertex* vertices, size_t vertexCount, 
                       const uint32_t* indices, size_t indexCount, bool hasTexture);
    bool ShouldFlushBatch(const UIRenderCommand& cmd) const;
    void OptimizeBatch(UIBatch& batch);
    void UpdateCullingSystem();
    void AllocateBuffers();
    void UpdateVertexBuffer();
    void UpdateIndexBuffer();
    void CreateUIConstantsBuffer();
    void UpdateUIConstantsBuffer();
};

// Fábrica para criar UIBatcherDX11
std::unique_ptr<IUIBatcher> CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx);

} // namespace Drift::RHI::DX11 