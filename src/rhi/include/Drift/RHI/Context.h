#pragma once

#include "Drift/RHI/Types.h"
#include "Drift/RHI/Texture.h"

namespace Drift::RHI {

    class ICommandList {
    public:
        virtual ~ICommandList() = default;
    };

    class IContext {
    public:
        virtual ~IContext() = default;
        using BackendHandle = void*;

        // Limpeza e apresentação
        virtual void Clear(float r, float g, float b, float a) = 0;
        virtual void Present() = 0;

        // Input Assembler
        virtual void IASetVertexBuffer(void* vb, UINT stride, UINT offset) = 0;
        virtual void IASetIndexBuffer(void* ib, Format format, UINT offset) = 0;
        virtual void IASetPrimitiveTopology(PrimitiveTopology topo) = 0;
        virtual void DrawIndexed(UINT indexCount, UINT startIndex, INT baseVertex) = 0;
        virtual void Draw(UINT vertexCount, UINT startVertex) = 0;
        virtual void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertex, UINT startInstance) = 0;
        virtual void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndex, INT baseVertex, UINT startInstance) = 0;

        // Redimensionamento
        virtual void Resize(unsigned width, unsigned height) = 0;

        // Acesso nativo
        virtual BackendHandle GetNativeDevice()  const = 0;
        virtual BackendHandle GetNativeContext() const = 0;

        // Recursos de pixel shader
        virtual void PSSetTexture(UINT slot, ITexture* tex) = 0;
        virtual void PSSetSampler(UINT slot, ISampler* samp) = 0;
        virtual void SetDepthTestEnabled(bool enabled) = 0;
        virtual void VSSetConstantBuffer(UINT slot, BackendHandle buffer) = 0;
        virtual void PSSetConstantBuffer(UINT slot, BackendHandle buffer) = 0;
    };

    class ISwapChain {
    public:
        virtual ~ISwapChain() = default;
        // Apenas chama IDXGISwapChain::ResizeBuffers
        virtual void Resize(unsigned width, unsigned height) = 0;
    };

} // namespace Drift::RHI
