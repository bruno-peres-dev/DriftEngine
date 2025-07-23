// src/rhi_dx11/src/PipelineStateDX11.cpp
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include "Drift/RHI/DX11/ShaderDX11.h"
#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/DepthStencilStateDX11.h"
#include "Drift/RHI/Format.h"
#include "Drift/Core/Log.h"

#include <stdexcept>
#include <vector>
#include <unordered_map>

using Microsoft::WRL::ComPtr;
using namespace Drift::RHI;
using namespace Drift::RHI::DX11;

// Converte VertexFormat para DXGI_FORMAT usando o novo sistema tipado
static DXGI_FORMAT VertexFormatToDXGIFormat(VertexFormat format) {
    switch (format) {
        case VertexFormat::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
        case VertexFormat::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
        case VertexFormat::R32_UINT: return DXGI_FORMAT_R32_UINT;
        case VertexFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case VertexFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
        case VertexFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case VertexFormat::R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
        case VertexFormat::R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
        case VertexFormat::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
        case VertexFormat::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
        case VertexFormat::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
        case VertexFormat::R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
        case VertexFormat::R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case VertexFormat::R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
        default:
            Drift::Core::Log("[DX11] WARNING: VertexFormat não suportado, usando UNKNOWN");
            return DXGI_FORMAT_UNKNOWN;
    }
}

// Cria e configura todos os estados fixos do pipeline (shaders, input layout, rasterizer, blend)
PipelineStateDX11::PipelineStateDX11(ID3D11Device* device, const PipelineDesc& desc) {
    // Compila VS/PS/GS com defines
    std::vector<D3D_SHADER_MACRO> macros;
    for (const auto& def : desc.defines) {
        macros.push_back({ def.first.c_str(), def.second.c_str() });
    }
    macros.push_back({ nullptr, nullptr });
    auto vsShader = CreateShaderDX11({ desc.vsFile, desc.vsEntry, "vs_5_0" }, macros.data());
    auto psShader = CreateShaderDX11({ desc.psFile, desc.psEntry, "ps_5_0" }, macros.data());

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

    // Compila Geometry Shader se especificado
    if (!desc.gsFile.empty()) {
        auto gsShader = CreateShaderDX11({ desc.gsFile, desc.gsEntry, "gs_5_0" }, macros.data());
        if (FAILED(device->CreateGeometryShader(
            gsShader->GetBytecode(), gsShader->GetBytecodeSize(),
            nullptr, _gs.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create GeometryShader");
        }
        Drift::Core::Log("[DX11] GeometryShader created successfully: " + desc.gsFile);
    }

    // Cria input layout usando o novo sistema VertexFormat
    std::vector<D3D11_INPUT_ELEMENT_DESC> dxLayout;
    for (const auto& elem : desc.inputLayout) {
        dxLayout.push_back({
            elem.semanticName,                                    // const char* (já é o tipo correto)
            elem.semanticIndex,                                   // uint32_t (já é o tipo correto)
            static_cast<DXGI_FORMAT>(VertexFormatToDXGIFormat(elem.format)), // Cast explícito
            0,                                                    // uint32_t InputSlot
            elem.offset,                                          // uint32_t (já é o tipo correto)
            static_cast<D3D11_INPUT_CLASSIFICATION>(D3D11_INPUT_PER_VERTEX_DATA), // Cast explícito
            0                                                     // uint32_t InstanceDataStepRate
        });
    }
    HRESULT hr = device->CreateInputLayout(
        dxLayout.data(), (UINT)dxLayout.size(),
        vsShader->GetBytecode(), vsShader->GetBytecodeSize(),
        _inputLayout.GetAddressOf());
    if (FAILED(hr)) {
        Drift::Core::Log("[DX11] ERRO: Failed to create InputLayout");
        throw std::runtime_error("Failed to create InputLayout");
    }

    // Configura rasterizer state
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = desc.rasterizer.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
    switch (desc.rasterizer.cullMode) {
        case Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::None:
            rastDesc.CullMode = D3D11_CULL_NONE;
            break;
        case Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::Back:
            rastDesc.CullMode = D3D11_CULL_BACK;
            break;
        case Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::Front:
            rastDesc.CullMode = D3D11_CULL_FRONT;
            break;
        default:
            rastDesc.CullMode = D3D11_CULL_NONE;
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

    hr = device->CreateRasterizerState(&rastDesc, &_rasterizerState);
    if (FAILED(hr)) {
        Drift::Core::Log("[DX11] Failed to create RasterizerState! HRESULT = " + std::to_string(hr));
        throw std::runtime_error("Failed to create RasterizerState");
    }

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

    // Configura depth stencil state usando a nova interface unificada
    _depthStencilState = CreateDepthStencilStateDX11(device, desc.depthStencil);
    if (!_depthStencilState) {
        Drift::Core::Log("[DX11] Failed to create DepthStencilState!");
        throw std::runtime_error("Failed to create DepthStencilState");
    }
}

// Aplica todos os estados do pipeline no contexto DX11
void PipelineStateDX11::Apply(IContext& ctx) {
    auto& dxCtx = static_cast<ContextDX11&>(ctx);
    auto* d3dCtx = dxCtx.GetDeviceContext();

    d3dCtx->IASetInputLayout(_inputLayout.Get());
    d3dCtx->VSSetShader(_vs.Get(), nullptr, 0);
    d3dCtx->PSSetShader(_ps.Get(), nullptr, 0);
    d3dCtx->GSSetShader(_gs.Get(), nullptr, 0); // Bind Geometry Shader (nullptr se não existir)
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
    // Cache de depth stencil usando nova interface
    if (dxCtx._currentDepthStencilState != _depthStencilState) {
        _depthStencilState->Apply(d3dCtx);
        dxCtx._currentDepthStencilState = _depthStencilState;
    }
}

namespace Drift::RHI::DX11 {
    // Fábrica de pipeline state DX11
    std::shared_ptr<IPipelineState> CreatePipelineDX11(ID3D11Device* device, const PipelineDesc& desc) {
        return std::make_shared<PipelineStateDX11>(device, desc);
    }
}
