// src/rhi_dx11/src/SamplerDX11.cpp

#include "Drift/RHI/DX11/SamplerDX11.h"
#include "Drift/RHI/Texture.h"       // ISampler, SamplerDesc
#include <stdexcept>

using namespace Drift::RHI::DX11;

namespace Drift::RHI::DX11 {

// Cria um sampler state DX11 padrão (linear, wrap)
std::shared_ptr<Drift::RHI::ISampler> CreateSamplerDX11(
    ID3D11Device* dev,
    const Drift::RHI::SamplerDesc& /*desc*/)
{
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;

    Microsoft::WRL::ComPtr<ID3D11SamplerState> s;
    if (FAILED(dev->CreateSamplerState(&sd, s.GetAddressOf())))
        throw std::runtime_error("Falha ao criar sampler");

    return std::make_shared<SamplerDX11>(s.Get());
}

} // namespace Drift::RHI::DX11
