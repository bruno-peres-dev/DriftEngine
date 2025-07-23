#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/DepthStencilStateDX11.h"
#include <stdexcept>
#include <wrl/client.h>
#include <cstring> // memcpy
#include <d3d11.h>
#include "Drift/Core/Log.h"
#include <unordered_map>
#include <mutex>

using namespace Drift::RHI::DX11;
using Microsoft::WRL::ComPtr;

// Construtor: inicializa contexto, swapchain e cria RTV/DSV + viewport
ContextDX11::ContextDX11(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    IDXGISwapChain* swapChain,
    unsigned width,
    unsigned height,
    bool vsync
) : _device(device)
, _context(context)
, _swapChain(swapChain)
, _width(width)
, _height(height)
, _vsync(vsync)
{
    if (!_swapChain) {
        throw std::runtime_error("[DX11] Erro: SwapChain deve ser criado antes do ContextDX11!");
    }
    CreateRTVandDSV();
}

ContextDX11::~ContextDX11() = default;

// Cria RTV (Render Target View) e DSV (Depth Stencil View) e configura viewport
void ContextDX11::CreateRTVandDSV() {
    ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(_swapChain->GetBuffer(
        0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
    {
        throw std::runtime_error("Failed to get back buffer");
    }
    if (FAILED(_device->CreateRenderTargetView(
        backBuffer.Get(), nullptr, _rtv.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create RTV");
    }

    D3D11_TEXTURE2D_DESC dsvDesc{};
    dsvDesc.Width = _width;
    dsvDesc.Height = _height;
    dsvDesc.MipLevels = 1;
    dsvDesc.ArraySize = 1;
    dsvDesc.Format = ToDXGIFormat(Drift::RHI::Format::D24_UNORM_S8_UINT);
    dsvDesc.SampleDesc = { 1, 0 };
    dsvDesc.Usage = D3D11_USAGE_DEFAULT;
    dsvDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthTex;
    if (FAILED(_device->CreateTexture2D(
        &dsvDesc, nullptr, depthTex.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create depth texture");
    }
    if (FAILED(_device->CreateDepthStencilView(
        depthTex.Get(), nullptr, _dsv.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create DSV");
    }

    _context->OMSetRenderTargets(1, _rtv.GetAddressOf(), _dsv.Get());
    D3D11_VIEWPORT vp{
        0.0f, 0.0f,
        static_cast<FLOAT>(_width),
        static_cast<FLOAT>(_height),
        0.0f, 1.0f
    };
    _context->RSSetViewports(1, &vp);
}

void ContextDX11::Clear(float r, float g, float b, float a) {
    const float col[4]{ r, g, b, a };
    _context->ClearRenderTargetView(_rtv.Get(), col);
    _context->ClearDepthStencilView(
        _dsv.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f, 0
    );
}

void ContextDX11::Present() {
    _swapChain->Present(_vsync ? 1 : 0, 0);
}

void ContextDX11::IASetVertexBuffer(void* vb, UINT stride, UINT offset) {
    _context->IASetVertexBuffers(
        0, 1,
        reinterpret_cast<ID3D11Buffer* const*>(&vb),
        &stride, &offset
    );
}

void ContextDX11::IASetIndexBuffer(void* ib, Drift::RHI::Format fmt, UINT offset) {
    _context->IASetIndexBuffer(
        reinterpret_cast<ID3D11Buffer*>(ib),
        ToDXGIFormat(fmt),
        offset
    );
}

void ContextDX11::IASetPrimitiveTopology(Drift::RHI::PrimitiveTopology topo) {
    _context->IASetPrimitiveTopology(ToD3DTopology(topo));
}

void ContextDX11::DrawIndexed(UINT indexCount, UINT startIndex, INT baseVertex) {
    _context->DrawIndexed(indexCount, startIndex, baseVertex);
}

void ContextDX11::Draw(UINT vertexCount, UINT startVertex) {
    _context->Draw(vertexCount, startVertex);
}

// Instancing
void ContextDX11::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertex, UINT startInstance) {
    _context->DrawInstanced(vertexCountPerInstance, instanceCount, startVertex, startInstance);
}
void ContextDX11::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndex, INT baseVertex, UINT startInstance) {
    _context->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndex, baseVertex, startInstance);
}

// Atualiza e faz bind de constant buffer dinâmico
void ContextDX11::UpdateConstantBuffer(
    ID3D11Buffer* buffer,
    const void* data,
    UINT size,
    UINT slot
) {
    D3D11_MAPPED_SUBRESOURCE mapped;
    _context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    std::memcpy(mapped.pData, data, size);
    _context->Unmap(buffer, 0);
    _context->VSSetConstantBuffers(slot, 1, &buffer);
    _context->PSSetConstantBuffers(slot, 1, &buffer);
}

// Redimensiona swapchain, RTV/DSV e viewport
void ContextDX11::Resize(unsigned width, unsigned height) {
    if (width == 0 || height == 0) {
        return;
    }
    _width = width;
    _height = height;

    // Desfaz o bind dos render targets antes do resize
    _context->OMSetRenderTargets(0, nullptr, nullptr);

    _rtv.Reset();
    _dsv.Reset();

    HRESULT hr = _swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        HRESULT removedReason = _device->GetDeviceRemovedReason();
        Drift::Core::Log("[DX11] DeviceRemovedReason = " + std::to_string(removedReason));
        Drift::Core::Log("[DX11] ERRO: SwapChain->ResizeBuffers falhou");
        throw std::runtime_error("SwapChain->ResizeBuffers falhou");
    }
    CreateRTVandDSV();
}

void ContextDX11::PSSetTexture(UINT slot, ITexture* tex) {
    // tex->GetBackendHandle() deve retornar ID3D11ShaderResourceView*
    auto srv = static_cast<ID3D11ShaderResourceView*>(tex->GetBackendHandle());
    if (!srv) {
        Drift::Core::Log("[DX11][ERRO] PSSetTexture: ShaderResourceView é nullptr!");
        return;
    }
    _context->PSSetShaderResources(slot, 1, &srv);
}

void ContextDX11::PSSetSampler(UINT slot, ISampler* samp) {
    // samp->GetBackendHandle() deve retornar ID3D11SamplerState*
    auto s = static_cast<ID3D11SamplerState*>(samp->GetBackendHandle());
    if (!s) {
        Drift::Core::Log("[DX11][ERRO] PSSetSampler: SamplerState é nullptr!");
        return;
    }
    _context->PSSetSamplers(slot, 1, &s);
}

// Cache global removido - agora usa a interface unificada IDepthStencilState

void ContextDX11::SetDepthTestEnabled(bool enabled) {
    // Usa a nova interface unificada de Depth/Stencil
    DepthStencilDesc dsDesc;
    dsDesc.depthEnable = enabled;
    dsDesc.depthWrite = enabled;
    dsDesc.depthFunc = ComparisonFunc::Less;
    dsDesc.stencilEnable = false;
    
    auto dsState = CreateDepthStencilStateDX11(_device.Get(), dsDesc);
    if (!dsState) {
        Drift::Core::Log("[DX11] ERRO: Falha ao criar DepthStencilState para SetDepthTestEnabled");
        return;
    }
    
    if (!_currentDepthStencilState || _currentDepthStencilState != dsState) {
        dsState->Apply(_context.Get());
        _currentDepthStencilState = dsState;
    }
}

void ContextDX11::SetViewport(int x, int y, int width, int height) {
    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = static_cast<float>(x);
    vp.TopLeftY = static_cast<float>(y);
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    
    _context->RSSetViewports(1, &vp);
}

void ContextDX11::UpdateConstantBuffer(IBuffer* buffer, const void* data, size_t size) {
    if (!buffer || !data || size == 0) {
        Drift::Core::Log("[DX11][ERRO] UpdateConstantBuffer: parâmetros inválidos!");
        return;
    }
    
    auto* d3dBuffer = static_cast<ID3D11Buffer*>(buffer->GetBackendHandle());
    if (!d3dBuffer) {
        Drift::Core::Log("[DX11][ERRO] UpdateConstantBuffer: buffer D3D11 é nullptr!");
        return;
    }
    
    // Detecta se o buffer é dinâmico (CPUAccessWrite) usando Desc
    D3D11_BUFFER_DESC bd{};
    d3dBuffer->GetDesc(&bd);
    if (bd.Usage == D3D11_USAGE_DYNAMIC) {
        // Usa Map/Unmap para buffers dinâmicos
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(_context->Map(d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            Drift::Core::Log("[DX11][ERRO] UpdateConstantBuffer: Map falhou!");
            return;
        }
        std::memcpy(mapped.pData, data, size);
        _context->Unmap(d3dBuffer, 0);
    } else {
        // Para buffers DEFAULT, usa UpdateSubresource
        _context->UpdateSubresource(d3dBuffer, 0, nullptr, data, 0, 0);
    }
}

// Conversão de enums para DXGI/D3D11
DXGI_FORMAT ContextDX11::ToDXGIFormat(Drift::RHI::Format fmt) {
    switch (fmt) {
    case Drift::RHI::Format::R8G8B8A8_UNORM:    return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Drift::RHI::Format::R16_UINT:          return DXGI_FORMAT_R16_UINT;
    case Drift::RHI::Format::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case Drift::RHI::Format::R32_UINT:          return DXGI_FORMAT_R32_UINT;
    default:                                    return DXGI_FORMAT_UNKNOWN;
    }
}

D3D11_PRIMITIVE_TOPOLOGY ContextDX11::ToD3DTopology(Drift::RHI::PrimitiveTopology topo) {
    switch (topo) {
    case Drift::RHI::PrimitiveTopology::PointList:     return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    case Drift::RHI::PrimitiveTopology::TriangleList:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case Drift::RHI::PrimitiveTopology::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case Drift::RHI::PrimitiveTopology::LineList:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case Drift::RHI::PrimitiveTopology::LineStrip:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    default:                                           return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }
}

void ContextDX11::VSSetConstantBuffer(UINT slot, BackendHandle buffer) {
    ID3D11Buffer* buf = reinterpret_cast<ID3D11Buffer*>(buffer);
    _context->VSSetConstantBuffers(slot, 1, &buf);
}
void ContextDX11::PSSetConstantBuffer(UINT slot, BackendHandle buffer) {
    ID3D11Buffer* buf = reinterpret_cast<ID3D11Buffer*>(buffer);
    _context->PSSetConstantBuffers(slot, 1, &buf);
}
void ContextDX11::GSSetConstantBuffer(UINT slot, BackendHandle buffer) {
    ID3D11Buffer* buf = reinterpret_cast<ID3D11Buffer*>(buffer);
    _context->GSSetConstantBuffers(slot, 1, &buf);
}

// Define render target e depth (pode ser backbuffer ou custom)
void ContextDX11::SetRenderTarget(ITexture* color, ITexture* depth) {
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;
    if (color) {
        // Supondo que GetBackendHandle retorna ID3D11RenderTargetView* para render targets
        rtv = static_cast<ID3D11RenderTargetView*>(color->GetBackendHandle());
        if (!rtv) {
            Drift::Core::Log("[DX11][ERRO] SetRenderTarget: RenderTargetView é nullptr!");
            return;
        }
    } else if (_rtv) {
        rtv = _rtv.Get(); // fallback para backbuffer
    }
    if (depth) {
        dsv = static_cast<ID3D11DepthStencilView*>(depth->GetBackendHandle());
        if (!dsv) {
            Drift::Core::Log("[DX11][ERRO] SetRenderTarget: DepthStencilView é nullptr!");
            return;
        }
    } else if (_dsv) {
        dsv = _dsv.Get();
    }
    _context->OMSetRenderTargets(1, &rtv, dsv);
}

// Debug helpers (logs)
void ContextDX11::SetDebugLabel(const char* label) {
    Drift::Core::Log(std::string("[DX11] SetDebugLabel: ") + (label ? label : ""));
}
void ContextDX11::BeginDebugEvent(const char* name) {
    Drift::Core::Log(std::string("[DX11] BeginDebugEvent: ") + (name ? name : ""));
}
void ContextDX11::EndDebugEvent() {
    Drift::Core::Log("[DX11] EndDebugEvent");
}

void ContextDX11::BindBackBufferRTV() {
    // Obter o buffer atual da swapchain e recriar RTV se mudou
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuf;
    if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuf.GetAddressOf())))) {
        Drift::Core::Log("[DX11][ERRO] BindBackBufferRTV: GetBuffer falhou");
        return;
    }

    // Compare pointers
    ComPtr<ID3D11Resource> prevRes;
    if (_rtv) _rtv->GetResource(prevRes.GetAddressOf());
    if (prevRes.Get() != backBuf.Get()) {
        // Need new RTV
        _rtv.Reset();
        HRESULT hr = _device->CreateRenderTargetView(backBuf.Get(), nullptr, _rtv.GetAddressOf());
        if (FAILED(hr)) {
            Drift::Core::Log("[DX11][ERRO] BindBackBufferRTV: CreateRenderTargetView falhou");
            return;
        }
    }

    _context->OMSetRenderTargets(1, _rtv.GetAddressOf(), _dsv.Get());
}
