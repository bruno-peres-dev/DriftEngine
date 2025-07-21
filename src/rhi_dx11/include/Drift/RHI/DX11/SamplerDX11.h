#pragma once

#include "Drift/RHI/Texture.h"   // ISampler, SamplerDesc
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Drift::RHI::DX11 {

    class SamplerDX11 : public ISampler {
    public:
        explicit SamplerDX11(ID3D11SamplerState* s) : _s(s) {}
        BackendHandle GetBackendHandle() const override { return _s.Get(); }
    private:
        Microsoft::WRL::ComPtr<ID3D11SamplerState> _s;
    };

    std::shared_ptr<ISampler> CreateSamplerDX11(
        ID3D11Device* dev,
        const SamplerDesc& desc);

} // namespace Drift::RHI::DX11
