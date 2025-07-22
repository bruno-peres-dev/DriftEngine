#pragma once

#include "Drift/RHI/Types.h"
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Buffer.h"

namespace Drift::RHI {

    // Interface para listas de comandos (futuro: suporte a command buffers)
    class ICommandList {
    public:
        virtual ~ICommandList() = default;
    };

    // Interface de contexto de renderização (encapsula device context, swapchain, etc)
    class IContext {
    public:
        virtual ~IContext() = default;
        using BackendHandle = void*;

        // Limpa o render target e apresenta o frame
        virtual void Clear(float r, float g, float b, float a) = 0;
        virtual void Present() = 0;

        // Configuração do Input Assembler
        virtual void IASetVertexBuffer(void* vb, UINT stride, UINT offset) = 0;
        virtual void IASetIndexBuffer(void* ib, Format format, UINT offset) = 0;
        virtual void IASetPrimitiveTopology(PrimitiveTopology topo) = 0;
        virtual void DrawIndexed(UINT indexCount, UINT startIndex, INT baseVertex) = 0;
        virtual void Draw(UINT vertexCount, UINT startVertex) = 0;
        virtual void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertex, UINT startInstance) = 0;
        virtual void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndex, INT baseVertex, UINT startInstance) = 0;

        // Redimensiona o swapchain e recursos associados
        virtual void Resize(unsigned width, unsigned height) = 0;

        // Acesso nativo ao device/contexto
        virtual BackendHandle GetNativeDevice()  const = 0;
        virtual BackendHandle GetNativeContext() const = 0;

        // Bind de recursos para pixel shader
        virtual void PSSetTexture(UINT slot, ITexture* tex) = 0;
        virtual void PSSetSampler(UINT slot, ISampler* samp) = 0;
        virtual void SetDepthTestEnabled(bool enabled) = 0;
        virtual void VSSetConstantBuffer(UINT slot, BackendHandle buffer) = 0;
        virtual void PSSetConstantBuffer(UINT slot, BackendHandle buffer) = 0;
        virtual void GSSetConstantBuffer(UINT slot, BackendHandle buffer) = 0;
        
        // Viewport control
        virtual void SetViewport(int x, int y, int width, int height) = 0;
        
        // Atualiza constant buffer (Map/Unmap para dinâmicos, UpdateSubresource para default)
        virtual void UpdateConstantBuffer(IBuffer* buffer, const void* data, size_t size) = 0;

        // Garantir RTV atual para swap-chain com múltiplos buffers
        virtual void BindBackBufferRTV() = 0;
    };

    // Interface para swapchain (controle de buffers de apresentação)
    class ISwapChain {
    public:
        virtual ~ISwapChain() = default;
        // Redimensiona os buffers de apresentação
        virtual void Resize(unsigned width, unsigned height) = 0;
    };

    // Factory functions para implementações stub (Linux)
    std::unique_ptr<IContext> CreateContextStub();
    std::unique_ptr<ISwapChain> CreateSwapChainStub();

} // namespace Drift::RHI
