#include "Drift/RHI/DX11/DepthStencilStateDX11.h"
#include "Drift/RHI/DepthStencilState.h"
#include "Drift/Core/Log.h"
#include <unordered_map>
#include <mutex>

using namespace Drift::RHI::DX11;

// Cache thread-safe para Depth/Stencil States
namespace {
    struct DepthStencilCache {
        std::unordered_map<Drift::RHI::DepthStencilDesc, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>> states;
        std::mutex mutex;
        
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetOrCreate(ID3D11Device* device, const Drift::RHI::DepthStencilDesc& desc) {
            std::lock_guard<std::mutex> lock(mutex);
            
            auto it = states.find(desc);
            if (it != states.end()) {
                return it->second;
            }
            
            // Cria novo estado
            D3D11_DEPTH_STENCIL_DESC d3dDesc = {};
            d3dDesc.DepthEnable = desc.depthEnable ? TRUE : FALSE;
            d3dDesc.DepthWriteMask = desc.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            d3dDesc.DepthFunc = DepthStencilStateDX11::ToD3D11Comparison(desc.depthFunc);
            d3dDesc.StencilEnable = desc.stencilEnable ? TRUE : FALSE;
            d3dDesc.StencilReadMask = desc.stencilReadMask;
            d3dDesc.StencilWriteMask = desc.stencilWriteMask;
            
            // Front face
            d3dDesc.FrontFace.StencilFailOp = DepthStencilStateDX11::ToD3D11StencilOp(desc.frontStencilFailOp);
            d3dDesc.FrontFace.StencilDepthFailOp = DepthStencilStateDX11::ToD3D11StencilOp(desc.frontStencilDepthFailOp);
            d3dDesc.FrontFace.StencilPassOp = DepthStencilStateDX11::ToD3D11StencilOp(desc.frontStencilPassOp);
            d3dDesc.FrontFace.StencilFunc = DepthStencilStateDX11::ToD3D11Comparison(desc.frontStencilFunc);
            
            // Back face
            if (desc.separateBackFace) {
                d3dDesc.BackFace.StencilFailOp = DepthStencilStateDX11::ToD3D11StencilOp(desc.backStencilFailOp);
                d3dDesc.BackFace.StencilDepthFailOp = DepthStencilStateDX11::ToD3D11StencilOp(desc.backStencilDepthFailOp);
                d3dDesc.BackFace.StencilPassOp = DepthStencilStateDX11::ToD3D11StencilOp(desc.backStencilPassOp);
                d3dDesc.BackFace.StencilFunc = DepthStencilStateDX11::ToD3D11Comparison(desc.backStencilFunc);
            } else {
                d3dDesc.BackFace = d3dDesc.FrontFace;
            }
            
            Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;
            HRESULT hr = device->CreateDepthStencilState(&d3dDesc, state.GetAddressOf());
            if (FAILED(hr)) {
                Drift::Core::Log("[DX11] ERRO: Falha ao criar DepthStencilState. HRESULT: " + std::to_string(hr));
                return nullptr;
            }
            
            states[desc] = state;
            return state;
        }
        
        void Clear() {
            std::lock_guard<std::mutex> lock(mutex);
            states.clear();
        }
    };
    
    static DepthStencilCache g_depthStencilCache;
}

DepthStencilStateDX11::DepthStencilStateDX11(ID3D11Device* device, const Drift::RHI::DepthStencilDesc& desc)
    : _desc(desc)
{
    _state = g_depthStencilCache.GetOrCreate(device, desc);
    if (!_state) {
        throw std::runtime_error("Failed to create DepthStencilState");
    }
}

void DepthStencilStateDX11::Apply(void* context) {
    auto* d3dContext = static_cast<ID3D11DeviceContext*>(context);
    if (d3dContext && _state) {
        d3dContext->OMSetDepthStencilState(_state.Get(), _desc.stencilRef);
    }
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

namespace Drift::RHI::DX11 {
std::shared_ptr<Drift::RHI::DepthStencilState> CreateDepthStencilStateDX11(ID3D11Device* device, const Drift::RHI::DepthStencilDesc& desc) {
    return std::make_shared<DepthStencilStateDX11>(device, desc);
}
} // namespace Drift::RHI::DX11 