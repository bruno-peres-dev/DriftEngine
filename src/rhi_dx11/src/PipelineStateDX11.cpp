// src/rhi_dx11/src/PipelineStateDX11.cpp
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include "Drift/RHI/DX11/ShaderDX11.h"
#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/Core/Log.h"

#include <stdexcept>
#include <vector>
#include <unordered_map>

using Microsoft::WRL::ComPtr;
using namespace Drift::RHI;
using namespace Drift::RHI::DX11;

// Converte string de formato para DXGI_FORMAT
static DXGI_FORMAT StringToDXGIFormat(const std::string& fmt) {
    static const std::unordered_map<std::string, DXGI_FORMAT> lut = {
        {"R32G32B32_FLOAT", DXGI_FORMAT_R32G32B32_FLOAT},
        {"R32G32_FLOAT", DXGI_FORMAT_R32G32_FLOAT},
        // Adicione outros formatos conforme necessário
    };
    auto it = lut.find(fmt);
    if (it != lut.end()) return it->second;
    return DXGI_FORMAT_UNKNOWN;
}

// Cria e configura todos os estados fixos do pipeline (shaders, input layout, rasterizer, blend)
PipelineStateDX11::PipelineStateDX11(ID3D11Device* device, const PipelineDesc& desc) {
    // Compila VS/PS com defines
    std::vector<D3D_SHADER_MACRO> macros;
    for (const auto& def : desc.defines) {
        macros.push_back({ def.first.c_str(), def.second.c_str() });
    }
    macros.push_back({ nullptr, nullptr });
    auto vsShader = CreateShaderDX11({ desc.vsFile, "VSMain", "vs_5_0" }, macros.data());
    auto psShader = CreateShaderDX11({ desc.psFile, "PSMain", "ps_5_0" }, macros.data());

    if (FAILED(device->CreateVertexShader(
        vsShader->GetBytecode(), vsShader->GetBytecodeSize(),
        nullptr, _vs.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create VertexShader");
    }
    if (FAILED(device->CreatePixelShader(
        psShader->GetBytecode(), psShader->GetBytecodeSize(),
        nullptr, _ps.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create PixelShader");
    }

    // Cria input layout
    std::vector<D3D11_INPUT_ELEMENT_DESC> dxLayout;
    for (const auto& elem : desc.inputLayout) {
        dxLayout.push_back({
            elem.semanticName.c_str(),
            elem.semanticIndex,
            StringToDXGIFormat(elem.format),
            0,
            elem.offset,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        });
    }
    std::string layoutLog = "[DX11] Criando InputLayout: ";
    for (const auto& elem : desc.inputLayout) {
        layoutLog += elem.semanticName + "(" + std::to_string(elem.offset) + "," + elem.format + ") ";
    }
    Drift::Core::Log(layoutLog);
    HRESULT hr = device->CreateInputLayout(
        dxLayout.data(), (UINT)dxLayout.size(),
        vsShader->GetBytecode(), vsShader->GetBytecodeSize(),
        _inputLayout.GetAddressOf());
    Drift::Core::Log(std::string("[DX11] CreateInputLayout HRESULT = ") + std::to_string(hr));
    if (FAILED(hr)) {
        Drift::Core::Log("[DX11] ERRO: Failed to create InputLayout");
        throw std::runtime_error("Failed to create InputLayout");
    }

    // Configura rasterizer state
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = desc.rasterizer.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
    Drift::Core::Log("[DX11] CullMode enum value = " + std::to_string(static_cast<int>(desc.rasterizer.cullMode)));
    switch (desc.rasterizer.cullMode) {
        case Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::None:
            rastDesc.CullMode = D3D11_CULL_NONE;
            Drift::Core::Log("[DX11] Setting CullMode to D3D11_CULL_NONE");
            break;
        case Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::Back:
            rastDesc.CullMode = D3D11_CULL_BACK;
            Drift::Core::Log("[DX11] Setting CullMode to D3D11_CULL_BACK");
            break;
        case Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::Front:
            rastDesc.CullMode = D3D11_CULL_FRONT;
            Drift::Core::Log("[DX11] Setting CullMode to D3D11_CULL_FRONT");
            break;
        default:
            rastDesc.CullMode = D3D11_CULL_NONE;
            Drift::Core::Log("[DX11] Setting CullMode to D3D11_CULL_NONE (default)");
            break;
    }
    rastDesc.FrontCounterClockwise = FALSE;
    rastDesc.DepthClipEnable = TRUE;
    rastDesc.ScissorEnable = FALSE;
    rastDesc.MultisampleEnable = FALSE;
    rastDesc.AntialiasedLineEnable = FALSE;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.SlopeScaledDepthBias = 0.0f;

    Drift::Core::Log("[DX11] Rasterizer Final State: FillMode=" + std::to_string(rastDesc.FillMode) +
                     " CullMode=" + std::to_string(rastDesc.CullMode) +
                     " FrontCCW=" + std::to_string(rastDesc.FrontCounterClockwise));

    hr = device->CreateRasterizerState(&rastDesc, &_rasterizerState);
    if (FAILED(hr)) {
        Drift::Core::Log("[DX11] Failed to create RasterizerState! HRESULT = " + std::to_string(hr));
        throw std::runtime_error("Failed to create RasterizerState");
    }
    Drift::Core::Log("[DX11] RasterizerState created successfully");

    // Configura blend state
    D3D11_BLEND_DESC blendDesc = {};
    const auto& b = desc.blend;
    blendDesc.AlphaToCoverageEnable = b.alphaToCoverage ? TRUE : FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    auto& rt = blendDesc.RenderTarget[0];
    if (b.enable) {
        auto toDX11 = [](PipelineDesc::BlendDesc::BlendFactor f) {
            switch (f) {
                case PipelineDesc::BlendDesc::BlendFactor::Zero: return D3D11_BLEND_ZERO;
                case PipelineDesc::BlendDesc::BlendFactor::One: return D3D11_BLEND_ONE;
                case PipelineDesc::BlendDesc::BlendFactor::SrcColor: return D3D11_BLEND_SRC_COLOR;
                case PipelineDesc::BlendDesc::BlendFactor::InvSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
                case PipelineDesc::BlendDesc::BlendFactor::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
                case PipelineDesc::BlendDesc::BlendFactor::InvSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
                case PipelineDesc::BlendDesc::BlendFactor::DestAlpha: return D3D11_BLEND_DEST_ALPHA;
                case PipelineDesc::BlendDesc::BlendFactor::InvDestAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
                case PipelineDesc::BlendDesc::BlendFactor::DestColor: return D3D11_BLEND_DEST_COLOR;
                case PipelineDesc::BlendDesc::BlendFactor::InvDestColor: return D3D11_BLEND_INV_DEST_COLOR;
                case PipelineDesc::BlendDesc::BlendFactor::SrcAlphaSaturate: return D3D11_BLEND_SRC_ALPHA_SAT;
                default: return D3D11_BLEND_ONE;
            }
        };
        auto toDX11op = [](PipelineDesc::BlendDesc::BlendOp op) {
            switch (op) {
                case PipelineDesc::BlendDesc::BlendOp::Add: return D3D11_BLEND_OP_ADD;
                case PipelineDesc::BlendDesc::BlendOp::Subtract: return D3D11_BLEND_OP_SUBTRACT;
                case PipelineDesc::BlendDesc::BlendOp::RevSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
                case PipelineDesc::BlendDesc::BlendOp::Min: return D3D11_BLEND_OP_MIN;
                case PipelineDesc::BlendDesc::BlendOp::Max: return D3D11_BLEND_OP_MAX;
                default: return D3D11_BLEND_OP_ADD;
            }
        };
        rt.BlendEnable = TRUE;
        rt.SrcBlend = toDX11(b.srcColor);
        rt.DestBlend = toDX11(b.dstColor);
        rt.BlendOp = toDX11op(b.colorOp);
        if (b.blendFactorSeparate) {
            rt.SrcBlendAlpha = toDX11(b.srcAlpha);
            rt.DestBlendAlpha = toDX11(b.dstAlpha);
            rt.BlendOpAlpha = toDX11op(b.alphaOp);
        } else {
            rt.SrcBlendAlpha = rt.SrcBlend;
            rt.DestBlendAlpha = rt.DestBlend;
            rt.BlendOpAlpha = rt.BlendOp;
        }
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    } else {
        rt.BlendEnable = FALSE;
        rt.SrcBlend = D3D11_BLEND_ONE;
        rt.DestBlend = D3D11_BLEND_ZERO;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }
    hr = device->CreateBlendState(&blendDesc, &_blendState);
    if (FAILED(hr)) {
        Drift::Core::Log("[DX11] Failed to create BlendState! HRESULT = " + std::to_string(hr));
        throw std::runtime_error("Failed to create BlendState");
    }
    Drift::Core::Log("[DX11] BlendState created successfully");
}

// Aplica todos os estados do pipeline no contexto DX11
void PipelineStateDX11::Apply(IContext& ctx) {
    auto& dxCtx = static_cast<ContextDX11&>(ctx);
    auto* d3dCtx = dxCtx.GetDeviceContext();

    d3dCtx->IASetInputLayout(_inputLayout.Get());
    d3dCtx->VSSetShader(_vs.Get(), nullptr, 0);
    d3dCtx->PSSetShader(_ps.Get(), nullptr, 0);
    // Cache de rasterizer
    if (dxCtx._currentRasterizerState != _rasterizerState.Get()) {
        d3dCtx->RSSetState(_rasterizerState.Get());
        dxCtx._currentRasterizerState = _rasterizerState.Get();
    }
    // Cache de blend
    float blendFactor[4] = {1,1,1,1};
    if (dxCtx._currentBlendState != _blendState.Get()) {
        d3dCtx->OMSetBlendState(_blendState.Get(), blendFactor, 0xFFFFFFFF);
        dxCtx._currentBlendState = _blendState.Get();
    }
}

namespace Drift::RHI::DX11 {
    // Fábrica de pipeline state DX11
    std::shared_ptr<IPipelineState> CreatePipelineDX11(ID3D11Device* device, const PipelineDesc& desc) {
        return std::make_shared<PipelineStateDX11>(device, desc);
    }
}
