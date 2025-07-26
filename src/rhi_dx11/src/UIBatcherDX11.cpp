#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Device.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/DX11/SamplerDX11.h"
#include "Drift/RHI/Texture.h"
#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/UI/FontSystem/FontSystem.h"
#include <glm/vec2.hpp>
#include <cstring>
#include <algorithm>
#include <wrl/client.h>
#include <d3d11.h>

using namespace Drift::RHI::DX11;
using namespace Drift::RHI;

// Conversão ARGB para BGRA otimizada (inline para performance)
inline Drift::Color ConvertARGBtoBGRA(Drift::Color argb) {
    uint8_t a = (argb >> 24) & 0xFF;
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >> 8) & 0xFF;
    uint8_t b = argb & 0xFF;
    
    return (a << 24) | (b << 16) | (g << 8) | r;
}

// Construtor otimizado
UIBatcherDX11::UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx)
    : m_RingBuffer(std::move(ringBuffer)), m_Context(ctx) {
    
    // Inicializar configurações padrão
    m_BatchConfig.maxVertices = 65536;
    m_BatchConfig.maxIndices = 131072;
    m_BatchConfig.maxTextures = 8;
    m_BatchConfig.enableScissor = true;
    m_BatchConfig.enableDepthTest = false;
    m_BatchConfig.enableBlending = true;
    
    // Inicializar estatísticas
    m_Stats.Reset();
    
    // Inicializar sistema de renderização de texto
    m_TextRenderer = std::make_unique<Drift::UI::UIBatcherTextRenderer>(this);
    Core::Log("[UIBatcherDX11] TextRenderer inicializado com batcher: " + std::to_string(this != nullptr));
    
    // Criar sampler padrão para UI
    auto* device = static_cast<ID3D11Device*>(ctx->GetNativeDevice());
    if (device) {
        SamplerDesc samplerDesc{};
        // Criar sampler diretamente usando o device DX11
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sd.MinLOD = 0;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        
        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
        HRESULT hr = device->CreateSamplerState(&sd, samplerState.GetAddressOf());
        if (SUCCEEDED(hr)) {
            m_DefaultSampler = std::make_shared<SamplerDX11>(samplerState.Get());
        }
    }
    
    // Pré-alocar buffers
    m_VertexBuffer.reserve(m_BatchConfig.maxVertices);
    m_IndexBuffer.reserve(m_BatchConfig.maxIndices);
    
    // Criar pipelines
    EnsurePipeline();
    CreateTextPipeline();
    
    Core::Log("[UIBatcherDX11] Inicializado com sucesso");
}

UIBatcherDX11::~UIBatcherDX11() {
    Core::Log("[UIBatcherDX11] Destruindo...");
    
    // Limpar caches de geometria
    m_GeometryCaches.clear();
    
    // Limpar texturas
    m_Textures.clear();
    m_TextureArray.clear();
    
    Core::Log("[UIBatcherDX11] Destruído");
}

void UIBatcherDX11::Begin() {
    // Avançar para o próximo frame no ring buffer
    if (m_RingBuffer) {
        m_RingBuffer->NextFrame();
    }
    
    // Resetar estatísticas do frame
    ResetBatchStats();
    
    // Configurar estado de renderização
    m_Context->SetDepthTestEnabled(m_DepthTestEnabled);
    
    // Limpar batch atual
    m_CurrentBatch.Clear();
    m_BatchDirty = false;
    m_TextureChanged = false;
    
    // Limpar texturas do frame anterior para evitar conflitos
    ClearTextures();
    
    // Iniciar renderização de texto
    if (m_TextRenderer) {
        m_TextRenderer->BeginTextRendering();
    }
}

void UIBatcherDX11::End() {
    try {
        // Finalizar renderização de texto
        if (m_TextRenderer) {
            m_TextRenderer->EndTextRendering();
        }
        
        // Flush do batch atual se necessário
        if (!m_CurrentBatch.IsEmpty()) {
            FlushCurrentBatch();
        }
        
        // Atualizar estatísticas
        UpdateStats(m_CurrentBatch);
                 
    } catch (const std::exception& e) {
        Core::Log("[UIBatcherDX11] ERRO CRÍTICO no End(): " + std::string(e.what()));
    } catch (...) {
        Core::Log("[UIBatcherDX11] ERRO CRÍTICO desconhecido no End()");
    }
}

void UIBatcherDX11::AddRect(float x, float y, float w, float h, Drift::Color color) {
    // Verificar clipping
    ScissorRect currentScissor = GetCurrentScissorRect();
    if (currentScissor.IsValid()) {
        ScissorRect clippedRect = ClipRectToScissor(ScissorRect(x, y, w, h), currentScissor);
        if (!clippedRect.IsValid()) {
            return; // Retângulo completamente fora da área visível
        }
        x = clippedRect.x;
        y = clippedRect.y;
        w = clippedRect.width;
        h = clippedRect.height;
    }
    
    // Garantir que retângulos sólidos fiquem em um batch separado de geometrias texturizadas
    if (m_CurrentBatch.hasTexture) {
        FlushCurrentBatch();
        m_CurrentBatch.hasTexture = false;
    }

    // Verificar se precisa fazer flush do batch por limite de vértices/índices
    if (m_CurrentBatch.vertexCount + 4 > m_BatchConfig.maxVertices ||
        m_CurrentBatch.indexCount + 6 > m_BatchConfig.maxIndices) {
        FlushCurrentBatch();
    }
    
    // Converter cor
    Drift::Color bgra = ConvertARGBtoBGRA(color);
    
    // Adicionar vértices
    uint32_t baseIndex = static_cast<uint32_t>(m_CurrentBatch.vertices.size());
    
    float clipX0 = ToClipX(x);
    float clipY0 = ToClipY(y);
    float clipX1 = ToClipX(x + w);
    float clipY1 = ToClipY(y + h);
    
    m_CurrentBatch.vertices.emplace_back(clipX0, clipY0, 0.0f, 0.0f, bgra, 0);
    m_CurrentBatch.vertices.emplace_back(clipX1, clipY0, 1.0f, 0.0f, bgra, 0);
    m_CurrentBatch.vertices.emplace_back(clipX1, clipY1, 1.0f, 1.0f, bgra, 0);
    m_CurrentBatch.vertices.emplace_back(clipX0, clipY1, 0.0f, 1.0f, bgra, 0);
    
    // Adicionar índices
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    m_CurrentBatch.indices.push_back(baseIndex + 1);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 3);
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    
    m_CurrentBatch.vertexCount += 4;
    m_CurrentBatch.indexCount += 6;
    m_BatchDirty = true;
}

void UIBatcherDX11::AddQuad(float x0, float y0, float x1, float y1,
                            float x2, float y2, float x3, float y3,
                            Drift::Color color) {
    // Verificar clipping básico
    ScissorRect currentScissor = GetCurrentScissorRect();
    if (currentScissor.IsValid()) {
        float minX = (std::min)((std::min)(x0, x1), (std::min)(x2, x3));
        float minY = (std::min)((std::min)(y0, y1), (std::min)(y2, y3));
        float maxX = (std::max)((std::max)(x0, x1), (std::max)(x2, x3));
        float maxY = (std::max)((std::max)(y0, y1), (std::max)(y2, y3));
        
        if (maxX < currentScissor.x || minX > currentScissor.x + currentScissor.width ||
            maxY < currentScissor.y || minY > currentScissor.y + currentScissor.height) {
            return; // Quad completamente fora da área visível
        }
    }
    
    // Garantir que quads sólidos fiquem em um batch separado de geometrias texturizadas
    if (m_CurrentBatch.hasTexture) {
        FlushCurrentBatch();
        m_CurrentBatch.hasTexture = false;
    }

    // Verificar se precisa fazer flush do batch por limite de vértices/índices
    if (m_CurrentBatch.vertexCount + 4 > m_BatchConfig.maxVertices ||
        m_CurrentBatch.indexCount + 6 > m_BatchConfig.maxIndices) {
        FlushCurrentBatch();
    }
    
    // Converter cor
    Drift::Color bgra = ConvertARGBtoBGRA(color);
    
    // Adicionar vértices
    uint32_t baseIndex = static_cast<uint32_t>(m_CurrentBatch.vertices.size());
    
    m_CurrentBatch.vertices.emplace_back(ToClipX(x0), ToClipY(y0), 0.0f, 0.0f, bgra, 0);
    m_CurrentBatch.vertices.emplace_back(ToClipX(x1), ToClipY(y1), 1.0f, 0.0f, bgra, 0);
    m_CurrentBatch.vertices.emplace_back(ToClipX(x2), ToClipY(y2), 1.0f, 1.0f, bgra, 0);
    m_CurrentBatch.vertices.emplace_back(ToClipX(x3), ToClipY(y3), 0.0f, 1.0f, bgra, 0);
    
    // Adicionar índices
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    m_CurrentBatch.indices.push_back(baseIndex + 1);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 3);
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    
    m_CurrentBatch.vertexCount += 4;
    m_CurrentBatch.indexCount += 6;
    m_BatchDirty = true;
}

void UIBatcherDX11::AddTexturedRect(float x, float y, float w, float h,
                                    const glm::vec2& uvMin, const glm::vec2& uvMax,
                                    Drift::Color color, uint32_t textureId) {
    ScissorRect currentScissor = GetCurrentScissorRect();
    if (currentScissor.IsValid()) {
        ScissorRect clipped = ClipRectToScissor(ScissorRect(x, y, w, h), currentScissor);
        if (!clipped.IsValid()) {
            return;
        }
        x = clipped.x;
        y = clipped.y;
        w = clipped.width;
        h = clipped.height;
    }

    if (m_CurrentBatch.vertexCount + 4 > m_BatchConfig.maxVertices ||
        m_CurrentBatch.indexCount + 6 > m_BatchConfig.maxIndices ||
        (m_CurrentBatch.hasTexture && m_CurrentBatch.textureId != textureId)) {
        FlushCurrentBatch();
    }

    m_CurrentBatch.textureId = textureId;
    m_CurrentBatch.hasTexture = true;

    Drift::Color bgra = ConvertARGBtoBGRA(color);

    uint32_t baseIndex = static_cast<uint32_t>(m_CurrentBatch.vertices.size());
    m_CurrentBatch.vertices.emplace_back(ToClipX(x), ToClipY(y), uvMin.x, uvMin.y, bgra, textureId);
    m_CurrentBatch.vertices.emplace_back(ToClipX(x + w), ToClipY(y), uvMax.x, uvMin.y, bgra, textureId);
    m_CurrentBatch.vertices.emplace_back(ToClipX(x + w), ToClipY(y + h), uvMax.x, uvMax.y, bgra, textureId);
    m_CurrentBatch.vertices.emplace_back(ToClipX(x), ToClipY(y + h), uvMin.x, uvMax.y, bgra, textureId);

    m_CurrentBatch.indices.push_back(baseIndex + 0);
    m_CurrentBatch.indices.push_back(baseIndex + 1);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 3);
    m_CurrentBatch.indices.push_back(baseIndex + 0);

    m_CurrentBatch.vertexCount += 4;
    m_CurrentBatch.indexCount += 6;
    m_BatchDirty = true;
}

void UIBatcherDX11::AddText(float x, float y, const char* text, Drift::Color color) {
    
    if (m_TextRenderer) {
        // Converter Drift::Color para glm::vec4
        float r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
        float g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
        float b = static_cast<float>(color & 0xFF) / 255.0f;
        float a = static_cast<float>((color >> 24) & 0xFF) / 255.0f;
        glm::vec4 textColor(r, g, b, a);
        
        m_TextRenderer->AddText(std::string(text), glm::vec2(x, y), "default", 16.0f, textColor);
    } else {
        Core::Log("[UIBatcherDX11] ERRO: m_TextRenderer é nullptr!");
    }
}

void UIBatcherDX11::SetTexture(uint32_t textureId, ITexture* texture) {
    if (m_CurrentTextureId != textureId || m_Textures[textureId] != texture) {
        // Se mudou a textura, fazer flush do batch atual
        if (!m_CurrentBatch.IsEmpty()) {
            FlushCurrentBatch();
        }
        
        m_Textures[textureId] = texture;
        m_CurrentTextureId = textureId;
        m_TextureChanged = true;
        m_Stats.textureSwitches++;
    }
}

void UIBatcherDX11::ClearTextures() {
    if (!m_Textures.empty()) {
        FlushCurrentBatch();
        // Não limpar as texturas completamente, apenas marcar como mudadas
        // para que sejam reconfiguradas no próximo batch
        m_CurrentTextureId = 0;
        m_TextureChanged = true;
    }
}

void UIBatcherDX11::PushScissorRect(float x, float y, float w, float h) {
    ScissorRect newScissor(x, y, w, h);
    
    if (!m_ScissorStack.empty()) {
        // Clippar com o scissor atual
        newScissor = ClipRectToScissor(newScissor, m_ScissorStack.back());
    }
    
    m_ScissorStack.push_back(newScissor);
}

void UIBatcherDX11::PopScissorRect() {
    if (!m_ScissorStack.empty()) {
        m_ScissorStack.pop_back();
    }
}

void UIBatcherDX11::ClearScissorRects() {
    m_ScissorStack.clear();
}

ScissorRect UIBatcherDX11::GetCurrentScissorRect() const {
    if (m_ScissorStack.empty()) {
        return ScissorRect(0, 0, m_ScreenW, m_ScreenH);
    }
    return m_ScissorStack.back();
}

void UIBatcherDX11::SetScreenSize(float w, float h) {
    m_ScreenW = w;
    m_ScreenH = h;
    
    if (m_TextRenderer) {
        m_TextRenderer->SetScreenSize(static_cast<int>(w), static_cast<int>(h));
    }
}

void UIBatcherDX11::SetBatchConfig(const UIBatchConfig& config) {
    m_BatchConfig = config;
    
    // Redimensionar buffers se necessário
    m_VertexBuffer.reserve(config.maxVertices);
    m_IndexBuffer.reserve(config.maxIndices);
}

void UIBatcherDX11::ResetStats() {
    m_Stats.Reset();
}

void UIBatcherDX11::FlushBatch() {
    FlushCurrentBatch();
}

void UIBatcherDX11::SetBlendMode(uint32_t srcFactor, uint32_t dstFactor) {
    m_SrcBlendFactor = srcFactor;
    m_DstBlendFactor = dstFactor;
    // Nota: Implementação real dependeria da API gráfica específica
}

void UIBatcherDX11::SetDepthTest(bool enabled) {
    m_DepthTestEnabled = enabled;
}

void UIBatcherDX11::SetViewport(float x, float y, float w, float h) {
    // Implementação específica da API gráfica
    // Por enquanto, apenas atualizar as dimensões da tela
    m_ScreenW = w;
    m_ScreenH = h;
}

uint32_t UIBatcherDX11::CreateGeometryCache() {
    uint32_t cacheId = m_NextCacheId++;
    m_GeometryCaches[cacheId] = GeometryCache();
    m_GeometryCaches[cacheId].id = cacheId;
    return cacheId;
}

void UIBatcherDX11::DestroyGeometryCache(uint32_t cacheId) {
    m_GeometryCaches.erase(cacheId);
}

void UIBatcherDX11::UpdateGeometryCache(uint32_t cacheId, const std::vector<UIVertex>& vertices, 
                                       const std::vector<uint32_t>& indices) {
    auto it = m_GeometryCaches.find(cacheId);
    if (it != m_GeometryCaches.end()) {
        it->second.vertices = vertices;
        it->second.indices = indices;
        it->second.dirty = true;
        it->second.lastUsed = m_Stats.drawCalls;
    }
}

void UIBatcherDX11::RenderGeometryCache(uint32_t cacheId, float x, float y, Drift::Color color) {
    auto it = m_GeometryCaches.find(cacheId);
    if (it != m_GeometryCaches.end() && !it->second.vertices.empty()) {
        // Implementação simplificada - renderizar como quads
        // Em uma implementação real, seria renderizado diretamente
        for (size_t i = 0; i < it->second.vertices.size(); i += 4) {
            if (i + 3 < it->second.vertices.size()) {
                const auto& v0 = it->second.vertices[i];
                const auto& v1 = it->second.vertices[i + 1];
                const auto& v2 = it->second.vertices[i + 2];
                const auto& v3 = it->second.vertices[i + 3];
                
                AddQuad(x + v0.x, y + v0.y, x + v1.x, y + v1.y,
                       x + v2.x, y + v2.y, x + v3.x, y + v3.y, color);
            }
        }
        it->second.lastUsed = m_Stats.drawCalls;
    }
}

// Métodos auxiliares privados
void UIBatcherDX11::EnsurePipeline() {
    if (m_Pipeline) {
        return;
    }
    
    // Implementação simplificada - em uma implementação real,
    // seria criado um pipeline state específico para UI
    m_Pipeline = nullptr; // Placeholder
}

void UIBatcherDX11::CreateTextPipeline() {
    m_TextPipeline = m_Pipeline; // Por enquanto, usar o mesmo pipeline
}

void UIBatcherDX11::FlushCurrentBatch() {
    if (m_CurrentBatch.IsEmpty()) {
        return;
    }
    
    RenderBatch(m_CurrentBatch);
    m_CurrentBatch.Clear();
    m_BatchDirty = false;
}

void UIBatcherDX11::RenderBatch(const UIBatch& batch) {
    try {
        if (batch.IsEmpty() || !m_RingBuffer) {
            return;
        }
    
    // Calcular tamanhos dos buffers
    size_t vtxSize = batch.vertices.size() * sizeof(UIVertex);
    size_t idxSize = batch.indices.size() * sizeof(uint32_t);
    
    // Alocar no ring buffer
    size_t vtxOffset, idxOffset;
    void* vtxPtr = m_RingBuffer->Allocate(vtxSize, 16, vtxOffset);
    void* idxPtr = m_RingBuffer->Allocate(idxSize, 4, idxOffset);
    
    if (!vtxPtr || !idxPtr) {
        Core::Log("[UIBatcherDX11] ERRO: Falha ao alocar memória no ring buffer!");
        return;
    }
    
    // Copiar dados
    memcpy(vtxPtr, batch.vertices.data(), vtxSize);
    memcpy(idxPtr, batch.indices.data(), idxSize);
    
    // Obter contexto DX11
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (!contextDX11) {
        Core::Log("[UIBatcherDX11] ERRO: Contexto DX11 inválido!");
        return;
    }
    
    // Configurar pipeline UI se necessário
    EnsureUIPipeline();
    
    // Aplicar pipeline UI
    if (m_Pipeline) {
        m_Pipeline->Apply(*contextDX11);
    } else {
        Core::Log("[UIBatcherDX11] ERRO: Pipeline UI é nullptr!");
        return;
    }
    
    // Configurar vertex buffer
    auto* vertexBuffer = m_RingBuffer->GetBuffer();
    if (vertexBuffer) {
        contextDX11->IASetVertexBuffer(vertexBuffer->GetBackendHandle(), sizeof(UIVertex), static_cast<UINT>(vtxOffset));
    } else {
        Core::Log("[UIBatcherDX11] ERRO: Vertex buffer é nullptr!");
        return;
    }
    
    // Configurar index buffer
    auto* indexBuffer = m_RingBuffer->GetBuffer();
    if (indexBuffer) {
        contextDX11->IASetIndexBuffer(indexBuffer->GetBackendHandle(), Drift::RHI::Format::R32_UINT, static_cast<UINT>(idxOffset));
    } else {
        Core::Log("[UIBatcherDX11] ERRO: Index buffer é nullptr!");
        return;
    }
    
    // Configurar topologia
    contextDX11->IASetPrimitiveTopology(Drift::RHI::PrimitiveTopology::TriangleList);
    
    // Configurar todas as texturas necessárias para o array de texturas
    for (size_t i = 0; i < m_Textures.size() && i < 8; ++i) {
        if (m_Textures[i]) {
            contextDX11->PSSetTexture(static_cast<UINT>(i), m_Textures[i]);
            if (m_DefaultSampler) {
                contextDX11->PSSetSampler(static_cast<UINT>(i), m_DefaultSampler.get());
            }
        }
    }
    
    // Desabilitar depth test para UI
    contextDX11->SetDepthTestEnabled(false);
    
    // Renderizar usando DrawIndexed
    contextDX11->DrawIndexed(
        static_cast<UINT>(batch.indexCount),
        static_cast<UINT>(0), // startIndex
        static_cast<INT>(0)   // baseVertex
    );
    
    // Atualizar estatísticas
    m_Stats.drawCalls++;
    m_Stats.verticesRendered += batch.vertexCount;
    m_Stats.indicesRendered += batch.indexCount;
    m_Stats.batchesCreated++;
    } catch (const std::exception& e) {
        Core::Log("[UIBatcherDX11] ERRO CRÍTICO no RenderBatch: " + std::string(e.what()));
    } catch (...) {
        Core::Log("[UIBatcherDX11] ERRO CRÍTICO desconhecido no RenderBatch");
    }
}

bool UIBatcherDX11::IsRectVisible(const ScissorRect& rect) const {
    ScissorRect currentScissor = GetCurrentScissorRect();
    return rect.Intersects(currentScissor);
}

ScissorRect UIBatcherDX11::ClipRectToScissor(const ScissorRect& rect, const ScissorRect& scissor) const {
    return rect.Clip(scissor);
}

void UIBatcherDX11::UpdateStats(const UIBatch& batch) {
    // Estatísticas já são atualizadas em RenderBatch
}

void UIBatcherDX11::ResetBatchStats() {
    // Resetar apenas estatísticas do frame atual
    m_Stats.drawCalls = 0;
    m_Stats.verticesRendered = 0;
    m_Stats.indicesRendered = 0;
    m_Stats.batchesCreated = 0;
    m_Stats.textureSwitches = 0;
}

// Fábrica para criar UIBatcherDX11
std::unique_ptr<IUIBatcher> Drift::RHI::DX11::CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx) {
    return std::make_unique<UIBatcherDX11>(std::move(ringBuffer), ctx);
} 

void UIBatcherDX11::EnsureUIPipeline() {
    if (m_Pipeline) {
        return;
    }
    
    // Criar descrição do pipeline UI
    PipelineDesc uiDesc;
    uiDesc.vsFile = "shaders/UIBatch.hlsl";
    uiDesc.vsEntry = "VSMain";
    uiDesc.psFile = "shaders/UIBatch.hlsl";
    uiDesc.psEntry = "PSMain";
    
    Core::Log("[UIBatcherDX11] Configurando pipeline UI com shader: " + uiDesc.vsFile);
    
    // Configurar input layout para UIVertex
    uiDesc.inputLayout = {
        {"POSITION", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, x)},
        {"TEXCOORD", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, u)},
        {"COLOR", 0, VertexFormat::R8G8B8A8_UNORM, offsetof(UIVertex, color)},
        {"TEXCOORD", 1, VertexFormat::R32_UINT, offsetof(UIVertex, textureId)}
    };
    
    Core::Log("[UIBatcherDX11] Input layout configurado com " + std::to_string(uiDesc.inputLayout.size()) + " elementos");
    
    // Configurar rasterizer state
    uiDesc.rasterizer.wireframe = false;
    uiDesc.rasterizer.cullMode = PipelineDesc::RasterizerDesc::CullMode::None;
    
    // Configurar blend state para UI com transparência correta
    uiDesc.blend.enable = true;
    uiDesc.blend.srcColor = PipelineDesc::BlendDesc::BlendFactor::SrcAlpha;
    uiDesc.blend.dstColor = PipelineDesc::BlendDesc::BlendFactor::InvSrcAlpha;
    uiDesc.blend.colorOp = PipelineDesc::BlendDesc::BlendOp::Add;
    uiDesc.blend.srcAlpha = PipelineDesc::BlendDesc::BlendFactor::One;
    uiDesc.blend.dstAlpha = PipelineDesc::BlendDesc::BlendFactor::InvSrcAlpha;
    uiDesc.blend.alphaOp = PipelineDesc::BlendDesc::BlendOp::Add;
    uiDesc.blend.blendFactorSeparate = true;
    uiDesc.blend.alphaToCoverage = false;
    
    // Configurar depth stencil state
    uiDesc.depthStencil.depthEnable = false;
    uiDesc.depthStencil.depthWrite = false;
    
    // Criar pipeline
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (contextDX11) {
        auto* device = static_cast<ID3D11Device*>(contextDX11->GetNativeDevice());
        if (device) {
            Core::Log("[UIBatcherDX11] Criando pipeline UI...");
            m_Pipeline = CreatePipelineDX11(device, uiDesc);
            if (m_Pipeline) {
                Core::Log("[UIBatcherDX11] Pipeline UI criado com sucesso");
            } else {
                Core::Log("[UIBatcherDX11] ERRO: Falha ao criar pipeline UI!");
            }
        } else {
            Core::Log("[UIBatcherDX11] ERRO: Device DX11 é nullptr!");
        }
    } else {
        Core::Log("[UIBatcherDX11] ERRO: Context DX11 é nullptr!");
    }
}

 