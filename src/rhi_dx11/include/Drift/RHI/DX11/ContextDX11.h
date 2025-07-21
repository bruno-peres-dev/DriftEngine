#pragma once

#include "Drift/RHI/Context.h"     // IContext, ISwapChain
#include "Drift/RHI/Types.h"       // Format, PrimitiveTopology
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

namespace Drift::RHI::DX11 {

    /// Contexto DX11: encapsula ID3D11DeviceContext, RTV/DSV e viewport.
    class ContextDX11 : public Drift::RHI::IContext {
    public:
        ContextDX11(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            IDXGISwapChain* swapChain,
            unsigned width,
            unsigned height,
            bool vsync = true
        );
        ~ContextDX11() override;

        // Limpa e apresenta
        void Clear(float r, float g, float b, float a) override;
        void Present() override;

        // Input Assembler
        void IASetVertexBuffer(void* vb, UINT stride, UINT offset) override;
        void IASetIndexBuffer(void* ib, Drift::RHI::Format format, UINT offset) override;
        void IASetPrimitiveTopology(Drift::RHI::PrimitiveTopology topo) override;
        void DrawIndexed(UINT indexCount, UINT startIndex, INT baseVertex) override;
        void Draw(UINT vertexCount, UINT startVertex) override;
        // Instancing
        void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertex, UINT startInstance) override;
        void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndex, INT baseVertex, UINT startInstance) override;

        // Atualiza e liga um constant buffer dinâmico (slot bN)
        void UpdateConstantBuffer(
            ID3D11Buffer* buffer,
            const void* data,
            UINT size,
            UINT slot
        );

        // Redimensionamento: ajusta width/height, resize swap‑chain e recria RTV/DSV + viewport
        void Resize(unsigned width, unsigned height) override;

        virtual void PSSetTexture(UINT slot, ITexture* tex) override;
        virtual void PSSetSampler(UINT slot, ISampler* samp) override;
        void VSSetConstantBuffer(UINT slot, BackendHandle buffer) override;
        void PSSetConstantBuffer(UINT slot, BackendHandle buffer) override;

        void SetDepthTestEnabled(bool enabled) override;
        void SetRenderTarget(ITexture* color, ITexture* depth = nullptr);
        void SetDebugLabel(const char* label);
        void BeginDebugEvent(const char* name);
        void EndDebugEvent();

        // Acesso nativo
        void* GetNativeDevice()  const override { return _device.Get(); }
        void* GetNativeContext() const override { return _context.Get(); }
        ID3D11DeviceContext* GetDeviceContext() const { return _context.Get(); }

    private:
        friend class PipelineStateDX11;
        Microsoft::WRL::ComPtr<ID3D11Device>           _device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext>    _context;
        Microsoft::WRL::ComPtr<IDXGISwapChain>         _swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _rtv;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _dsv;
        unsigned                                       _width;
        unsigned                                       _height;
        bool _vsync = true;
        // Cache de estados
        ID3D11BlendState*      _currentBlendState = nullptr;
        ID3D11RasterizerState* _currentRasterizerState = nullptr;
        ID3D11DepthStencilState* _currentDepthStencilState = nullptr;

        void CreateRTVandDSV();

        static DXGI_FORMAT             ToDXGIFormat(Drift::RHI::Format fmt);
        static D3D11_PRIMITIVE_TOPOLOGY ToD3DTopology(Drift::RHI::PrimitiveTopology topo);
    };

} // namespace Drift::RHI::DX11
