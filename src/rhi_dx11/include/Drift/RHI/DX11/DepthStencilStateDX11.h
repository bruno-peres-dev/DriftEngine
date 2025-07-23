#pragma once

#include "Drift/RHI/DepthStencilState.h"
#include "Drift/RHI/ResourceManager.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Drift::RHI::DX11 {

    class DepthStencilStateDX11 : public DepthStencilState {
    public:
        explicit DepthStencilStateDX11(ID3D11Device* device, const Drift::RHI::DepthStencilDesc& desc);
        void Apply(void* context) override;
        const Drift::RHI::DepthStencilDesc& GetDesc() const override;
        void* GetBackendHandle() const override;
        size_t GetMemoryUsage() const override;

        static D3D11_COMPARISON_FUNC ToD3D11Comparison(Drift::RHI::ComparisonFunc func);
        static D3D11_STENCIL_OP ToD3D11StencilOp(Drift::RHI::StencilOp op);
    private:
        Drift::RHI::DepthStencilDesc _desc;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _state;
    };

    // Corrigido: declaração explícita no namespace correto
    std::shared_ptr<Drift::RHI::DepthStencilState> CreateDepthStencilStateDX11(ID3D11Device* device, const Drift::RHI::DepthStencilDesc& desc);

} // namespace Drift::RHI::DX11 