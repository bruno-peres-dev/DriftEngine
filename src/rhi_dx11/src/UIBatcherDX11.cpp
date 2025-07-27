// src/rhi_dx11/src/UIBatcherDX11.cpp
#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include "Drift/RHI/DX11/SamplerDX11.h"
#include "Drift/RHI/DX11/BufferDX11.h"
#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/Core/Log.h"
#include <d3d11.h>
#include <algorithm>
#include <chrono>
#include <thread>

namespace Drift::RHI::DX11 {

// Conversão de cor ARGB para RGBA
inline Drift::Color ConvertARGBtoRGBA(Drift::Color argb) {
    uint8_t a = (argb >> 24) & 0xFF;
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >> 8) & 0xFF;
    uint8_t b = argb & 0xFF;
    return r | (g << 8) | (b << 16) | (a << 24);
}

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
    
    // Criar pipeline de instancing
    CreateInstancedPipeline();
    
    // Inicializar text renderer
    m_TextRenderer = std::make_unique<Drift::UI::TextRenderer>();
    
    // Alocar buffers de renderização
    AllocateBuffers();
    
    // Criar constant buffer para UI
    CreateUIConstantsBuffer();
    
    Core::Log("[UIBatcherDX11] Inicializado com configurações AAA");
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
    
    // Limpar command buffer
    m_CommandBuffer.clear();
    
    // Atualizar culling system
    if (m_CullingSystem) {
        m_CullingSystem->SetViewport(0, 0, m_ScreenW, m_ScreenH);
    }
    
    // Configurar pipeline
    EnsurePipeline();
    
    // Configurar sampler padrão
    if (m_DefaultSampler) {
        auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
        if (contextDX11) {
            contextDX11->SetSampler(0, m_DefaultSampler);
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
    Core::Log("[UIBatcherDX11] Frame finalizado - DrawCalls: " + std::to_string(m_Stats.drawCalls) + 
              ", Vértices: " + std::to_string(m_Stats.verticesRendered) + 
              ", Culled: " + std::to_string(m_Stats.culledElements));
}

void UIBatcherDX11::OnAddRect(float x, float y, float w, float h, Drift::Color color) {
    // Verificar culling
    if (m_CullingSystem && !m_CullingSystem->IsVisible(new float[4]{x, y, w, h})) {
        m_Stats.culledElements++;
        return;
    }
    
    // Adicionar comando se buffering estiver habilitado
    if (m_BatchConfig.enableCommandBuffering) {
        UIRenderCommand cmd;
        cmd.type = UIRenderCommand::Type::Rect;
        cmd.x = x;
        cmd.y = y;
        cmd.w = w;
        cmd.h = h;
        cmd.color = color;
        m_CommandBuffer.push_back(cmd);
        return;
    }
    
    // Renderização imediata
    AddQuad(x, y, x + w, y, x + w, y + h, x, y + h, color);
}

void UIBatcherDX11::OnAddQuad(float x0, float y0, float x1, float y1,
                             float x2, float y2, float x3, float y3,
                             Drift::Color color) {
    // Converter cor ARGB para RGBA
    Drift::Color rgbaColor = ConvertARGBtoRGBA(color);
    
    // Criar vértices otimizados
    UIVertex* vertices = m_VertexPool->Allocate(4);
    if (!vertices) {
        Core::Log("[UIBatcherDX11] ERRO: Vertex pool cheio!");
        return;
    }
    
    vertices[0] = UIVertex(x0, y0, 0.0f, 0.0f, rgbaColor, 8);
    vertices[1] = UIVertex(x1, y1, 1.0f, 0.0f, rgbaColor, 8);
    vertices[2] = UIVertex(x2, y2, 1.0f, 1.0f, rgbaColor, 8);
    vertices[3] = UIVertex(x3, y3, 0.0f, 1.0f, rgbaColor, 8);
    
    // Renderizar vértices
    RenderVertices(vertices, 4, nullptr, 0, false);
    
    Core::Log("[UIBatcherDX11] Quad renderizado: (" + std::to_string(x0) + "," + std::to_string(y0) + 
              ") -> (" + std::to_string(x2) + "," + std::to_string(y2) + ")");
}

void UIBatcherDX11::OnAddTexturedRect(float x, float y, float w, float h,
                                     const glm::vec2& uvMin, const glm::vec2& uvMax,
                                     Drift::Color color, uint32_t textureId) {
    // Verificar culling
    if (m_CullingSystem && !m_CullingSystem->IsVisible(new float[4]{x, y, w, h})) {
        m_Stats.culledElements++;
        return;
    }
    
    // Converter cor ARGB para RGBA
    Drift::Color rgbaColor = ConvertARGBtoRGBA(color);
    
    // Criar vértices otimizados
    UIVertex* vertices = m_VertexPool->Allocate(4);
    if (!vertices) {
        Core::Log("[UIBatcherDX11] ERRO: Vertex pool cheio!");
        return;
    }
    
    vertices[0] = UIVertex(x, y, uvMin.x, uvMin.y, rgbaColor, textureId);
    vertices[1] = UIVertex(x + w, y, uvMax.x, uvMin.y, rgbaColor, textureId);
    vertices[2] = UIVertex(x + w, y + h, uvMax.x, uvMax.y, rgbaColor, textureId);
    vertices[3] = UIVertex(x, y + h, uvMin.x, uvMax.y, rgbaColor, textureId);
    
    // Renderizar vértices
    RenderVertices(vertices, 4, nullptr, 0, true);
    
    Core::Log("[UIBatcherDX11] Textured rect renderizado: textureId=" + std::to_string(textureId));
}

void UIBatcherDX11::OnAddText(float x, float y, const char* text, Drift::Color color) {
    if (!m_TextRenderer) {
        Core::Log("[UIBatcherDX11] ERRO: TextRenderer não inicializado!");
        return;
    }
    
    // Configurar textura de fonte
    if (m_Textures.find(0) != m_Textures.end()) {
        SetTexture(0, m_Textures[0]);
    }
    
    // Renderizar texto usando o text renderer
    m_TextRenderer->AddText(std::string(text), glm::vec2(x, y), "Arial", 16.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    Core::Log("[UIBatcherDX11] Texto renderizado: '" + std::string(text) + "' em (" + 
              std::to_string(x) + "," + std::to_string(y) + ")");
}

void UIBatcherDX11::OnBeginText() {
    m_AddingText = true;
    
    // Configurar pipeline de texto
    if (m_TextPipeline) {
        auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
        if (contextDX11) {
            contextDX11->SetPipelineState(m_TextPipeline);
        }
    }
    
    Core::Log("[UIBatcherDX11] Iniciando renderização de texto");
}

void UIBatcherDX11::OnEndText() {
    m_AddingText = false;
    
    // Restaurar pipeline normal
    if (m_Pipeline) {
        auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
        if (contextDX11) {
            contextDX11->SetPipelineState(m_Pipeline);
        }
    }
    
    Core::Log("[UIBatcherDX11] Finalizando renderização de texto");
}

void UIBatcherDX11::OnFlushBatch() {
    // Implementação específica do DX11 para flush
    Core::Log("[UIBatcherDX11] Flush de batch executado");
}

void UIBatcherDX11::OnSetBlendMode(uint32_t srcFactor, uint32_t dstFactor) {
    m_SrcBlendFactor = srcFactor;
    m_DstBlendFactor = dstFactor;
    
    // Atualizar pipeline se necessário
    EnsurePipeline();
    
    Core::Log("[UIBatcherDX11] Blend mode alterado: " + std::to_string(srcFactor) + "/" + std::to_string(dstFactor));
}

void UIBatcherDX11::OnSetDepthTest(bool enabled) {
    m_DepthTestEnabled = enabled;
    
    // Atualizar pipeline se necessário
    EnsurePipeline();
    
    Core::Log("[UIBatcherDX11] Depth test " + std::string(enabled ? "habilitado" : "desabilitado"));
}

void UIBatcherDX11::OnSetViewport(float x, float y, float w, float h) {
    if (m_CullingSystem) {
        m_CullingSystem->SetViewport(x, y, w, h);
    }
    
    Core::Log("[UIBatcherDX11] Viewport alterado: " + std::to_string(w) + "x" + std::to_string(h));
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
                    Core::Log("[UIBatcherDX11] Vertex buffer alocado via ring buffer");
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
            if (m_DefaultSampler) {
                Core::Log("[UIBatcherDX11] Sampler padrão criado com sucesso");
            } else {
                Core::Log("[UIBatcherDX11] ERRO: Falha ao criar sampler padrão!");
            }
        }
    }
}

void UIBatcherDX11::EnsurePipeline() {
    if (m_Pipeline) {
        return;
    }
    
    Core::Log("[UIBatcherDX11] Configurando pipeline UI...");
    
    // Criar descrição do pipeline UI
    PipelineDesc uiDesc;
    uiDesc.vsFile = "shaders/UIBatch.hlsl";
    uiDesc.vsEntry = "VSMain";
    uiDesc.psFile = "shaders/UIBatch.hlsl";
    uiDesc.psEntry = "PSMain";
    
    // Configurar input layout para UIVertex (compatível com shader simplificado)
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
    
    Core::Log("[UIBatcherDX11] Blend state configurado: SrcAlpha/InvSrcAlpha");
    
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

void UIBatcherDX11::CreateTextPipeline() {
    if (m_TextPipeline) {
        return;
    }

    Core::Log("[UIBatcherDX11] Criando pipeline de texto...");
    
    PipelineDesc textDesc;
    textDesc.vsFile = "shaders/BitmapFontVS.hlsl";
    textDesc.vsEntry = "main";
    textDesc.psFile = "shaders/BitmapFontPS.hlsl";
    textDesc.psEntry = "main";

    textDesc.inputLayout = {
        {"POSITION", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, x)},
        {"TEXCOORD", 0, VertexFormat::R32G32_FLOAT, offsetof(UIVertex, u)},
        {"COLOR", 0, VertexFormat::R8G8B8A8_UNORM, offsetof(UIVertex, color)},
        {"TEXCOORD", 1, VertexFormat::R32_UINT, offsetof(UIVertex, textureId)}
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
            Core::Log("[UIBatcherDX11] Criando pipeline de texto bitmap...");
            m_TextPipeline = CreatePipelineDX11(device, textDesc);
            if (m_TextPipeline) {
                Core::Log("[UIBatcherDX11] Pipeline de texto bitmap criado com sucesso");
            } else {
                Core::Log("[UIBatcherDX11] ERRO: Falha ao criar pipeline de texto bitmap!");
            }
        }
    }
}

void UIBatcherDX11::CreateInstancedPipeline() {
    // Criar pipeline para instancing
    // Por enquanto, usar o pipeline padrão
    // Implementação completa seria específica para instancing
    Core::Log("[UIBatcherDX11] Pipeline de instancing configurado");
}

void UIBatcherDX11::ProcessCommandBuffer() {
    if (m_CommandBuffer.empty()) {
        return;
    }
    
    Core::Log("[UIBatcherDX11] Processando " + std::to_string(m_CommandBuffer.size()) + " comandos");
    
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
        return;
    }
    
    auto* contextDX11 = static_cast<ContextDX11*>(m_Context);
    if (!contextDX11) {
        return;
    }
    
    // Configurar pipeline
    contextDX11->SetPipelineState(m_Pipeline.get());
    
    // Configurar constant buffer
    if (m_UIConstantsBuffer) {
        contextDX11->VSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
        contextDX11->PSSetConstantBuffer(0, m_UIConstantsBuffer->GetBackendHandle());
    }
    
    // Configurar sampler se necessário
    if (hasTexture && m_DefaultSampler) {
        contextDX11->SetSampler(0, m_DefaultSampler.get());
    }
    
    // CRÍTICO: Alocar dados no ring buffer
    if (m_RingBuffer) {
        size_t vertexOffset;
        void* vertexData = m_RingBuffer->Allocate(vertexCount * sizeof(UIVertex), 16, vertexOffset);
        if (vertexData) {
            // Copiar vértices para o ring buffer
            memcpy(vertexData, vertices, vertexCount * sizeof(UIVertex));
            
            // Fazer bind do vertex buffer
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
            
            Core::Log("[UIBatcherDX11] Renderizando " + std::to_string(vertexCount) + " vértices via ring buffer");
        }
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
    
    if (m_UIConstantsBuffer) {
        Core::Log("[UIBatcherDX11] Constant buffer UI criado com sucesso");
    } else {
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
    }
}

// Fábrica para criar UIBatcherDX11
std::unique_ptr<IUIBatcher> CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx) {
    return std::make_unique<UIBatcherDX11>(std::move(ringBuffer), ctx);
}

} // namespace Drift::RHI::DX11 

 