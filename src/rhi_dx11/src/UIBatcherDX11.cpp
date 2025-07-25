#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/Context.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include "Drift/UI/FontSystem/TextRenderer.h"
#include <cstring>
#include <algorithm>

using namespace Drift::RHI::DX11;
using namespace Drift::RHI;

// Conversão ARGB para BGRA otimizada (inline para performance)
inline Drift::Color ConvertARGBtoBGRA(Drift::Color argb) {
    // ARGB: AAAA AAAA RRRR RRRR GGGG GGGG BBBB BBBB
    // BGRA: BBBB BBBB GGGG GGGG RRRR RRRR AAAA AAAA
    return ((argb & 0x000000FF) << 16) |  // B -> posição 16
           ((argb & 0x0000FF00)) |         // G -> posição 8  
           ((argb & 0x00FF0000) >> 16) |   // R -> posição 0
           ((argb & 0xFF000000));          // A -> posição 24 (corrigido)
}

// TODO: Para otimização futura, considerar lookup table para cores comuns
// static const unsigned COLOR_LOOKUP_TABLE[256] = { ... };

// Construtor: armazena ring buffer e contexto
UIBatcherDX11::UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, IContext* ctx)
    : _ringBuffer(std::move(ringBuffer)), _ctx(ctx) {
    
    // Inicializa o sistema de renderização de texto
    _textRenderer = std::make_unique<Drift::UI::UIBatcherTextRenderer>(this);
}

void UIBatcherDX11::Begin() {
    // Desativa teste de profundidade e escrita, para overlay
    _ctx->SetDepthTestEnabled(false);

    _vertices.clear();
    _indices.clear();
    
    // Inicia renderização de texto
    if (_textRenderer) {
        _textRenderer->BeginTextRendering();
    }
}

// Adiciona um retângulo ao batch de UI
void UIBatcherDX11::AddRect(float x, float y, float w, float h, Drift::Color color) {
    // Verifica se o retângulo está visível dentro do scissor atual
    ScissorRect currentScissor = GetCurrentScissorRect();
    if (currentScissor.IsValid()) {
        // Se há scissor ativo, calcula a interseção
        ScissorRect clippedRect = ClipRectToScissor(ScissorRect(x, y, w, h), currentScissor);
        if (!clippedRect.IsValid()) {
            return; // Retângulo completamente fora da área visível
        }
        // Usa as coordenadas recortadas
        x = clippedRect.x;
        y = clippedRect.y;
        w = clippedRect.width;
        h = clippedRect.height;
    }
    
    auto toClipX = [this](float px) {
        return (px / _screenW) * 2.0f - 1.0f;
    };
    auto toClipY = [this](float py) {
        return 1.0f - (py / _screenH) * 2.0f;
    };

    // Conversão ARGB para BGRA otimizada
    Drift::Color bgra = ConvertARGBtoBGRA(color);

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

void UIBatcherDX11::AddQuad(float x0, float y0, float x1, float y1,
                            float x2, float y2, float x3, float y3,
                            Drift::Color color) {
    ScissorRect currentScissor = GetCurrentScissorRect();
    if (currentScissor.IsValid()) {
        float minX = std::min({x0, x1, x2, x3});
        float minY = std::min({y0, y1, y2, y3});
        float maxX = std::max({x0, x1, x2, x3});
        float maxY = std::max({y0, y1, y2, y3});
        if (maxX <= currentScissor.x || minX >= currentScissor.x + currentScissor.width ||
            maxY <= currentScissor.y || minY >= currentScissor.y + currentScissor.height) {
            return;
        }
    }

    auto toClipX = [this](float px) { return (px / _screenW) * 2.0f - 1.0f; };
    auto toClipY = [this](float py) { return 1.0f - (py / _screenH) * 2.0f; };

    Drift::Color bgra = ConvertARGBtoBGRA(color);
    unsigned base = (unsigned)_vertices.size();
    _vertices.push_back({toClipX(x0), toClipY(y0), bgra});
    _vertices.push_back({toClipX(x1), toClipY(y1), bgra});
    _vertices.push_back({toClipX(x2), toClipY(y2), bgra});
    _vertices.push_back({toClipX(x3), toClipY(y3), bgra});
    _indices.push_back(base + 0);
    _indices.push_back(base + 1);
    _indices.push_back(base + 2);
    _indices.push_back(base + 2);
    _indices.push_back(base + 3);
    _indices.push_back(base + 0);
}

// Implementação do sistema de renderização de texto com MSDF
void UIBatcherDX11::AddText(float x, float y, const char* text, Drift::Color color) {
    if (!text || !_textRenderer) {
        return;
    }
    
    // Usa o renderizador de texto integrado
    _textRenderer->AddText(x, y, text, color);
}

// Finaliza o batch e envia draw calls para a UI
void UIBatcherDX11::End() {
    // Renderiza elementos básicos (retângulos)
    if (!_vertices.empty() && !_indices.empty()) {
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

    // Renderiza texto se houver renderizador de texto
    if (_textRenderer) {
        _textRenderer->EndTextRendering();
    }
}

void UIBatcherDX11::SetScreenSize(float w, float h) {
    _screenW = w;
    _screenH = h;

    // Propaga para o renderizador de texto
    if (_textRenderer) {
        _textRenderer->SetScreenSize(w, h);
    }
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

void UIBatcherDX11::PushScissorRect(float x, float y, float w, float h) {
    _scissorStack.push_back(ScissorRect(x, y, w, h));
}

void UIBatcherDX11::PopScissorRect() {
    if (!_scissorStack.empty()) {
        _scissorStack.pop_back();
    }
}

void UIBatcherDX11::ClearScissorRects() {
    _scissorStack.clear();
}

bool UIBatcherDX11::IsRectVisible(const ScissorRect& rect) const {
    ScissorRect currentScissor = GetCurrentScissorRect();
    if (!currentScissor.IsValid()) {
        return true; // Sem scissor ativo, tudo é visível
    }
    
    // Verifica se há interseção entre os retângulos
    return !(rect.x + rect.width <= currentScissor.x ||
             rect.x >= currentScissor.x + currentScissor.width ||
             rect.y + rect.height <= currentScissor.y ||
             rect.y >= currentScissor.y + currentScissor.height);
}

ScissorRect UIBatcherDX11::GetCurrentScissorRect() const {
    if (_scissorStack.empty()) {
        return ScissorRect(); // Retorna retângulo inválido
    }
    return _scissorStack.back();
}

ScissorRect UIBatcherDX11::ClipRectToScissor(const ScissorRect& rect, const ScissorRect& scissor) const {
    // Calcula a interseção entre o retângulo e o scissor
    float left = (rect.x > scissor.x) ? rect.x : scissor.x;
    float top = (rect.y > scissor.y) ? rect.y : scissor.y;
    float right = (rect.x + rect.width < scissor.x + scissor.width) ? rect.x + rect.width : scissor.x + scissor.width;
    float bottom = (rect.y + rect.height < scissor.y + scissor.height) ? rect.y + rect.height : scissor.y + scissor.height;
    
    // Se não há interseção, retorna retângulo inválido
    if (left >= right || top >= bottom) {
        return ScissorRect();
    }
    
    // Retorna a interseção
    return ScissorRect(left, top, right - left, bottom - top);
} 