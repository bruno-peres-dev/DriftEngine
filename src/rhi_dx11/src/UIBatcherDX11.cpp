#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/Context.h"
#include <cstring>

using namespace Drift::RHI::DX11;
using namespace Drift::RHI;

// Construtor: armazena ring buffer e contexto
UIBatcherDX11::UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx)
    : _ringBuffer(std::move(ringBuffer)), _ctx(ctx) {}

void UIBatcherDX11::Begin() {
    _vertices.clear();
    _indices.clear();
}

// Adiciona um retângulo ao batch de UI
void UIBatcherDX11::AddRect(float x, float y, float w, float h, unsigned color) {
    unsigned base = (unsigned)_vertices.size();
    _vertices.push_back({x,     y,     color});
    _vertices.push_back({x + w, y,     color});
    _vertices.push_back({x + w, y + h, color});
    _vertices.push_back({x,     y + h, color});
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
    if (_vertices.empty() || _indices.empty()) return;
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

// Fábrica de UIBatcherDX11
std::unique_ptr<IUIBatcher> Drift::RHI::DX11::CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx) {
    return std::make_unique<UIBatcherDX11>(std::move(ringBuffer), ctx);
} 