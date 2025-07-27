#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/DepthStencilStateDX11.h"
#include "Drift/RHI/RHIException.h"
#include "Drift/RHI/RHIDebug.h"
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
    Drift::Core::LogRHI("Iniciando Context DX11");
    
    // Validar parâmetros
    if (!RHIDebug::ValidateDX11Device(device, "ContextDX11 constructor")) {
        throw DeviceException("Device inválido em ContextDX11 constructor");
    }
    if (!RHIDebug::ValidateDX11Context(context, "ContextDX11 constructor")) {
        throw ContextException("Context inválido em ContextDX11 constructor");
    }
    if (!RHIDebug::ValidatePointer(swapChain, "ContextDX11 constructor - swapChain")) {
        throw RHIException("SwapChain inválido em ContextDX11 constructor");
    }
    RHIDebug::ValidateDimensions(width, height, "ContextDX11 constructor");
    
    if (!_swapChain) {
        Drift::Core::LogRHIError("SwapChain deve ser criado antes do ContextDX11!");
        throw ContextException("SwapChain deve ser criado antes do ContextDX11!");
    }
    
    try {
        CreateRTVandDSV();
        Drift::Core::LogRHI("Context DX11 inicializado com sucesso");
    } catch (const std::exception& e) {
        Drift::Core::LogException("ContextDX11 constructor", e);
        throw;
    }
}

ContextDX11::~ContextDX11() = default;

// Cria RTV (Render Target View) e DSV (Depth Stencil View) e configura viewport
void ContextDX11::CreateRTVandDSV() {
    Drift::Core::LogRHIDebug("Criando RTV e DSV");
    
    if (!RHIDebug::ValidateDX11Device(_device.Get(), "CreateRTVandDSV")) {
        throw DeviceException("Device inválido em CreateRTVandDSV");
    }
    if (!RHIDebug::ValidateDX11Context(_context.Get(), "CreateRTVandDSV")) {
        throw ContextException("Context inválido em CreateRTVandDSV");
    }
    
    // Obter back buffer
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = _swapChain->GetBuffer(
        0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("SwapChain.GetBuffer", hr);
        throw RHIException("Falha ao obter back buffer");
    }
    
    // Criar Render Target View
    hr = _device->CreateRenderTargetView(
        backBuffer.Get(), nullptr, _rtv.GetAddressOf());
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("Device.CreateRenderTargetView", hr);
        throw RHIException("Falha ao criar Render Target View");
    }

    // Configurar descrição da textura de depth
    D3D11_TEXTURE2D_DESC dsvDesc{};
    dsvDesc.Width = _width;
    dsvDesc.Height = _height;
    dsvDesc.MipLevels = 1;
    dsvDesc.ArraySize = 1;
    dsvDesc.Format = ToDXGIFormat(Drift::RHI::Format::D24_UNORM_S8_UINT);
    dsvDesc.SampleDesc = { 1, 0 };
    dsvDesc.Usage = D3D11_USAGE_DEFAULT;
    dsvDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    // Criar textura de depth
    ComPtr<ID3D11Texture2D> depthTex;
    hr = _device->CreateTexture2D(&dsvDesc, nullptr, depthTex.GetAddressOf());
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("Device.CreateTexture2D (depth)", hr);
        throw RHIException("Falha ao criar textura de depth");
    }
    
    // Criar Depth Stencil View
    hr = _device->CreateDepthStencilView(depthTex.Get(), nullptr, _dsv.GetAddressOf());
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("Device.CreateDepthStencilView", hr);
        throw RHIException("Falha ao criar Depth Stencil View");
    }

    // Configurar render targets
    _context->OMSetRenderTargets(1, _rtv.GetAddressOf(), _dsv.Get());
    
    // Configurar viewport
    D3D11_VIEWPORT vp{
        0.0f, 0.0f,
        static_cast<FLOAT>(_width),
        static_cast<FLOAT>(_height),
        0.0f, 1.0f
    };
    _context->RSSetViewports(1, &vp);
    
    Drift::Core::LogRHIDebug("RTV e DSV criados com sucesso");
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
    Core::LogRHIDebug("[ContextDX11] PSSetTexture: slot=" + std::to_string(slot) + 
                     ", texture=" + (tex ? "válida" : "nullptr"));
    
    if (!tex) {
        Core::Log("[ContextDX11][ERRO] PSSetTexture: textura é nullptr para slot " + std::to_string(slot));
        return;
    }
    
    // tex->GetBackendHandle() deve retornar ID3D11ShaderResourceView*
    auto srv = static_cast<ID3D11ShaderResourceView*>(tex->GetBackendHandle());
    if (!srv) {
        Core::Log("[ContextDX11][ERRO] PSSetTexture: ShaderResourceView é nullptr para slot " + std::to_string(slot));
        return;
    }
    
    Core::LogRHIDebug("[ContextDX11] PSSetTexture: SRV válido para slot " + std::to_string(slot) + 
                     " (handle: " + std::to_string(reinterpret_cast<uintptr_t>(srv)) + ")");
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

// === Novos métodos para suporte AAA ===

void ContextDX11::SetPipelineState(IPipelineState* pipeline) {
    if (!pipeline) {
        Drift::Core::LogRHIError("SetPipelineState: pipeline é nullptr");
        return;
    }
    
    if (_currentPipeline == pipeline) {
        return; // Evitar state changes desnecessários
    }
    
    _currentPipeline = pipeline;
    
    // Aplicar pipeline state (implementação específica do DX11)
    // Em uma implementação real, seria necessário fazer bind do pipeline
    Drift::Core::LogRHIDebug("Pipeline state alterado");
}

void ContextDX11::SetPipelineState(std::shared_ptr<IPipelineState> pipeline) {
    SetPipelineState(pipeline.get());
}

void ContextDX11::SetSampler(UINT slot, ISampler* sampler) {
    if (!sampler) {
        Drift::Core::LogRHIError("SetSampler: sampler é nullptr");
        return;
    }
    
    // Verificar se o sampler já está bound neste slot
    if (slot < _boundSamplers.size() && _boundSamplers[slot] == sampler) {
        return; // Evitar state changes desnecessários
    }
    
    // Garantir que o vector tem tamanho suficiente
    if (slot >= _boundSamplers.size()) {
        _boundSamplers.resize(slot + 1, nullptr);
    }
    
    _boundSamplers[slot] = sampler;
    
    // Aplicar sampler (implementação específica do DX11)
    PSSetSampler(slot, sampler);
}

void ContextDX11::SetSampler(UINT slot, std::shared_ptr<ISampler> sampler) {
    SetSampler(slot, sampler.get());
}

void ContextDX11::PSSetTextureArray(UINT startSlot, UINT count, ITexture** textures) {
    if (!textures) {
        Drift::Core::LogRHIError("PSSetTextureArray: textures é nullptr");
        return;
    }
    
    // Verificar se as texturas já estão bound
    bool needsUpdate = false;
    for (UINT i = 0; i < count; ++i) {
        UINT slot = startSlot + i;
        if (slot >= _boundTextures.size() || _boundTextures[slot] != textures[i]) {
            needsUpdate = true;
            break;
        }
    }
    
    if (!needsUpdate) {
        return; // Evitar state changes desnecessários
    }
    
    // Garantir que o vector tem tamanho suficiente
    if (startSlot + count > _boundTextures.size()) {
        _boundTextures.resize(startSlot + count, nullptr);
    }
    
    // Atualizar cache e aplicar texturas
    for (UINT i = 0; i < count; ++i) {
        UINT slot = startSlot + i;
        _boundTextures[slot] = textures[i];
        PSSetTexture(slot, textures[i]);
    }
}

void ContextDX11::PSSetSamplerArray(UINT startSlot, UINT count, ISampler** samplers) {
    if (!samplers) {
        Drift::Core::LogRHIError("PSSetSamplerArray: samplers é nullptr");
        return;
    }
    
    // Verificar se os samplers já estão bound
    bool needsUpdate = false;
    for (UINT i = 0; i < count; ++i) {
        UINT slot = startSlot + i;
        if (slot >= _boundSamplers.size() || _boundSamplers[slot] != samplers[i]) {
            needsUpdate = true;
            break;
        }
    }
    
    if (!needsUpdate) {
        return; // Evitar state changes desnecessários
    }
    
    // Garantir que o vector tem tamanho suficiente
    if (startSlot + count > _boundSamplers.size()) {
        _boundSamplers.resize(startSlot + count, nullptr);
    }
    
    // Atualizar cache e aplicar samplers
    for (UINT i = 0; i < count; ++i) {
        UINT slot = startSlot + i;
        _boundSamplers[slot] = samplers[i];
        PSSetSampler(slot, samplers[i]);
    }
}

void ContextDX11::SetScissorRect(int x, int y, int width, int height) {
    D3D11_RECT rect = { x, y, x + width, y + height };
    
    // Verificar se o scissor rect mudou
    if (_currentScissorRect.left == rect.left &&
        _currentScissorRect.top == rect.top &&
        _currentScissorRect.right == rect.right &&
        _currentScissorRect.bottom == rect.bottom) {
        return; // Evitar state changes desnecessários
    }
    
    _currentScissorRect = rect;
    _context->RSSetScissorRects(1, &rect);
    
    Drift::Core::LogRHIDebug("Scissor rect definido: " + std::to_string(width) + "x" + std::to_string(height));
}

void ContextDX11::SetBlendFactor(float r, float g, float b, float a) {
    _blendFactor[0] = r;
    _blendFactor[1] = g;
    _blendFactor[2] = b;
    _blendFactor[3] = a;
    
    // Aplicar blend factor
    _context->OMSetBlendState(_currentBlendState, _blendFactor, 0xFFFFFFFF);
}

void ContextDX11::SetStencilRef(UINT ref) {
    if (_stencilRef == ref) {
        return; // Evitar state changes desnecessários
    }
    
    _stencilRef = ref;
    
    // Aplicar stencil reference
    if (_currentDepthStencilState) {
        // Em uma implementação real, seria necessário aplicar o stencil ref
        Drift::Core::LogRHIDebug("Stencil ref alterado: " + std::to_string(ref));
    }
}

void ContextDX11::ExecuteCommandList(ID3D11CommandList* commandList) {
    if (!commandList) {
        Drift::Core::LogRHIError("ExecuteCommandList: commandList é nullptr");
        return;
    }
    
    _context->ExecuteCommandList(commandList, FALSE);
    Drift::Core::LogRHIDebug("Command list executado");
}

void ContextDX11::FinishCommandList(ID3D11CommandList** commandList) {
    if (!commandList) {
        Drift::Core::LogRHIError("FinishCommandList: commandList é nullptr");
        return;
    }
    
    HRESULT hr = _context->FinishCommandList(FALSE, commandList);
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("FinishCommandList", hr);
        throw RHIException("Falha ao finalizar command list");
    }
    
    Drift::Core::LogRHIDebug("Command list finalizado");
}

void ContextDX11::BeginQuery(ID3D11Query* query) {
    if (!query) {
        Drift::Core::LogRHIError("BeginQuery: query é nullptr");
        return;
    }
    
    _context->Begin(query);
}

void ContextDX11::EndQuery(ID3D11Query* query) {
    if (!query) {
        Drift::Core::LogRHIError("EndQuery: query é nullptr");
        return;
    }
    
    _context->End(query);
}

void ContextDX11::GetData(ID3D11Query* query, void* data, UINT dataSize, UINT flags) {
    if (!query) {
        Drift::Core::LogRHIError("GetData: query é nullptr");
        return;
    }
    
    if (!data) {
        Drift::Core::LogRHIError("GetData: data é nullptr");
        return;
    }
    
    HRESULT hr = _context->GetData(query, data, dataSize, flags);
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("GetData", hr);
        throw RHIException("Falha ao obter dados da query");
    }
}
