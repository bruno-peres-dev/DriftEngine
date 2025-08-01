#pragma once

#include "Drift/RHI/Texture.h"   // ISampler, SamplerDesc
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Drift::RHI::DX11 {

    // Implementação DX11 de ISampler
    class SamplerDX11 : public ISampler {
    public:
        explicit SamplerDX11(ID3D11SamplerState* s) : _s(s) {}
        void* GetBackendHandle() const override { return _s.Get(); }
        size_t GetMemoryUsage() const override;
    private:
        Microsoft::WRL::ComPtr<ID3D11SamplerState> _s;
    };

    // Cria um SamplerDX11
    std::shared_ptr<ISampler> CreateSamplerDX11(
        ID3D11Device* dev,
        const SamplerDesc& desc);

} // namespace Drift::RHI::DX11
