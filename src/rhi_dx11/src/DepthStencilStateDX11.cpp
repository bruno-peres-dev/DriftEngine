#include "Drift/RHI/DX11/DepthStencilStateDX11.h"
#include "Drift/RHI/DepthStencilState.h"
#include "Drift/RHI/ResourceManager.h"
#include "Drift/Core/Log.h"
#include <unordered_map>
#include <mutex>

using Microsoft::WRL::ComPtr;
using namespace Drift::RHI;
using namespace Drift::RHI::DX11;

namespace Drift::RHI::DX11 {

DepthStencilStateDX11::DepthStencilStateDX11(ID3D11Device* device, const DepthStencilDesc& desc)
    : _desc(desc)
{
    D3D11_DEPTH_STENCIL_DESC d3dDesc = {};
    d3dDesc.DepthEnable = desc.depthEnable ? TRUE : FALSE;
    d3dDesc.DepthWriteMask = desc.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    d3dDesc.DepthFunc = ToD3D11Comparison(desc.depthFunc);
    d3dDesc.StencilEnable = desc.stencilEnable ? TRUE : FALSE;
    d3dDesc.StencilReadMask = desc.stencilReadMask;
    d3dDesc.StencilWriteMask = desc.stencilWriteMask;
    
    // Front face stencil operations
    d3dDesc.FrontFace.StencilFailOp = ToD3D11StencilOp(desc.frontStencilFailOp);
    d3dDesc.FrontFace.StencilDepthFailOp = ToD3D11StencilOp(desc.frontStencilDepthFailOp);
    d3dDesc.FrontFace.StencilPassOp = ToD3D11StencilOp(desc.frontStencilPassOp);
    d3dDesc.FrontFace.StencilFunc = ToD3D11Comparison(desc.frontStencilFunc);
    
    // Back face stencil operations
    if (desc.separateBackFace) {
        d3dDesc.BackFace.StencilFailOp = ToD3D11StencilOp(desc.backStencilFailOp);
        d3dDesc.BackFace.StencilDepthFailOp = ToD3D11StencilOp(desc.backStencilDepthFailOp);
        d3dDesc.BackFace.StencilPassOp = ToD3D11StencilOp(desc.backStencilPassOp);
        d3dDesc.BackFace.StencilFunc = ToD3D11Comparison(desc.backStencilFunc);
    } else {
        d3dDesc.BackFace = d3dDesc.FrontFace;
    }

    HRESULT hr = device->CreateDepthStencilState(&d3dDesc, _state.GetAddressOf());
    if (FAILED(hr)) {
        Drift::Core::Log("[DX11] Failed to create DepthStencilState! HRESULT = " + std::to_string(hr));
        throw std::runtime_error("Failed to create DepthStencilState");
    }
}

void DepthStencilStateDX11::Apply(void* context) {
    auto* d3dContext = static_cast<ID3D11DeviceContext*>(context);
    d3dContext->OMSetDepthStencilState(_state.Get(), _desc.stencilRef);
}

const DepthStencilDesc& DepthStencilStateDX11::GetDesc() const {
    return _desc;
}

void* DepthStencilStateDX11::GetBackendHandle() const {
    return _state.Get();
}

size_t DepthStencilStateDX11::GetMemoryUsage() const {
    // Estimativa do tamanho do DepthStencilState (geralmente pequeno)
    return sizeof(D3D11_DEPTH_STENCIL_DESC) + sizeof(ID3D11DepthStencilState*);
}

D3D11_COMPARISON_FUNC DepthStencilStateDX11::ToD3D11Comparison(ComparisonFunc func) {
    switch (func) {
        case ComparisonFunc::Never: return D3D11_COMPARISON_NEVER;
        case ComparisonFunc::Less: return D3D11_COMPARISON_LESS;
        case ComparisonFunc::Equal: return D3D11_COMPARISON_EQUAL;
        case ComparisonFunc::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
        case ComparisonFunc::Greater: return D3D11_COMPARISON_GREATER;
        case ComparisonFunc::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
        case ComparisonFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
        case ComparisonFunc::Always: return D3D11_COMPARISON_ALWAYS;
        default: return D3D11_COMPARISON_LESS;
    }
}

D3D11_STENCIL_OP DepthStencilStateDX11::ToD3D11StencilOp(StencilOp op) {
    switch (op) {
        case StencilOp::Keep: return D3D11_STENCIL_OP_KEEP;
        case StencilOp::Zero: return D3D11_STENCIL_OP_ZERO;
        case StencilOp::Replace: return D3D11_STENCIL_OP_REPLACE;
        case StencilOp::IncrementSaturate: return D3D11_STENCIL_OP_INCR_SAT;
        case StencilOp::DecrementSaturate: return D3D11_STENCIL_OP_DECR_SAT;
        case StencilOp::Invert: return D3D11_STENCIL_OP_INVERT;
        case StencilOp::Increment: return D3D11_STENCIL_OP_INCR;
        case StencilOp::Decrement: return D3D11_STENCIL_OP_DECR;
        default: return D3D11_STENCIL_OP_KEEP;
    }
}

std::shared_ptr<DepthStencilState> CreateDepthStencilStateDX11(ID3D11Device* device, const DepthStencilDesc& desc) {
    // Usa o Resource Manager para cache
    auto& cache = g_resourceManager.GetCache<DepthStencilDesc, DepthStencilState>(device);
    
    return cache.GetOrCreate(desc, [device, &desc]() -> std::shared_ptr<DepthStencilState> {
        try {
            return std::make_shared<DepthStencilStateDX11>(device, desc);
        } catch (...) {
            return nullptr;
        }
    });
}

} // namespace Drift::RHI::DX11 