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
    // Desativa teste de profundidade e escrita, para overlay
    _ctx->SetDepthTestEnabled(false);

    _vertices.clear();
    _indices.clear();
}

// Adiciona um retângulo ao batch de UI
void UIBatcherDX11::AddRect(float x, float y, float w, float h, unsigned color) {
    // Debug: Log a cor recebida
    static int addRectLogCount = 0;
    addRectLogCount++;
    if (addRectLogCount % 10 == 0) { // Log a cada 10 retângulos
        Core::Log("[UIBatcher] AddRect - Posição: (" + std::to_string(x) + ", " + std::to_string(y) + 
                 ") - Cor recebida: 0x" + std::to_string(color));
    }
    auto toClipX = [this](float px) {
        return (px / _screenW) * 2.0f - 1.0f;
    };
    auto toClipY = [this](float py) {
        return 1.0f - (py / _screenH) * 2.0f;
    };

    // Debug: Log as coordenadas de clip calculadas
    static int clipLogCount = 0;
    clipLogCount++;
    if (clipLogCount % 10 == 0) { // Log a cada 10 retângulos
        float clipX = toClipX(x);
        float clipY = toClipY(y);
        float clipX2 = toClipX(x + w);
        float clipY2 = toClipY(y + h);
        Core::Log("[UIBatcher] Coordenadas de clip - Screen: (" + std::to_string(_screenW) + ", " + std::to_string(_screenH) + 
                 ") - Pos: (" + std::to_string(x) + ", " + std::to_string(y) + 
                 ") -> Clip: (" + std::to_string(clipX) + ", " + std::to_string(clipY) + 
                 ") to (" + std::to_string(clipX2) + ", " + std::to_string(clipY2) + ")");
    }

                    // Debug: Verificar se a cor é válida (32 bits)
                if (color > 0xFFFFFFFF) {
                    Core::Log("[UIBatcher] ERRO: Cor inválida detectada: 0x" + std::to_string(color) + 
                              " (maior que 32 bits) - Convertendo para 32 bits");
                    color = color & 0xFFFFFFFF; // Trunca para 32 bits
                }
                
                // Converte ARGB para BGRA (DirectX espera BGRA para R8G8B8A8_UNORM)
                unsigned a = (color >> 24) & 0xFF;
                unsigned r = (color >> 16) & 0xFF;
                unsigned g = (color >> 8) & 0xFF;
                unsigned b = color & 0xFF;
                unsigned bgra = (b) | (g << 8) | (r << 16) | (a << 24);
    
                    // Debug: Log cores para debug
                static int colorCount = 0;
                colorCount++;
                if (colorCount % 10 == 0) { // Log mais frequente para debug
                    Core::Log("[UIBatcher] Cor original: 0x" + std::to_string(color) + 
                              " -> BGRA: 0x" + std::to_string(bgra) + 
                              " (R:" + std::to_string(r) + " G:" + std::to_string(g) + 
                              " B:" + std::to_string(b) + " A:" + std::to_string(a) + ")");
        }

    // Debug: Log a cada 10 retângulos para não spam
    static int rectCount = 0;
    rectCount++;
    if (rectCount % 10 == 0) {
        Core::Log("[UIBatcher] Adicionando retângulo " + std::to_string(rectCount) + 
                  " na posição (" + std::to_string(x) + ", " + std::to_string(y) + 
                  ") com cor 0x" + std::to_string(color) + " -> 0x" + std::to_string(bgra));
    }

    unsigned base = (unsigned)_vertices.size();
    _vertices.push_back({toClipX(x),       toClipY(y),       bgra});
    _vertices.push_back({toClipX(x + w),   toClipY(y),       bgra});
    _vertices.push_back({toClipX(x + w),   toClipY(y + h),   bgra});
    _vertices.push_back({toClipX(x),       toClipY(y + h),   bgra});
    _indices.push_back(base + 0);
    _indices.push_back(base + 1);
    _indices.push_back(base + 2);
    _indices.push_back(base + 2);
    _indices.push_back(base + 3);
    _indices.push_back(base + 0);
}

// Stub: integração futura com sistema de fontes/texto
void UIBatcherDX11::AddText(float, float, const char*, unsigned) {
    // Não implementado
}

// Finaliza o batch e envia draw calls para a UI
void UIBatcherDX11::End() {
    if (_vertices.empty() || _indices.empty()) {
        return;
    }

    EnsurePipeline();

    // Desabilita depth
    _ctx->SetDepthTestEnabled(false);
    // Assume viewport já configurado pelo RenderManager
    _pipeline->Apply(*_ctx);

    // Garante que o ring buffer está pronto para o próximo frame
    _ringBuffer->NextFrame();

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
    _ctx->DrawIndexed((UINT)_indices.size(), 0, 0);
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
        {"POSITION", 0, VertexFormat::R32G32_FLOAT, 0},
        {"COLOR",   0, VertexFormat::R8G8B8A8_UNORM, 8}
    };

    // Sem defines de debug por padrão
    desc.blend.enable = true;
    desc.blend.srcColor = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::One;
    desc.blend.dstColor = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::Zero;
    desc.blend.srcAlpha = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::One;
    desc.blend.dstAlpha = Drift::RHI::PipelineDesc::BlendDesc::BlendFactor::Zero;
    desc.blend.blendFactorSeparate = true;

    desc.depthStencil.depthEnable = false;
    desc.depthStencil.depthWrite = false;
    desc.rasterizer.cullMode = Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::None;

    _pipeline = Drift::RHI::DX11::CreatePipelineDX11(device, desc);

}

// Fábrica de UIBatcherDX11
std::unique_ptr<IUIBatcher> Drift::RHI::DX11::CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx) {
    return std::make_unique<UIBatcherDX11>(std::move(ringBuffer), ctx);
} 