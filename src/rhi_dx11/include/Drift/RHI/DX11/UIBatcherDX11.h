#pragma once
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/UIBatcher.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Texture.h"
#include "Drift/Core/Color.h"
#include <glm/vec2.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>

// Forward declarations
namespace Drift::UI { class UIBatcherTextRenderer; }
namespace Drift::RHI { class IPipelineState; }

namespace Drift::RHI::DX11 {

// Estrutura para cache de geometria
struct GeometryCache {
    uint32_t id;
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
    size_t lastUsed;
    bool dirty;
    
    GeometryCache() : id(0), lastUsed(0), dirty(false) {}
};

// Estrutura para batch de renderização
struct UIBatch {
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t textureId;
    bool hasTexture;
    bool isText;
    size_t vertexCount;
    size_t indexCount;
    
    UIBatch() : textureId(0), hasTexture(false), isText(false), vertexCount(0), indexCount(0) {}
    
    void Clear() {
        vertices.clear();
        indices.clear();
        textureId = 0;
        hasTexture = false;
        isText = false;
        vertexCount = 0;
        indexCount = 0;
    }
    
    bool IsEmpty() const { return vertexCount == 0; }
};

// Implementação DX11 otimizada de IUIBatcher para batching de primitivas 2D
class UIBatcherDX11 : public Drift::RHI::IUIBatcher {
public:
    UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, Drift::RHI::IContext* ctx);
    ~UIBatcherDX11();
    
    // === Gerenciamento de ciclo de vida ===
    void Begin() override;
    void End() override;
    
    // === Primitivas básicas ===
    void AddRect(float x, float y, float w, float h, Drift::Color color) override;
    void AddQuad(float x0, float y0, float x1, float y1,
                 float x2, float y2, float x3, float y3,
                 Drift::Color color) override;
    void AddTexturedRect(float x, float y, float w, float h,
                         const glm::vec2& uvMin, const glm::vec2& uvMax,
                         Drift::Color color, uint32_t textureId) override;
    
    // === Renderização de texto ===
    void AddText(float x, float y, const char* text, Drift::Color color) override;
    
    // === Gerenciamento de texturas ===
    void SetTexture(uint32_t textureId, ITexture* texture) override;
    void ClearTextures() override;
    
    // === Sistema de clipping ===
    void PushScissorRect(float x, float y, float w, float h) override;
    void PopScissorRect() override;
    void ClearScissorRects() override;
    ScissorRect GetCurrentScissorRect() const override;
    
    // === Configuração e estatísticas ===
    void SetScreenSize(float w, float h) override;
    void SetBatchConfig(const UIBatchConfig& config) override;
    UIBatchConfig GetBatchConfig() const override { return m_BatchConfig; }
    UIBatchStats GetStats() const override { return m_Stats; }
    void ResetStats() override;
    
    // === Otimizações avançadas ===
    void FlushBatch() override;
    void SetBlendMode(uint32_t srcFactor, uint32_t dstFactor) override;
    void SetDepthTest(bool enabled) override;
    void SetViewport(float x, float y, float w, float h) override;
    
    // === Cache de geometria ===
    uint32_t CreateGeometryCache() override;
    void DestroyGeometryCache(uint32_t cacheId) override;
    void UpdateGeometryCache(uint32_t cacheId, const std::vector<UIVertex>& vertices, 
                           const std::vector<uint32_t>& indices) override;
    void RenderGeometryCache(uint32_t cacheId, float x, float y, Drift::Color color) override;

private:
    // === Estruturas de dados ===
    UIBatchConfig m_BatchConfig;
    UIBatchStats m_Stats;
    UIBatch m_CurrentBatch;
    
    // === Estado de renderização ===
    float m_ScreenW{1280.0f};
    float m_ScreenH{720.0f};
    bool m_DepthTestEnabled{false};
    bool m_BlendingEnabled{true};
    uint32_t m_SrcBlendFactor{1}; // SRC_ALPHA
    uint32_t m_DstBlendFactor{6}; // ONE_MINUS_SRC_ALPHA
    
    // === Recursos gráficos ===
    std::shared_ptr<Drift::RHI::IPipelineState> m_Pipeline;
    std::shared_ptr<Drift::RHI::IPipelineState> m_TextPipeline;
    std::shared_ptr<IRingBuffer> m_RingBuffer;
    Drift::RHI::IContext* m_Context;
    
    // === Sistema de texturas ===
    std::unordered_map<uint32_t, ITexture*> m_Textures;
    std::vector<ITexture*> m_TextureArray;
    uint32_t m_CurrentTextureId{0};
    bool m_TextureChanged{false};
    
    // === Sistema de clipping ===
    std::vector<ScissorRect> m_ScissorStack;
    
    // === Cache de geometria ===
    std::unordered_map<uint32_t, GeometryCache> m_GeometryCaches;
    uint32_t m_NextCacheId{1};
    
    // === Sistema de renderização de texto ===
    std::unique_ptr<Drift::UI::UIBatcherTextRenderer> m_TextRenderer;
    std::shared_ptr<ISampler> m_DefaultSampler;
    std::shared_ptr<IBuffer> m_TextCB;
    bool m_AddingText{false};
    
    // === Otimizações ===
    std::vector<UIVertex> m_VertexBuffer;
    std::vector<uint32_t> m_IndexBuffer;
    bool m_BatchDirty{false};
    
    // === Métodos auxiliares ===
    void EnsurePipeline();
    void CreateTextPipeline();
    void FlushCurrentBatch();
    void RenderBatch(const UIBatch& batch);
    bool IsRectVisible(const ScissorRect& rect) const;
    ScissorRect ClipRectToScissor(const ScissorRect& rect, const ScissorRect& scissor) const;
    void EnsureUIPipeline();
    
    // === Conversões de coordenadas ===
    float ToClipX(float px) const { 
        float clipX = (px / m_ScreenW) * 2.0f - 1.0f;
        return clipX; 
    }
    float ToClipY(float py) const { 
        float clipY = 1.0f - (py / m_ScreenH) * 2.0f;
        return clipY; 
    }
    
    // === Otimizações de memória ===
    void AllocateBuffers();
    void UpdateVertexBuffer();
    void UpdateIndexBuffer();
    
    // === Estatísticas ===
    void UpdateStats(const UIBatch& batch);
    void ResetBatchStats();
};

// Fábrica para criar UIBatcherDX11
std::unique_ptr<Drift::RHI::IUIBatcher> CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, Drift::RHI::IContext* ctx);

} // namespace Drift::RHI::DX11 