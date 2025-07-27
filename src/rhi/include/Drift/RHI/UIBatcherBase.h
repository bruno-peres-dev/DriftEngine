// src/rhi/include/Drift/RHI/UIBatcherBase.h
#pragma once
#include "Drift/RHI/UIBatcher.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace Drift::RHI {

/**
 * @brief Classe base abstrata para implementações de UIBatcher
 * Fornece funcionalidades comuns e facilita a criação de novos backends
 */
class UIBatcherBase : public IUIBatcher {
public:
    UIBatcherBase();
    virtual ~UIBatcherBase() = default;
    
    // === Implementações padrão da interface ===
    void SetScreenSize(float w, float h) override;
    void SetBatchConfig(const UIBatchConfig& config) override;
    UIBatchConfig GetBatchConfig() const override { return m_BatchConfig; }
    UIBatchStats GetStats() const override { return m_Stats; }
    void ResetStats() override;
    
    // === Gerenciamento de texturas ===
    void SetTexture(uint32_t textureId, ITexture* texture) override;
    void ClearTextures() override;
    
    // === Sistema de clipping ===
    void PushScissorRect(float x, float y, float w, float h) override;
    void PopScissorRect() override;
    void ClearScissorRects() override;
    ScissorRect GetCurrentScissorRect() const override;
    
    // === Cache de geometria ===
    uint32_t CreateGeometryCache() override;
    void DestroyGeometryCache(uint32_t cacheId) override;
    void UpdateGeometryCache(uint32_t cacheId, const std::vector<UIVertex>& vertices, 
                           const std::vector<uint32_t>& indices) override;
    void RenderGeometryCache(uint32_t cacheId, float x, float y, Drift::Color color) override;
    
    // === Novas funcionalidades AAA ===
    void SetQualityConfig(const UIBatchQualityConfig& config) override;
    UIBatchQualityConfig GetQualityConfig() const override { return m_QualityConfig; }
    
    // Instancing para elementos repetidos
    void AddInstancedRect(float x, float y, float w, float h, Drift::Color color, size_t instanceCount) override;
    void AddInstancedTexturedRect(float x, float y, float w, float h,
                                 const glm::vec2& uvMin, const glm::vec2& uvMax,
                                 Drift::Color color, uint32_t textureId, size_t instanceCount) override;
    
    // Otimizações de cache
    void TrimGeometryCache() override;
    void TrimTextureCache() override;
    
    // Detecção automática de qualidade
    void AutoDetectQuality() override;
    
    // Informações do backend
    bool SupportsFeature(const std::string& feature) const override;

protected:
    // === Estruturas de dados protegidas ===
    UIBatchConfig m_BatchConfig;
    UIBatchStats m_Stats;
    UIBatchQualityConfig m_QualityConfig;
    
    // === Estado de renderização ===
    float m_ScreenW{1280.0f};
    float m_ScreenH{720.0f};
    bool m_DepthTestEnabled{false};
    bool m_BlendingEnabled{true};
    uint32_t m_SrcBlendFactor{1};
    uint32_t m_DstBlendFactor{6};
    
    // === Sistema de texturas ===
    std::unordered_map<uint32_t, ITexture*> m_Textures;
    std::vector<ITexture*> m_TextureArray;
    uint32_t m_CurrentTextureId{0};
    bool m_TextureChanged{false};
    
    // === Sistema de clipping ===
    std::vector<ScissorRect> m_ScissorStack;
    
    // === Cache de geometria ===
    struct GeometryCache {
        uint32_t id;
        std::vector<UIVertex> vertices;
        std::vector<uint32_t> indices;
        size_t lastUsed;
        bool dirty;
        uint32_t usageCount;
        float boundingBox[4];  // x, y, w, h para frustum culling
        
        GeometryCache() : id(0), lastUsed(0), dirty(false), usageCount(0) {
            std::fill(boundingBox, boundingBox + 4, 0.0f);
        }
    };
    std::unordered_map<uint32_t, GeometryCache> m_GeometryCaches;
    uint32_t m_NextCacheId{1};
    
    // === Métodos virtuais para implementação específica do backend ===
    virtual void OnBegin() = 0;
    virtual void OnEnd() = 0;
    virtual void OnAddRect(float x, float y, float w, float h, Drift::Color color) = 0;
    virtual void OnAddQuad(float x0, float y0, float x1, float y1,
                          float x2, float y2, float x3, float y3,
                          Drift::Color color) = 0;
    virtual void OnAddTexturedRect(float x, float y, float w, float h,
                                  const glm::vec2& uvMin, const glm::vec2& uvMax,
                                  Drift::Color color, uint32_t textureId) = 0;
    virtual void OnAddText(float x, float y, const char* text, Drift::Color color) = 0;
    virtual void OnBeginText() = 0;
    virtual void OnEndText() = 0;
    virtual void OnFlushBatch() = 0;
    virtual void OnSetBlendMode(uint32_t srcFactor, uint32_t dstFactor) = 0;
    virtual void OnSetDepthTest(bool enabled) = 0;
    virtual void OnSetViewport(float x, float y, float w, float h) = 0;
    
    // === Métodos auxiliares protegidos ===
    void UpdateStats(const UIBatchStats& additionalStats);
    void ResetBatchStats();
    bool IsRectVisible(const ScissorRect& rect) const;
    ScissorRect ClipRectToScissor(const ScissorRect& rect, const ScissorRect& scissor) const;
    
    // === Conversões de coordenadas ===
    float ToClipX(float px) const { 
        float clipX = (px / m_ScreenW) * 2.0f - 1.0f;
        return clipX; 
    }
    float ToClipY(float py) const { 
        float clipY = 1.0f - (py / m_ScreenH) * 2.0f;
        return clipY; 
    }
    
    // === Otimizações de cache ===
    void UpdateGeometryCacheUsage(uint32_t cacheId);
    void TrimGeometryCacheInternal();
    void TrimTextureCacheInternal();
    
    // === Detecção de recursos ===
    virtual bool DetectAnisotropicFiltering() const = 0;
    virtual bool DetectMSAA() const = 0;
    virtual uint32_t DetectMaxAnisotropy() const = 0;
    virtual size_t DetectMaxTextureUnits() const = 0;
    virtual size_t DetectMaxVertexAttributes() const = 0;
    
    // === Conversão de cor ===
    static Drift::Color ConvertARGBtoRGBA(Drift::Color argb) {
        uint8_t a = (argb >> 24) & 0xFF;
        uint8_t r = (argb >> 16) & 0xFF;
        uint8_t g = (argb >> 8) & 0xFF;
        uint8_t b = argb & 0xFF;
        return r | (g << 8) | (b << 16) | (a << 24);
    }
};

} // namespace Drift::RHI 