// src/rhi/include/Drift/RHI/UIBatcher.h
#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/mat4x4.hpp>
#include "Drift/Core/Color.h"
#include "Drift/RHI/Scissor.h"

// Forward declarations
namespace Drift::RHI { class ITexture; }

namespace Drift::RHI {

// Estrutura para vértices de UI otimizada
struct UIVertex {
    float x, y;           // Posição em coordenadas de tela
    float u, v;           // Coordenadas UV (para texturas)
    Drift::Color color;   // Cor RGBA
    uint32_t textureId;   // ID da textura (0 = sem textura)
    
    UIVertex() : x(0), y(0), u(0), v(0), color(0xFFFFFFFF), textureId(0) {}
    UIVertex(float x, float y, float u, float v, Drift::Color color, uint32_t textureId = 0)
        : x(x), y(y), u(u), v(v), color(color), textureId(textureId) {}
};

// Configurações de batching
struct UIBatchConfig {
    size_t maxVertices = 65536;      // Máximo de vértices por batch
    size_t maxIndices = 131072;      // Máximo de índices por batch
    size_t maxTextures = 8;          // Máximo de texturas por batch
    bool enableScissor = true;       // Habilitar clipping
    bool enableDepthTest = false;    // Teste de profundidade para UI
    bool enableBlending = true;      // Blending para transparência
};

// Estatísticas de renderização
struct UIBatchStats {
    size_t drawCalls = 0;
    size_t verticesRendered = 0;
    size_t indicesRendered = 0;
    size_t batchesCreated = 0;
    size_t textureSwitches = 0;
    
    void Reset() {
        drawCalls = 0;
        verticesRendered = 0;
        indicesRendered = 0;
        batchesCreated = 0;
        textureSwitches = 0;
    }
};

// Interface para batching de UI otimizada e profissional
class IUIBatcher {
public:
    virtual ~IUIBatcher() = default;
    
    // === Gerenciamento de ciclo de vida ===
    virtual void Begin() = 0;                    // Inicia um novo frame de UI
    virtual void End() = 0;                      // Finaliza e renderiza todos os batches
    
    // === Primitivas básicas ===
    virtual void AddRect(float x, float y, float w, float h, Drift::Color color) = 0;
    virtual void AddQuad(float x0, float y0, float x1, float y1,
                         float x2, float y2, float x3, float y3,
                         Drift::Color color) = 0;
    virtual void AddQuad(const glm::mat4& transform, float w, float h, Drift::Color color) {
        glm::vec4 p0 = transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 p1 = transform * glm::vec4(w, 0.0f, 0.0f, 1.0f);
        glm::vec4 p2 = transform * glm::vec4(w, h, 0.0f, 1.0f);
        glm::vec4 p3 = transform * glm::vec4(0.0f, h, 0.0f, 1.0f);
        AddQuad(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color);
    }
    
    // === Renderização de texto ===
    virtual void AddText(float x, float y, const char* text, Drift::Color color) = 0;
    virtual void AddText(const std::string& text, float x, float y, Drift::Color color) {
        AddText(x, y, text.c_str(), color);
    }
    
    // === Gerenciamento de texturas ===
    virtual void SetTexture(uint32_t textureId, ITexture* texture) = 0;
    virtual void ClearTextures() = 0;
    
    // === Sistema de clipping ===
    virtual void PushScissorRect(float x, float y, float w, float h) = 0;
    virtual void PopScissorRect() = 0;
    virtual void ClearScissorRects() = 0;
    virtual ScissorRect GetCurrentScissorRect() const = 0;
    
    // === Configuração e estatísticas ===
    virtual void SetScreenSize(float w, float h) = 0;
    virtual void SetBatchConfig(const UIBatchConfig& config) = 0;
    virtual UIBatchConfig GetBatchConfig() const = 0;
    virtual UIBatchStats GetStats() const = 0;
    virtual void ResetStats() = 0;
    
    // === Otimizações avançadas ===
    virtual void FlushBatch() = 0;               // Força renderização do batch atual
    virtual void SetBlendMode(uint32_t srcFactor, uint32_t dstFactor) = 0;
    virtual void SetDepthTest(bool enabled) = 0;
    virtual void SetViewport(float x, float y, float w, float h) = 0;
    
    // === Cache de geometria ===
    virtual uint32_t CreateGeometryCache() = 0;
    virtual void DestroyGeometryCache(uint32_t cacheId) = 0;
    virtual void UpdateGeometryCache(uint32_t cacheId, const std::vector<UIVertex>& vertices, 
                                   const std::vector<uint32_t>& indices) = 0;
    virtual void RenderGeometryCache(uint32_t cacheId, float x, float y, Drift::Color color) = 0;
};

} // namespace Drift::RHI 