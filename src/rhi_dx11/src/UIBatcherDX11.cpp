// src/rhi_dx11/src/UIBatcherDX11.cpp
#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include "Drift/RHI/DX11/SamplerDX11.h"
#include "Drift/RHI/DX11/BufferDX11.h"
#include "Drift/UI/FontSystem/FontRendering.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <d3d11.h>
#include <algorithm>
#include <chrono>
#include <thread>

namespace Drift::RHI::DX11 {

// Implementação do VertexPool
VertexPool::VertexPool(size_t capacity) : m_Capacity(capacity), m_Used(0) {
    m_Vertices.resize(capacity);
}

VertexPool::~VertexPool() = default;

UIVertex* VertexPool::Allocate(size_t count) {
    if (m_Used + count > m_Capacity) {
        return nullptr; // Pool cheio
    }
    
    UIVertex* ptr = &m_Vertices[m_Used];
    m_Used += count;
    return ptr;
}

void VertexPool::Reset() {
    m_Used = 0;
}

// Implementação do UICullingSystem
void UICullingSystem::SetViewport(float x, float y, float w, float h) {
    m_Viewport[0] = x;
    m_Viewport[1] = y;
    m_Viewport[2] = w;
    m_Viewport[3] = h;
}

bool UICullingSystem::IsVisible(const float* boundingBox) const {
    if (!m_EnableFrustumCulling) {
        return true;
    }
    
    // Verificar se o retângulo está dentro da viewport
    if (boundingBox[0] + boundingBox[2] < m_Viewport[0] || 
        boundingBox[0] > m_Viewport[0] + m_Viewport[2] ||
        boundingBox[1] + boundingBox[3] < m_Viewport[1] || 
        boundingBox[1] > m_Viewport[1] + m_Viewport[3]) {
        return false;
    }
    
    return true;
}

// Implementação do UIBatcherDX11
UIBatcherDX11::UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx)
    : UIBatcherBase()
    , m_RingBuffer(std::move(ringBuffer))
    , m_Context(ctx)
    , m_AddingText(false)
    , m_BatchDirty(false) {
    
    // Auto-detect quality para DX11
    AutoDetectQuality();
    
    // Inicializar pools e sistemas
    m_VertexPool = std::make_unique<VertexPool>(m_QualityConfig.vertexPoolSize);
    m_CullingSystem = std::make_unique<UICullingSystem>();
    
    // Configurar culling
    m_CullingSystem->EnableFrustumCulling(m_BatchConfig.enableFrustumCulling);
    m_CullingSystem->EnableOcclusionCulling(m_BatchConfig.enableOcclusionCulling);
    
    // Pré-alocar command buffer
    m_CommandBuffer.reserve(m_QualityConfig.maxBatchesPerFrame);
    
    // Criar sampler padrão com configurações AAA
    CreateDefaultSampler();
    
    // Criar constant buffer UI
    CreateUIConstantsBuffer();
    
    // Criar pipeline de instancing
    CreateInstancedPipeline();
    
    // Criar pipeline de texto
    CreateTextPipeline();
    
    // Inicializar text renderer
    m_TextRenderer = std::make_unique<Drift::UI::FontRendering>(nullptr);
    
    Core::Log("[UIBatcherDX11] Inicializado com sucesso");
}

UIBatcherDX11::~UIBatcherDX11() = default;

// === Implementação dos métodos da interface ===

void UIBatcherDX11::Begin() {
    OnBegin();
}

void UIBatcherDX11::End() {
    OnEnd();
}

void UIBatcherDX11::AddRect(float x, float y, float w, float h, Drift::Color color) {
    OnAddRect(x, y, w, h, color);
}

void UIBatcherDX11::AddQuad(float x0, float y0, float x1, float y1,
                           float x2, float y2, float x3, float y3,
                           Drift::Color color) {
    OnAddQuad(x0, y0, x1, y1, x2, y2, x3, y3, color);
}

void UIBatcherDX11::AddTexturedRect(float x, float y, float w, float h,
                                   const glm::vec2& uvMin, const glm::vec2& uvMax,
                                   Drift::Color color, uint32_t textureId) {
    OnAddTexturedRect(x, y, w, h, uvMin, uvMax, color, textureId);
}

void UIBatcherDX11::AddText(float x, float y, const char* text, Drift::Color color) {
    OnAddText(x, y, text, color);
}

void UIBatcherDX11::BeginText() {
    OnBeginText();
}

void UIBatcherDX11::EndText() {
    OnEndText();
}

void UIBatcherDX11::FlushBatch() {
    OnFlushBatch();
}

void UIBatcherDX11::SetBlendMode(uint32_t srcFactor, uint32_t dstFactor) {
    OnSetBlendMode(srcFactor, dstFactor);
}

void UIBatcherDX11::SetDepthTest(bool enabled) {
    OnSetDepthTest(enabled);
}

void UIBatcherDX11::SetViewport(float x, float y, float w, float h) {
    OnSetViewport(x, y, w, h);
}

// === Implementação dos métodos virtuais da classe base ===

void UIBatcherDX11::OnBegin() {
    // Reset estatísticas
    ResetBatchStats();
    
    // Reset pools
    if (m_VertexPool) {
        m_VertexPool->Reset();
    }
    
    // CRÍTICO: Limpar batch atual
    m_CurrentBatch.Clear();
    m_BatchDirty = false;
    
    // Limpar command buffer
    m_CommandBuffer.clear();
    
    // Atualizar culling system
    if (m_CullingSystem) {
        m_CullingSystem->SetViewport(0, 0, m_ScreenW, m_ScreenH);
    }
    
    // Configurar pipeline - CRÍTICO para renderização
    EnsurePipeline();
    
    if (!m_Pipeline) {
        Core::Log("[UIBatcherDX11] ERRO CRÍTICO: Pipeline não foi criado!");
        return;
    }
    
    // Configurar sampler padrão
    if (m_DefaultSampler) {
        auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
        if (contextDX11) {
            contextDX11->SetSampler(0, m_DefaultSampler.get());
        }
    }
    
    // Configurar text renderer
    if (m_TextRenderer) {
        m_TextRenderer->SetBatcher(this);
        m_TextRenderer->SetScreenSize(m_ScreenW, m_ScreenH);
    }
    
    // Atualizar constant buffer com tamanho da tela
    UpdateUIConstantsBuffer();
}

void UIBatcherDX11::OnEnd() {
    // CRÍTICO: Fazer flush do batch final
    if (!m_CurrentBatch.IsEmpty()) {
        FlushBatch();
    }
    
    // Processar command buffer se habilitado
    if (m_BatchConfig.enableCommandBuffering && !m_CommandBuffer.empty()) {
        ProcessCommandBuffer();
    }
    
    // Trim caches se LRU estiver habilitado
    if (m_QualityConfig.enableLRUCache) {
        TrimGeometryCache();
        TrimTextureCache();
    }
    
    // CRÍTICO: NextFrame do RingBuffer para sincronização
    if (m_RingBuffer) {
        m_RingBuffer->NextFrame();
    }
    
    // Log estatísticas finais
    //Core::Log("[UIBatcherDX11] Frame finalizado - DrawCalls: " + std::to_string(m_Stats.drawCalls) + 
    //          ", Vértices: " + std::to_string(m_Stats.verticesRendered) + 
    //          ", Culled: " + std::to_string(m_Stats.culledElements));
}

void UIBatcherDX11::OnAddRect(float x, float y, float w, float h, Drift::Color color) {
    // Verificar culling
    float boundingBox[4] = {x, y, w, h};
    if (m_CullingSystem && !m_CullingSystem->IsVisible(boundingBox)) {
        m_Stats.culledElements++;
        return;
    }
    
    // Verificar se precisa fazer flush do batch atual
    if (!m_CurrentBatch.IsEmpty() && m_CurrentBatch.hasTexture) {
        FlushBatch();
    }
    
    // Verificar se o batch está cheio
    if (m_CurrentBatch.vertexCount + 4 > m_BatchConfig.maxVertices ||
        m_CurrentBatch.indexCount + 6 > m_BatchConfig.maxIndices) {
        FlushBatch();
    }
    
    // Converter cor ARGB para RGBA usando método da classe base
    Drift::Color rgba = ConvertARGBtoRGBA(color);
    
    // Usar métodos da classe base para conversão de coordenadas
    float clipX0 = ToClipX(x);
    float clipY0 = ToClipY(y);
    float clipX1 = ToClipX(x + w);
    float clipY1 = ToClipY(y);
    float clipX2 = ToClipX(x + w);
    float clipY2 = ToClipY(y + h);
    float clipX3 = ToClipX(x);
    float clipY3 = ToClipY(y + h);
    
    // Adicionar vértices ao batch
    uint32_t baseIndex = static_cast<uint32_t>(m_CurrentBatch.vertices.size());
    
    m_CurrentBatch.vertices.emplace_back(clipX0, clipY0, 0.0f, 0.0f, rgba, 8);
    m_CurrentBatch.vertices.emplace_back(clipX1, clipY1, 1.0f, 0.0f, rgba, 8);
    m_CurrentBatch.vertices.emplace_back(clipX2, clipY2, 1.0f, 1.0f, rgba, 8);
    m_CurrentBatch.vertices.emplace_back(clipX3, clipY3, 0.0f, 1.0f, rgba, 8);
    
    // Adicionar índices
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    m_CurrentBatch.indices.push_back(baseIndex + 1);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 3);
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    
    // Atualizar contadores
    m_CurrentBatch.vertexCount += 4;
    m_CurrentBatch.indexCount += 6;
    m_CurrentBatch.hasTexture = false;
    m_CurrentBatch.isText = false;
    
    m_BatchDirty = true;
}

void UIBatcherDX11::OnAddQuad(float x0, float y0, float x1, float y1,
                             float x2, float y2, float x3, float y3,
                             Drift::Color color) {
    // Verificar se precisa fazer flush do batch atual
    if (!m_CurrentBatch.IsEmpty() && m_CurrentBatch.hasTexture) {
        FlushBatch();
    }
    
    // Verificar se o batch está cheio
    if (m_CurrentBatch.vertexCount + 4 > m_BatchConfig.maxVertices ||
        m_CurrentBatch.indexCount + 6 > m_BatchConfig.maxIndices) {
        FlushBatch();
    }
    
    // Converter cor ARGB para RGBA usando método da classe base
    Drift::Color rgba = ConvertARGBtoRGBA(color);
    
    // Usar métodos da classe base para conversão de coordenadas
    float clipX0 = ToClipX(x0);
    float clipY0 = ToClipY(y0);
    float clipX1 = ToClipX(x1);
    float clipY1 = ToClipY(y1);
    float clipX2 = ToClipX(x2);
    float clipY2 = ToClipY(y2);
    float clipX3 = ToClipX(x3);
    float clipY3 = ToClipY(y3);
    
    // Adicionar vértices ao batch
    uint32_t baseIndex = static_cast<uint32_t>(m_CurrentBatch.vertices.size());
    
    m_CurrentBatch.vertices.emplace_back(clipX0, clipY0, 0.0f, 0.0f, rgba, 8);
    m_CurrentBatch.vertices.emplace_back(clipX1, clipY1, 1.0f, 0.0f, rgba, 8);
    m_CurrentBatch.vertices.emplace_back(clipX2, clipY2, 1.0f, 1.0f, rgba, 8);
    m_CurrentBatch.vertices.emplace_back(clipX3, clipY3, 0.0f, 1.0f, rgba, 8);
    
    // Adicionar índices
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    m_CurrentBatch.indices.push_back(baseIndex + 1);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 3);
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    
    // Atualizar contadores
    m_CurrentBatch.vertexCount += 4;
    m_CurrentBatch.indexCount += 6;
    m_CurrentBatch.hasTexture = false;
    m_CurrentBatch.isText = false;
    
    m_BatchDirty = true;
}

void UIBatcherDX11::OnAddTexturedRect(float x, float y, float w, float h,
                                     const glm::vec2& uvMin, const glm::vec2& uvMax,
                                     Drift::Color color, uint32_t textureId) {
    // Verificar culling
    float boundingBox[4] = {x, y, w, h};
    if (m_CullingSystem && !m_CullingSystem->IsVisible(boundingBox)) {
        m_Stats.culledElements++;
        return;
    }
    
    // Verificar se precisa fazer flush do batch atual
    bool isText = (textureId == 0); // Textura 0 é para texto
    if (!m_CurrentBatch.IsEmpty() && m_CurrentBatch.isText != isText) {
        FlushBatch();
    }
    
    // Verificar se o batch está cheio
    if (m_CurrentBatch.vertexCount + 4 > m_BatchConfig.maxVertices ||
        m_CurrentBatch.indexCount + 6 > m_BatchConfig.maxIndices) {
        FlushBatch();
    }
    
    // Configurar textura no batch
    m_CurrentBatch.textureId = textureId;
    m_CurrentBatch.hasTexture = true;
    m_CurrentBatch.isText = isText;
    
    // Converter cor ARGB para RGBA usando método da classe base
    Drift::Color rgba = ConvertARGBtoRGBA(color);
    
    // Usar métodos da classe base para conversão de coordenadas
    float clipX0 = ToClipX(x);
    float clipY0 = ToClipY(y);
    float clipX1 = ToClipX(x + w);
    float clipY1 = ToClipY(y);
    float clipX2 = ToClipX(x + w);
    float clipY2 = ToClipY(y + h);
    float clipX3 = ToClipX(x);
    float clipY3 = ToClipY(y + h);
    
    // Adicionar vértices ao batch
    uint32_t baseIndex = static_cast<uint32_t>(m_CurrentBatch.vertices.size());
    
    m_CurrentBatch.vertices.emplace_back(clipX0, clipY0, uvMin.x, uvMin.y, rgba, textureId);
    m_CurrentBatch.vertices.emplace_back(clipX1, clipY1, uvMax.x, uvMin.y, rgba, textureId);
    m_CurrentBatch.vertices.emplace_back(clipX2, clipY2, uvMax.x, uvMax.y, rgba, textureId);
    m_CurrentBatch.vertices.emplace_back(clipX3, clipY3, uvMin.x, uvMax.y, rgba, textureId);
    
    // Adicionar índices
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    m_CurrentBatch.indices.push_back(baseIndex + 1);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 2);
    m_CurrentBatch.indices.push_back(baseIndex + 3);
    m_CurrentBatch.indices.push_back(baseIndex + 0);
    
    // Atualizar contadores
    m_CurrentBatch.vertexCount += 4;
    m_CurrentBatch.indexCount += 6;
    
    m_BatchDirty = true;
}

void UIBatcherDX11::OnAddText(float x, float y, const char* text, Drift::Color color) {
    if (!m_TextRenderer) {
        Core::Log("[UIBatcherDX11] ERRO: TextRenderer não inicializado!");
        return;
    }
    
    // Renderizar texto usando o text renderer
    // Converter Drift::Color (uint32_t) para glm::vec4
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    m_TextRenderer->RenderText(std::string(text), glm::vec2(x, y), "fonts/Arial-Regular.ttf", 16.0f, glm::vec4(r, g, b, a));
}

void UIBatcherDX11::OnBeginText() {
    m_AddingText = true;
    
    // Configurar pipeline de texto
    if (m_TextPipeline) {
        auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
        if (contextDX11) {
            contextDX11->SetPipelineState(m_TextPipeline);
            
            // Configurar constant buffer para o shader de texto
            if (m_UIConstantsBuffer) {
                contextDX11->VSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
                contextDX11->PSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
            }
        }
    }
}

void UIBatcherDX11::OnEndText() {
    m_AddingText = false;
    
    // Restaurar pipeline normal
    if (m_Pipeline) {
        auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
        if (contextDX11) {
            contextDX11->SetPipelineState(m_Pipeline);
            
            // Restaurar constant buffer para o pipeline normal
            if (m_UIConstantsBuffer) {
                contextDX11->VSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
                contextDX11->PSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
            }
        }
    }
}

void UIBatcherDX11::OnFlushBatch() {
    if (m_CurrentBatch.IsEmpty()) {
        return;
    }
    
    RenderBatch(m_CurrentBatch);
    m_CurrentBatch.Clear();
    m_BatchDirty = false;
}

void UIBatcherDX11::OnSetBlendMode(uint32_t srcFactor, uint32_t dstFactor) {
    m_SrcBlendFactor = srcFactor;
    m_DstBlendFactor = dstFactor;
    
    // Atualizar pipeline se necessário
    EnsurePipeline();
}

void UIBatcherDX11::OnSetDepthTest(bool enabled) {
    m_DepthTestEnabled = enabled;
    
    // Atualizar pipeline se necessário
    EnsurePipeline();
}

void UIBatcherDX11::OnSetViewport(float x, float y, float w, float h) {
    if (m_CullingSystem) {
        m_CullingSystem->SetViewport(x, y, w, h);
    }
}

// === Detecção de recursos específica do DX11 ===

bool UIBatcherDX11::DetectAnisotropicFiltering() const {
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (!contextDX11) return false;
    
    auto* device = static_cast<ID3D11Device*>(contextDX11->GetNativeDevice());
    if (!device) return false;
    
    // Verificar suporte a anisotropic filtering
    D3D11_FEATURE_DATA_D3D11_OPTIONS options;
    if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &options, sizeof(options)))) {
        return true; // DX11 suporta anisotropic filtering
    }
    
    return false;
}

bool UIBatcherDX11::DetectMSAA() const {
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (!contextDX11) return false;
    
    auto* device = static_cast<ID3D11Device*>(contextDX11->GetNativeDevice());
    if (!device) return false;
    
    // Verificar suporte a MSAA
    UINT numQualityLevels;
    if (SUCCEEDED(device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQualityLevels))) {
        return numQualityLevels > 0;
    }
    
    return false;
}

uint32_t UIBatcherDX11::DetectMaxAnisotropy() const {
    // DX11 suporta até 16x anisotropic filtering
    return 16;
}

size_t UIBatcherDX11::DetectMaxTextureUnits() const {
    // DX11 suporta até 128 texture units
    return 128;
}

size_t UIBatcherDX11::DetectMaxVertexAttributes() const {
    // DX11 suporta até 16 vertex attributes
    return 16;
}

// === Métodos auxiliares específicos do DX11 ===

void UIBatcherDX11::AllocateBuffers() {
    // Alocar vertex buffer
    BufferDesc vertexBufferDesc;
    vertexBufferDesc.type = BufferType::Vertex;
    vertexBufferDesc.sizeBytes = m_QualityConfig.vertexPoolSize * sizeof(UIVertex);
    
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (contextDX11) {
        auto* device = static_cast<ID3D11Device*>(contextDX11->GetNativeDevice());
        if (device) {
            // Criar vertex buffer usando o ring buffer
            if (m_RingBuffer) {
                size_t offset;
                void* data = m_RingBuffer->Allocate(vertexBufferDesc.sizeBytes, 16, offset);
                if (data) {
                    m_VertexBuffer.resize(m_QualityConfig.vertexPoolSize);
                }
            }
        }
    }
}

void UIBatcherDX11::CreateDefaultSampler() {
    SamplerDesc samplerDesc;
    samplerDesc.filter = SamplerDesc::Filter::Anisotropic;
    samplerDesc.addressU = SamplerDesc::AddressMode::Clamp;
    samplerDesc.addressV = SamplerDesc::AddressMode::Clamp;
    samplerDesc.addressW = SamplerDesc::AddressMode::Clamp;
    samplerDesc.maxAnisotropy = m_QualityConfig.maxAnisotropy;
    
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (contextDX11) {
        auto* device = static_cast<ID3D11Device*>(contextDX11->GetNativeDevice());
        if (device) {
            m_DefaultSampler = CreateSamplerDX11(device, samplerDesc);
            if (!m_DefaultSampler) {
                Core::Log("[UIBatcherDX11] ERRO: Falha ao criar sampler padrão!");
            }
        }
    }
}

void UIBatcherDX11::EnsurePipeline() {
    if (m_Pipeline) {
        return;
    }
    
    // Criar descrição do pipeline UI
    PipelineDesc uiDesc;
    uiDesc.vsFile = "shaders/UIBatch.hlsl";
    uiDesc.vsEntry = "VSMain";
    uiDesc.psFile = "shaders/UIBatch.hlsl";
    uiDesc.psEntry = "PSMain";
    
    // Configurar input layout para UIVertex - compatível com shader UIBatch.hlsl
    uiDesc.inputLayout = {
        {"POSITION", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, x)},
        {"TEXCOORD", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, u)},
        {"COLOR", 0, VertexFormat::R8G8B8A8_UNORM, offsetof(UIVertex, color)},
        {"TEXCOORD", 1, VertexFormat::R32_UINT, offsetof(UIVertex, textureId)}
    };
    
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
            m_Pipeline = CreatePipelineDX11(device, uiDesc);
            if (!m_Pipeline) {
                Core::Log("[UIBatcherDX11] ERRO: Falha ao criar pipeline UI!");
                // Tentar criar um pipeline mais simples como fallback
                uiDesc.inputLayout.clear(); // Remover input layout complexo
                m_Pipeline = CreatePipelineDX11(device, uiDesc);
                if (!m_Pipeline) {
                    Core::Log("[UIBatcherDX11] ERRO CRÍTICO: Falha ao criar pipeline fallback!");
                }
            }
        } else {
            Core::Log("[UIBatcherDX11] ERRO: Device DX11 é nullptr!");
        }
    } else {
        Core::Log("[UIBatcherDX11] ERRO: Context DX11 é nullptr!");
    }
}

void UIBatcherDX11::CreateTextPipeline() {
    if (m_TextPipeline) {
        return;
    }

    PipelineDesc textDesc;
    textDesc.vsFile = "shaders/BitmapFontVS.hlsl";
    textDesc.vsEntry = "main";
    textDesc.psFile = "shaders/BitmapFontPS.hlsl";
    textDesc.psEntry = "main";

    // Configurar input layout para UIVertex - compatível com shader BitmapFontVS.hlsl
    textDesc.inputLayout = {
        {"POSITION", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, x)},
        {"TEXCOORD", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, u)},
        {"COLOR", 0, VertexFormat::R8G8B8A8_UNORM, offsetof(UIVertex, color)},
        {"TEXCOORD", 1, VertexFormat::R32_UINT, offsetof(UIVertex, textureId)},
        {"TEXCOORD", 2, VertexFormat::R32_FLOAT, offsetof(UIVertex, offsetX)},
        {"TEXCOORD", 3, VertexFormat::R32_FLOAT, offsetof(UIVertex, offsetY)},
        {"TEXCOORD", 4, VertexFormat::R32_FLOAT, offsetof(UIVertex, scale)},
        {"TEXCOORD", 5, VertexFormat::R32_FLOAT, offsetof(UIVertex, rotation)}
    };

    textDesc.rasterizer.wireframe = false;
    textDesc.rasterizer.cullMode = PipelineDesc::RasterizerDesc::CullMode::None;

    textDesc.blend.enable = true;
    textDesc.blend.srcColor = PipelineDesc::BlendDesc::BlendFactor::SrcAlpha;
    textDesc.blend.dstColor = PipelineDesc::BlendDesc::BlendFactor::InvSrcAlpha;
    textDesc.blend.colorOp = PipelineDesc::BlendDesc::BlendOp::Add;
    textDesc.blend.srcAlpha = PipelineDesc::BlendDesc::BlendFactor::One;
    textDesc.blend.dstAlpha = PipelineDesc::BlendDesc::BlendFactor::InvSrcAlpha;
    textDesc.blend.alphaOp = PipelineDesc::BlendDesc::BlendOp::Add;
    textDesc.blend.blendFactorSeparate = true;
    textDesc.blend.alphaToCoverage = false;

    textDesc.depthStencil.depthEnable = false;
    textDesc.depthStencil.depthWrite = false;

    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (contextDX11) {
        auto* device = static_cast<ID3D11Device*>(contextDX11->GetNativeDevice());
        if (device) {
            m_TextPipeline = CreatePipelineDX11(device, textDesc);
            if (!m_TextPipeline) {
                Core::Log("[UIBatcherDX11] ERRO: Falha ao criar pipeline de texto bitmap!");
            }
        }
    }
}

void UIBatcherDX11::CreateInstancedPipeline() {
    // Criar pipeline para instancing
    // Por enquanto, usar o pipeline padrão
    // Implementação completa seria específica para instancing
}

void UIBatcherDX11::RenderBatch(const UIBatch& batch) {
    try {
        if (batch.IsEmpty() || !m_RingBuffer) {
            return;
        }

        // Garante que a textura 0 está correta para texto
        if (batch.isText && !m_Textures[0]) {
            Core::Log("[UIBatcherDX11][ERRO] Textura não configurada para renderização de texto!");
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
        
        // Configurar pipeline
        EnsurePipeline();
        if (batch.isText && m_TextPipeline) {
            contextDX11->SetPipelineState(m_TextPipeline.get());
            if (m_UIConstantsBuffer) {
                contextDX11->VSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
                contextDX11->PSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
            }
        } else if (m_Pipeline) {
            contextDX11->SetPipelineState(m_Pipeline.get());
            if (m_UIConstantsBuffer) {
                contextDX11->VSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
                contextDX11->PSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
            }
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
        for (size_t i = 0; i < m_Textures.size() && i < 16; ++i) {
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

void UIBatcherDX11::ProcessCommandBuffer() {
    if (m_CommandBuffer.empty()) {
        return;
    }
    
    // Ordenar comandos por textura para minimizar state changes
    SortCommandsByTexture();
    
    // Processar comandos
    for (const auto& cmd : m_CommandBuffer) {
        switch (cmd.type) {
            case UIRenderCommand::Type::Rect:
                AddQuad(cmd.x, cmd.y, cmd.x + cmd.w, cmd.y, 
                       cmd.x + cmd.w, cmd.y + cmd.h, cmd.x, cmd.y + cmd.h, cmd.color);
                break;
            case UIRenderCommand::Type::TexturedRect:
                AddTexturedRect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.uvMin, cmd.uvMax, cmd.color, cmd.textureId);
                break;
            case UIRenderCommand::Type::Instanced:
                // Implementar instancing
                break;
            default:
                break;
        }
    }
    
    m_CommandBuffer.clear();
}

void UIBatcherDX11::SortCommandsByTexture() {
    std::sort(m_CommandBuffer.begin(), m_CommandBuffer.end(),
              [](const UIRenderCommand& a, const UIRenderCommand& b) {
                  if (a.textureId != b.textureId) {
                      return a.textureId < b.textureId;
                  }
                  return static_cast<int>(a.type) < static_cast<int>(b.type);
              });
}

void UIBatcherDX11::RenderVertices(const UIVertex* vertices, size_t vertexCount, 
                                  const uint32_t* indices, size_t indexCount, bool hasTexture) {
    if (!m_Context || !m_Pipeline) {
        Core::Log("[UIBatcherDX11] ERRO: Context ou Pipeline é nullptr!");
        return;
    }
    
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (!contextDX11) {
        Core::Log("[UIBatcherDX11] ERRO: Context DX11 é nullptr!");
        return;
    }
    
    // Configurar pipeline
    contextDX11->SetPipelineState(m_Pipeline.get());
    
    // Configurar constant buffer
    if (m_UIConstantsBuffer) {
        contextDX11->VSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
        contextDX11->PSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
    }
    
    // CRÍTICO: Configurar texturas se necessário
    if (hasTexture) {
        // Configurar texturas do array de texturas
        for (size_t i = 0; i < m_Textures.size() && i < 16; ++i) {
            if (m_Textures[i]) {
                contextDX11->PSSetTexture(static_cast<UINT>(i), m_Textures[i]);
                if (m_DefaultSampler) {
                    contextDX11->PSSetSampler(static_cast<UINT>(i), m_DefaultSampler.get());
                }
            }
        }
    }
    
    // CRÍTICO: Alocar dados no ring buffer
    if (m_RingBuffer) {
        size_t vertexOffset;
        void* vertexData = m_RingBuffer->Allocate(vertexCount * sizeof(UIVertex), 16, vertexOffset);
        if (vertexData) {
            // Copiar vértices para o ring buffer
            memcpy(vertexData, vertices, vertexCount * sizeof(UIVertex));
            
            // Fazer bind do vertex buffer usando a interface correta
            IBuffer* ringBuffer = m_RingBuffer->GetBuffer();
            if (ringBuffer) {
                contextDX11->IASetVertexBuffer(ringBuffer->GetBackendHandle(), sizeof(UIVertex), vertexOffset);
            }
            
            // Renderizar
            if (indexCount > 0) {
                // Renderizar com índices
                size_t indexOffset;
                void* indexData = m_RingBuffer->Allocate(indexCount * sizeof(uint32_t), 4, indexOffset);
                if (indexData) {
                    memcpy(indexData, indices, indexCount * sizeof(uint32_t));
                    contextDX11->IASetIndexBuffer(ringBuffer->GetBackendHandle(), Format::R32_UINT, indexOffset);
                    contextDX11->DrawIndexed(indexCount, 0, 0);
                }
            } else {
                // Renderizar sem índices (quads)
                contextDX11->Draw(vertexCount, 0);
            }
        } else {
            Core::Log("[UIBatcherDX11] ERRO: Falha ao alocar vértices no ring buffer!");
        }
    } else {
        Core::Log("[UIBatcherDX11] ERRO: Ring buffer é nullptr!");
    }
    
    // Atualizar estatísticas
    m_Stats.verticesRendered += vertexCount;
    m_Stats.indicesRendered += indexCount;
    m_Stats.drawCalls++;
    
    if (hasTexture) {
        m_Stats.textureSwitches++;
    }
}

void UIBatcherDX11::CreateUIConstantsBuffer() {
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (!contextDX11) {
        Core::Log("[UIBatcherDX11] ERRO: Context DX11 é nullptr!");
        return;
    }
    
    auto* device = static_cast<ID3D11Device*>(contextDX11->GetNativeDevice());
    if (!device) {
        Core::Log("[UIBatcherDX11] ERRO: Device DX11 é nullptr!");
        return;
    }
    
    // Criar constant buffer para UI
    BufferDesc cbDesc;
    cbDesc.type = BufferType::Constant;
    cbDesc.sizeBytes = sizeof(UIConstants);
    
    m_UIConstantsBuffer = CreateBufferDX11(device, static_cast<ID3D11DeviceContext*>(contextDX11->GetNativeContext()), cbDesc);
    
    if (!m_UIConstantsBuffer) {
        Core::Log("[UIBatcherDX11] ERRO: Falha ao criar constant buffer UI!");
    }
}

void UIBatcherDX11::UpdateUIConstantsBuffer() {
    if (!m_UIConstantsBuffer) {
        return;
    }
    
    // Atualizar constantes
    m_UIConstants.screenSize[0] = static_cast<float>(m_ScreenW);
    m_UIConstants.screenSize[1] = static_cast<float>(m_ScreenH);
    m_UIConstants.atlasSize[0] = 1024.0f; // Tamanho padrão do atlas
    m_UIConstants.atlasSize[1] = 1024.0f;
    m_UIConstants.padding[0] = 0.0f;
    m_UIConstants.padding[1] = 0.0f;
    m_UIConstants.time = 0.0f; // TODO: Implementar tempo real
    m_UIConstants.debugColor[0] = 1.0f;
    m_UIConstants.debugColor[1] = 0.0f;
    m_UIConstants.debugColor[2] = 0.0f;
    m_UIConstants.debugColor[3] = 1.0f;
    
    // Atualizar buffer
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (contextDX11) {
        contextDX11->UpdateConstantBuffer(m_UIConstantsBuffer.get(), &m_UIConstants, sizeof(UIConstants));
        contextDX11->VSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
        contextDX11->PSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
    } else {
        Core::Log("[UIBatcherDX11] ERRO: Context DX11 é nullptr ao atualizar constant buffer!");
    }
}

// Fábrica para criar UIBatcherDX11
std::unique_ptr<IUIBatcher> CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx) {
    return std::make_unique<UIBatcherDX11>(std::move(ringBuffer), ctx);
}

} // namespace Drift::RHI::DX11 

 