#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/Context.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include <cstring>

using namespace Drift::RHI::DX11;
using namespace Drift::RHI;

// Construtor: armazena ring buffer e contexto
UIBatcherDX11::UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx)
    : _ringBuffer(std::move(ringBuffer)), _ctx(ctx) {}

void UIBatcherDX11::Begin() {
    // Garante que estamos renderizando no back-buffer atual
    _ctx->BindBackBufferRTV();
    
    // Log para debug
    Drift::Core::Log("[UIBatcher] Begin: BindBackBufferRTV chamado");

    // Desativa teste de profundidade e escrita, para overlay
    _ctx->SetDepthTestEnabled(false);
    
    // Log para debug
    Drift::Core::Log("[UIBatcher] Begin: SetDepthTestEnabled(false) chamado");

    _vertices.clear();
    _indices.clear();
}

// Adiciona um retângulo ao batch de UI
void UIBatcherDX11::AddRect(float x, float y, float w, float h, unsigned color) {
    auto toClipX = [this](float px) {
        return (px / _screenW) * 2.0f - 1.0f;
    };
    auto toClipY = [this](float py) {
        return 1.0f - (py / _screenH) * 2.0f;
    };

    // Log para debug
    float clipX1 = toClipX(x);
    float clipY1 = toClipY(y);
    float clipX2 = toClipX(x + w);
    float clipY2 = toClipY(y + h);
    
    Drift::Core::Log("[UIBatcher] AddRect: screen(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(w) + "," + std::to_string(h) + 
                     ") -> clip(" + std::to_string(clipX1) + "," + std::to_string(clipY1) + "," + std::to_string(clipX2) + "," + std::to_string(clipY2) + 
                     ") screenSize(" + std::to_string(_screenW) + "," + std::to_string(_screenH) + ")");

    unsigned base = (unsigned)_vertices.size();
    _vertices.push_back({toClipX(x),       toClipY(y),       color});
    _vertices.push_back({toClipX(x + w),   toClipY(y),       color});
    _vertices.push_back({toClipX(x + w),   toClipY(y + h),   color});
    _vertices.push_back({toClipX(x),       toClipY(y + h),   color});
    _indices.push_back(base + 0);
    _indices.push_back(base + 1);
    _indices.push_back(base + 2);
    _indices.push_back(base + 2);
    _indices.push_back(base + 3);
    _indices.push_back(base + 0);

    // Log de depuração removido para melhor performance
}

// Stub: integração futura com sistema de fontes/texto
void UIBatcherDX11::AddText(float, float, const char*, unsigned) {
    // Não implementado
}

// Finaliza o batch e envia draw calls para a UI
void UIBatcherDX11::End() {
    if (_vertices.empty() || _indices.empty()) {
        Drift::Core::Log("[UIBatcher] End: Vertices ou indices vazios, pulando renderização");
        return;
    }
    
    Drift::Core::Log("[UIBatcher] End: Renderizando " + std::to_string(_vertices.size()) + " vertices, " + std::to_string(_indices.size()) + " indices");

    EnsurePipeline();

    // Desabilita depth - garantindo que está desabilitado antes do draw
    _ctx->SetDepthTestEnabled(false);
    Drift::Core::Log("[UIBatcher] End: SetDepthTestEnabled(false) chamado antes do draw");
    
    // Assume viewport já configurado pelo RenderManager
    _pipeline->Apply(*_ctx);
    Drift::Core::Log("[UIBatcher] End: Pipeline aplicado");

    size_t vtxSize = _vertices.size() * sizeof(Vertex);
    size_t idxSize = _indices.size() * sizeof(unsigned);
    size_t vtxOffset = 0, idxOffset = 0;
    void* vtxPtr = _ringBuffer->Allocate(vtxSize, 16, vtxOffset);
    void* idxPtr = _ringBuffer->Allocate(idxSize, 4, idxOffset);
    std::memcpy(vtxPtr, _vertices.data(), vtxSize);
    std::memcpy(idxPtr, _indices.data(), idxSize);
    IBuffer* vtxBuf = _ringBuffer->GetBuffer();
    IBuffer* idxBuf = _ringBuffer->GetBuffer();
    _ctx->IASetVertexBuffer(vtxBuf->GetBackendHandle(), sizeof(Vertex), (UINT)vtxOffset);
    _ctx->IASetIndexBuffer(idxBuf->GetBackendHandle(), Format::R32_UINT, (UINT)idxOffset);
    _ctx->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);
    
    Drift::Core::Log("[UIBatcher] End: Buffers configurados, chamando DrawIndexed com " + std::to_string(_indices.size()) + " indices");
    _ctx->DrawIndexed((UINT)_indices.size(), 0, 0);
    
    Drift::Core::Log("[UIBatcher] End: DrawIndexed completado");

    // Log removido para performance - DrawIndexed count=" + std::to_string(_indices.size())
}

void UIBatcherDX11::EnsurePipeline() {
    if (_pipeline) return;

    // Cria pipeline simples
    ID3D11Device* device = static_cast<ID3D11Device*>(_ctx->GetNativeDevice());
    Drift::RHI::PipelineDesc desc;
    desc.vsFile = "shaders/UIBatch.hlsl";
    desc.psFile = "shaders/UIBatch.hlsl";
    desc.vsEntry = "VSMain";
    desc.psEntry = "PSMain";
    desc.inputLayout = {
        {"POSITION", 0, 0, "R32G32_FLOAT"},
        {"COLOR",   0, 8, "R8G8B8A8_UNORM"}
    };

    // Sem defines de debug por padrão
    desc.blend.enable = true;
    desc.blend.srcColor = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::SrcAlpha;
    desc.blend.dstColor = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::InvSrcAlpha;
    desc.blend.srcAlpha = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::One;
    desc.blend.dstAlpha = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::InvSrcAlpha;
    desc.blend.blendFactorSeparate = true;

    desc.depthStencil.depthEnable = false;
    desc.depthStencil.depthWrite = false;
    desc.rasterizer.cullMode = Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::None;

    _pipeline = Drift::RHI::DX11::CreatePipelineDX11(device, desc);

    // Log removido para performance: Pipeline UI criado com sucesso
}

// Fábrica de UIBatcherDX11
std::unique_ptr<IUIBatcher> Drift::RHI::DX11::CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx) {
    return std::make_unique<UIBatcherDX11>(std::move(ringBuffer), ctx);
} 