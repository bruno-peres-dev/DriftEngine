// src/rhi/src/UIBatcherBase.cpp
#include "Drift/RHI/UIBatcherBase.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <chrono>

namespace Drift::RHI {

UIBatcherBase::UIBatcherBase() {
    // Configurar qualidade padrão
    m_QualityConfig = UIBatchQualityConfig::GetConfigForQuality(
        UIBatchQualityConfig::QualityLevel::High
    );
    
    // Configurar batch padrão
    m_BatchConfig.maxTextures = m_QualityConfig.qualityLevel == UIBatchQualityConfig::QualityLevel::Ultra ? 16 : 8;
    m_BatchConfig.maxVertices = static_cast<size_t>(m_QualityConfig.vertexPoolSize);
    m_BatchConfig.maxIndices = static_cast<size_t>(m_QualityConfig.indexPoolSize);
}

void UIBatcherBase::SetScreenSize(float w, float h) {
    m_ScreenW = w;
    m_ScreenH = h;
    Core::Log("[UIBatcherBase] Tamanho da tela definido: " + std::to_string(w) + "x" + std::to_string(h));
}

void UIBatcherBase::SetBatchConfig(const UIBatchConfig& config) {
    m_BatchConfig = config;
    Core::Log("[UIBatcherBase] Configuração de batch atualizada");
}

void UIBatcherBase::ResetStats() {
    m_Stats.Reset();
    Core::Log("[UIBatcherBase] Estatísticas resetadas");
}

void UIBatcherBase::SetTexture(uint32_t textureId, ITexture* texture) {
    if (textureId >= m_BatchConfig.maxTextures) {
        Core::Log("[UIBatcherBase] AVISO: textureId " + std::to_string(textureId) + 
                  " excede o máximo de " + std::to_string(m_BatchConfig.maxTextures));
        return;
    }
    
    m_Textures[textureId] = texture;
    m_TextureChanged = true;
    
    // Atualizar array de texturas se necessário
    if (m_TextureArray.size() <= textureId) {
        m_TextureArray.resize(textureId + 1, nullptr);
    }
    m_TextureArray[textureId] = texture;
}

void UIBatcherBase::ClearTextures() {
    m_Textures.clear();
    m_TextureArray.clear();
    m_TextureChanged = true;
    Core::Log("[UIBatcherBase] Texturas limpas");
}

void UIBatcherBase::PushScissorRect(float x, float y, float w, float h) {
    ScissorRect newScissor(x, y, w, h);
    
    // Se já existe um scissor, fazer clipping
    if (!m_ScissorStack.empty()) {
        const ScissorRect& current = m_ScissorStack.back();
        newScissor = ClipRectToScissor(newScissor, current);
    }
    
    m_ScissorStack.push_back(newScissor);
}

void UIBatcherBase::PopScissorRect() {
    if (!m_ScissorStack.empty()) {
        m_ScissorStack.pop_back();
    }
}

void UIBatcherBase::ClearScissorRects() {
    m_ScissorStack.clear();
}

ScissorRect UIBatcherBase::GetCurrentScissorRect() const {
    if (m_ScissorStack.empty()) {
        return ScissorRect(0, 0, m_ScreenW, m_ScreenH);
    }
    return m_ScissorStack.back();
}

uint32_t UIBatcherBase::CreateGeometryCache() {
    uint32_t cacheId = m_NextCacheId++;
    m_GeometryCaches[cacheId] = GeometryCache();
    m_GeometryCaches[cacheId].id = cacheId;
    Core::Log("[UIBatcherBase] Cache de geometria criado: " + std::to_string(cacheId));
    return cacheId;
}

void UIBatcherBase::DestroyGeometryCache(uint32_t cacheId) {
    auto it = m_GeometryCaches.find(cacheId);
    if (it != m_GeometryCaches.end()) {
        m_GeometryCaches.erase(it);
        Core::Log("[UIBatcherBase] Cache de geometria destruído: " + std::to_string(cacheId));
    }
}

void UIBatcherBase::UpdateGeometryCache(uint32_t cacheId, const std::vector<UIVertex>& vertices, 
                                       const std::vector<uint32_t>& indices) {
    auto it = m_GeometryCaches.find(cacheId);
    if (it != m_GeometryCaches.end()) {
        it->second.vertices = vertices;
        it->second.indices = indices;
        it->second.dirty = true;
        it->second.lastUsed = m_Stats.drawCalls;
        
        // Calcular bounding box
        if (!vertices.empty()) {
            float minX = vertices[0].x, maxX = vertices[0].x;
            float minY = vertices[0].y, maxY = vertices[0].y;
            
            for (const auto& v : vertices) {
                minX = std::min(minX, v.x);
                maxX = std::max(maxX, v.x);
                minY = std::min(minY, v.y);
                maxY = std::max(maxY, v.y);
            }
            
            it->second.boundingBox[0] = minX;
            it->second.boundingBox[1] = minY;
            it->second.boundingBox[2] = maxX - minX;
            it->second.boundingBox[3] = maxY - minY;
        }
    }
}

void UIBatcherBase::RenderGeometryCache(uint32_t cacheId, float x, float y, Drift::Color color) {
    auto it = m_GeometryCaches.find(cacheId);
    if (it != m_GeometryCaches.end()) {
        UpdateGeometryCacheUsage(cacheId);
        
        // Renderizar geometria (implementação específica do backend)
        // Por enquanto, apenas log
        Core::Log("[UIBatcherBase] Renderizando cache de geometria: " + std::to_string(cacheId));
    }
}

void UIBatcherBase::SetQualityConfig(const UIBatchQualityConfig& config) {
    m_QualityConfig = config;
    
    // Atualizar configurações de batch baseadas na qualidade
    m_BatchConfig.maxTextures = config.qualityLevel == UIBatchQualityConfig::QualityLevel::Ultra ? 16 : 8;
    m_BatchConfig.maxVertices = static_cast<size_t>(config.vertexPoolSize);
    m_BatchConfig.maxIndices = static_cast<size_t>(config.indexPoolSize);
    
    Core::Log("[UIBatcherBase] Configuração de qualidade atualizada para nível: " + 
              std::to_string(static_cast<int>(config.qualityLevel)));
}

void UIBatcherBase::AddInstancedRect(float x, float y, float w, float h, Drift::Color color, size_t instanceCount) {
    if (!m_BatchConfig.enableInstancing) {
        // Fallback para renderização normal
        for (size_t i = 0; i < instanceCount; ++i) {
            AddRect(x, y, w, h, color);
        }
        return;
    }
    
    // Implementação específica do backend
    Core::Log("[UIBatcherBase] Instanced rect: " + std::to_string(instanceCount) + " instâncias");
}

void UIBatcherBase::AddInstancedTexturedRect(float x, float y, float w, float h,
                                            const glm::vec2& uvMin, const glm::vec2& uvMax,
                                            Drift::Color color, uint32_t textureId, size_t instanceCount) {
    if (!m_BatchConfig.enableInstancing) {
        // Fallback para renderização normal
        for (size_t i = 0; i < instanceCount; ++i) {
            AddTexturedRect(x, y, w, h, uvMin, uvMax, color, textureId);
        }
        return;
    }
    
    // Implementação específica do backend
    Core::Log("[UIBatcherBase] Instanced textured rect: " + std::to_string(instanceCount) + " instâncias");
}

void UIBatcherBase::TrimGeometryCache() {
    if (!m_QualityConfig.enableLRUCache) {
        return;
    }
    
    TrimGeometryCacheInternal();
}

void UIBatcherBase::TrimTextureCache() {
    if (!m_QualityConfig.enableLRUCache) {
        return;
    }
    
    TrimTextureCacheInternal();
}

void UIBatcherBase::AutoDetectQuality() {
    // Detectar recursos do sistema
    bool hasAnisotropic = DetectAnisotropicFiltering();
    bool hasMSAA = DetectMSAA();
    uint32_t maxAnisotropy = DetectMaxAnisotropy();
    size_t maxTextureUnits = DetectMaxTextureUnits();
    size_t maxVertexAttributes = DetectMaxVertexAttributes();
    
    // Determinar nível de qualidade baseado nos recursos
    UIBatchQualityConfig::QualityLevel level = UIBatchQualityConfig::QualityLevel::Medium;
    
    if (maxTextureUnits >= 16 && maxAnisotropy >= 16 && maxVertexAttributes >= 16) {
        level = UIBatchQualityConfig::QualityLevel::Ultra;
    } else if (maxTextureUnits >= 8 && maxAnisotropy >= 8 && maxVertexAttributes >= 8) {
        level = UIBatchQualityConfig::QualityLevel::High;
    } else if (maxTextureUnits >= 4 && maxAnisotropy >= 4 && maxVertexAttributes >= 4) {
        level = UIBatchQualityConfig::QualityLevel::Medium;
    } else {
        level = UIBatchQualityConfig::QualityLevel::Low;
    }
    
    SetQualityConfig(UIBatchQualityConfig::GetConfigForQuality(level));
    
    Core::Log("[UIBatcherBase] Qualidade auto-detectada: " + std::to_string(static_cast<int>(level)) +
              " (Anisotropic: " + std::to_string(hasAnisotropic) + 
              ", MSAA: " + std::to_string(hasMSAA) + 
              ", MaxAnisotropy: " + std::to_string(maxAnisotropy) + ")");
}

bool UIBatcherBase::SupportsFeature(const std::string& feature) const {
    if (feature == "instancing") {
        return m_BatchConfig.enableInstancing;
    } else if (feature == "frustum_culling") {
        return m_BatchConfig.enableFrustumCulling;
    } else if (feature == "occlusion_culling") {
        return m_BatchConfig.enableOcclusionCulling;
    } else if (feature == "command_buffering") {
        return m_BatchConfig.enableCommandBuffering;
    } else if (feature == "anisotropic_filtering") {
        return m_BatchConfig.enableAnisotropicFiltering;
    } else if (feature == "lru_cache") {
        return m_QualityConfig.enableLRUCache;
    }
    
    return false;
}

void UIBatcherBase::UpdateStats(const UIBatchStats& additionalStats) {
    m_Stats.drawCalls += additionalStats.drawCalls;
    m_Stats.verticesRendered += additionalStats.verticesRendered;
    m_Stats.indicesRendered += additionalStats.indicesRendered;
    m_Stats.batchesCreated += additionalStats.batchesCreated;
    m_Stats.textureSwitches += additionalStats.textureSwitches;
    m_Stats.culledElements += additionalStats.culledElements;
    m_Stats.instancedDraws += additionalStats.instancedDraws;
    m_Stats.cacheHits += additionalStats.cacheHits;
    m_Stats.cacheMisses += additionalStats.cacheMisses;
    m_Stats.vertexPoolAllocations += additionalStats.vertexPoolAllocations;
    m_Stats.commandBufferSize += additionalStats.commandBufferSize;
}

void UIBatcherBase::ResetBatchStats() {
    m_Stats.Reset();
}

bool UIBatcherBase::IsRectVisible(const ScissorRect& rect) const {
    if (!m_BatchConfig.enableFrustumCulling) {
        return true;
    }
    
    // Verificar se o retângulo está dentro da tela
    if (rect.x + rect.width < 0 || rect.x > m_ScreenW ||
        rect.y + rect.height < 0 || rect.y > m_ScreenH) {
        return false;
    }
    
    // Verificar clipping com scissor atual
    if (!m_ScissorStack.empty()) {
        const ScissorRect& scissor = m_ScissorStack.back();
        ScissorRect clipped = ClipRectToScissor(rect, scissor);
        return clipped.width > 0 && clipped.height > 0;
    }
    
    return true;
}

ScissorRect UIBatcherBase::ClipRectToScissor(const ScissorRect& rect, const ScissorRect& scissor) const {
    float x1 = std::max(rect.x, scissor.x);
    float y1 = std::max(rect.y, scissor.y);
    float x2 = std::min(rect.x + rect.width, scissor.x + scissor.width);
    float y2 = std::min(rect.y + rect.height, scissor.y + scissor.height);
    
    if (x2 <= x1 || y2 <= y1) {
        return ScissorRect(0, 0, 0, 0); // Retângulo vazio
    }
    
    return ScissorRect(x1, y1, x2 - x1, y2 - y1);
}

void UIBatcherBase::UpdateGeometryCacheUsage(uint32_t cacheId) {
    auto it = m_GeometryCaches.find(cacheId);
    if (it != m_GeometryCaches.end()) {
        it->second.lastUsed = m_Stats.drawCalls;
        it->second.usageCount++;
    }
}

void UIBatcherBase::TrimGeometryCacheInternal() {
    if (m_GeometryCaches.size() <= m_QualityConfig.geometryCacheSize) {
        return;
    }
    
    // Ordenar por uso (LRU)
    std::vector<std::pair<uint32_t, size_t>> usageOrder;
    for (const auto& pair : m_GeometryCaches) {
        usageOrder.emplace_back(pair.first, pair.second.lastUsed);
    }
    
    std::sort(usageOrder.begin(), usageOrder.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    
    // Remover caches menos usados
    size_t toRemove = m_GeometryCaches.size() - m_QualityConfig.geometryCacheSize;
    for (size_t i = 0; i < toRemove; ++i) {
        m_GeometryCaches.erase(usageOrder[i].first);
    }
    
    Core::Log("[UIBatcherBase] Cache de geometria trimado: " + std::to_string(toRemove) + " entradas removidas");
}

void UIBatcherBase::TrimTextureCacheInternal() {
    // Implementação básica - em uma implementação real,
    // seria necessário gerenciar o cache de texturas
    Core::Log("[UIBatcherBase] Cache de texturas trimado");
}

} // namespace Drift::RHI 