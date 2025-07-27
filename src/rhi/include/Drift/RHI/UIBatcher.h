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

/**
 * @brief Estrutura para vértices de UI otimizada e genérica
 */
struct UIVertex {
    float x, y;           // Posição em coordenadas de tela
    float u, v;           // Coordenadas UV (para texturas)
    Drift::Color color;   // Cor RGBA
    uint32_t textureId;   // ID da textura (8 = sem textura, 0-7 = slots de textura)
    float offsetX, offsetY; // Offset adicional para texto
    float scale;            // Escala para texto
    float rotation;         // Rotação para texto

    UIVertex()
        : x(0), y(0), u(0), v(0), color(0xFFFFFFFF), textureId(8),
          offsetX(0.0f), offsetY(0.0f), scale(1.0f), rotation(0.0f) {}
    UIVertex(float x, float y, float u, float v, Drift::Color color,
             uint32_t textureId = 8,
             float offsetX = 0.0f, float offsetY = 0.0f,
             float scale = 1.0f, float rotation = 0.0f)
        : x(x), y(y), u(u), v(v), color(color), textureId(textureId),
          offsetX(offsetX), offsetY(offsetY), scale(scale), rotation(rotation) {}
};

/**
 * @brief Configurações de batching genéricas
 */
struct UIBatchConfig {
    size_t maxVertices = 65536;      // Máximo de vértices por batch
    size_t maxIndices = 131072;      // Máximo de índices por batch
    size_t maxTextures = 16;         // Máximo de texturas por batch (aumentado)
    bool enableScissor = true;       // Habilitar clipping
    bool enableDepthTest = false;    // Teste de profundidade para UI
    bool enableBlending = true;      // Blending para transparência
    
    // Configurações avançadas genéricas
    bool enableFrustumCulling = true;      // Culling de elementos fora da tela
    bool enableOcclusionCulling = true;    // Culling de elementos sobrepostos
    bool enableInstancing = true;          // Instancing para elementos repetidos
    bool enableCommandBuffering = true;    // Buffer de comandos para multi-threading
    bool enableAnisotropicFiltering = true; // Filtro anisotrópico para texturas
    uint32_t maxLODLevels = 3;             // Níveis de LOD para UI
};

/**
 * @brief Estatísticas de renderização genéricas
 */
struct UIBatchStats {
    size_t drawCalls = 0;
    size_t verticesRendered = 0;
    size_t indicesRendered = 0;
    size_t batchesCreated = 0;
    size_t textureSwitches = 0;
    
    // Estatísticas avançadas
    size_t culledElements = 0;
    size_t instancedDraws = 0;
    size_t cacheHits = 0;
    size_t cacheMisses = 0;
    size_t vertexPoolAllocations = 0;
    size_t commandBufferSize = 0;
    
    void Reset() {
        drawCalls = 0;
        verticesRendered = 0;
        indicesRendered = 0;
        batchesCreated = 0;
        textureSwitches = 0;
        culledElements = 0;
        instancedDraws = 0;
        cacheHits = 0;
        cacheMisses = 0;
        vertexPoolAllocations = 0;
        commandBufferSize = 0;
    }
};

/**
 * @brief Configurações de qualidade adaptativas
 */
struct UIBatchQualityConfig {
    enum class QualityLevel {
        Low,        // Para dispositivos móveis/baixa performance
        Medium,     // Para desktop padrão
        High,       // Para desktop de alta performance
        Ultra       // Para estações de trabalho
    };
    
    QualityLevel qualityLevel = QualityLevel::High;
    bool enableMSAA = false;               // MSAA para UI (opcional)
    uint32_t maxAnisotropy = 16;           // Máximo de anisotropia
    size_t vertexPoolSize = 262144;        // Tamanho do pool de vértices
    size_t indexPoolSize = 524288;         // Tamanho do pool de índices
    size_t maxBatchesPerFrame = 1024;      // Máximo de batches por frame
    size_t geometryCacheSize = 1000;       // Cache de geometria
    size_t textureCacheSize = 500;         // Cache de texturas
    bool enableLRUCache = true;            // Cache LRU para recursos
    
    // Configurações automáticas baseadas no nível de qualidade
    static UIBatchQualityConfig GetConfigForQuality(QualityLevel level) {
        UIBatchQualityConfig config;
        config.qualityLevel = level;
        
        switch (level) {
            case QualityLevel::Low:
                config.enableMSAA = false;
                config.maxAnisotropy = 1;
                config.vertexPoolSize = 65536;
                config.indexPoolSize = 131072;
                config.maxBatchesPerFrame = 256;
                config.geometryCacheSize = 100;
                config.textureCacheSize = 50;
                config.enableLRUCache = false;
                break;
                
            case QualityLevel::Medium:
                config.enableMSAA = false;
                config.maxAnisotropy = 4;
                config.vertexPoolSize = 131072;
                config.indexPoolSize = 262144;
                config.maxBatchesPerFrame = 512;
                config.geometryCacheSize = 500;
                config.textureCacheSize = 250;
                config.enableLRUCache = true;
                break;
                
            case QualityLevel::High:
                config.enableMSAA = false;
                config.maxAnisotropy = 8;
                config.vertexPoolSize = 262144;
                config.indexPoolSize = 524288;
                config.maxBatchesPerFrame = 1024;
                config.geometryCacheSize = 1000;
                config.textureCacheSize = 500;
                config.enableLRUCache = true;
                break;
                
            case QualityLevel::Ultra:
                config.enableMSAA = true;
                config.maxAnisotropy = 16;
                config.vertexPoolSize = 524288;
                config.indexPoolSize = 1048576;
                config.maxBatchesPerFrame = 2048;
                config.geometryCacheSize = 2000;
                config.textureCacheSize = 1000;
                config.enableLRUCache = true;
                break;
        }
        
        return config;
    }
};

/**
 * @brief Interface para batching de UI otimizada e profissional
 * Preparada para múltiplos backends (DX11, Vulkan, etc.)
 */
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

    // Adiciona um retângulo texturizado especificando UVs e id de textura
    virtual void AddTexturedRect(float x, float y, float w, float h,
                                 const glm::vec2& uvMin, const glm::vec2& uvMax,
                                 Drift::Color color, uint32_t textureId) = 0;
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
    
    // === Controle de renderização de texto ===
    virtual void BeginText() = 0;  // Marca início de renderização de texto
    virtual void EndText() = 0;    // Marca fim de renderização de texto
    
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
    
    // === Novas funcionalidades AAA genéricas ===
    virtual void SetQualityConfig(const UIBatchQualityConfig& config) = 0;
    virtual UIBatchQualityConfig GetQualityConfig() const = 0;
    
    // Instancing para elementos repetidos
    virtual void AddInstancedRect(float x, float y, float w, float h, Drift::Color color, size_t instanceCount) = 0;
    virtual void AddInstancedTexturedRect(float x, float y, float w, float h,
                                         const glm::vec2& uvMin, const glm::vec2& uvMax,
                                         Drift::Color color, uint32_t textureId, size_t instanceCount) = 0;
    
    // Otimizações de cache
    virtual void TrimGeometryCache() = 0;
    virtual void TrimTextureCache() = 0;
    
    // Detecção automática de qualidade
    virtual void AutoDetectQuality() = 0;
    
    // Informações do backend
    virtual std::string GetBackendName() const = 0;
    virtual bool SupportsFeature(const std::string& feature) const = 0;
};

} // namespace Drift::RHI 