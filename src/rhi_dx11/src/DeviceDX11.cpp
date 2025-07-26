#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/ResourceManager.h"
#include "Drift/Core/Log.h"

#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/SwapChainDX11.h"
#include "Drift/RHI/DX11/BufferDX11.h"
#include "Drift/RHI/DX11/PipelineStateDX11.h"
#include "Drift/RHI/DX11/ShaderDX11.h"
#include "Drift/RHI/DX11/TextureDX11.h"
#include "Drift/RHI/DX11/SamplerDX11.h"

#include <stdexcept>
using Microsoft::WRL::ComPtr;
using namespace Drift::RHI;
using namespace Drift::RHI::DX11;

// Inicializa o dispositivo DX11 e contexto principal
DeviceDX11::DeviceDX11(const DeviceDesc& desc)
    : _desc(desc)
{
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        flags, levels, _countof(levels),
        D3D11_SDK_VERSION,
        _device.GetAddressOf(),
        &featureLevel,
        _context.GetAddressOf());
    // D3D11CreateDevice HRESULT = " + std::to_string(hr)
    // FeatureLevel = " + std::to_string(featureLevel)
    if (FAILED(hr)) {
        throw std::runtime_error("Falha ao criar D3D11Device");
    }
}

DeviceDX11::~DeviceDX11() {
    // Remove este dispositivo do Resource Manager
    g_resourceManager.RemoveDevice(_device.Get());
}

// Cria contexto de renderização associado ao swapchain
std::shared_ptr<Drift::RHI::IContext> DeviceDX11::CreateContext() {
    if (!_swapChain)
        throw std::runtime_error("SwapChain não criada antes de CreateContext");
    return std::make_shared<ContextDX11>(
        _device.Get(), _context.Get(),
        _swapChain.Get(),
        _desc.width, _desc.height,
        _desc.vsync
    );
}

// Cria swapchain para a janela especificada
std::shared_ptr<ISwapChain> DeviceDX11::CreateSwapChain(void* hwnd) {
    ComPtr<IDXGIDevice>  dxgiDev;
    ComPtr<IDXGIAdapter> dxgiAdap;
    ComPtr<IDXGIFactory> factory;

    if (FAILED(_device.As(&dxgiDev)) ||
        FAILED(dxgiDev->GetAdapter(dxgiAdap.GetAddressOf())) ||
        FAILED(dxgiAdap->GetParent(__uuidof(IDXGIFactory),
            reinterpret_cast<void**>(factory.GetAddressOf()))))
    {
        throw std::runtime_error("Falha ao obter DXGI Factory");
    }

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = _desc.width;
    scd.BufferDesc.Height = _desc.height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = static_cast<HWND>(hwnd);
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = 0;

    ComPtr<IDXGISwapChain> sc;
    HRESULT hr = factory->CreateSwapChain(_device.Get(), &scd, sc.GetAddressOf());
    if (FAILED(hr))
        throw std::runtime_error("Falha ao criar SwapChain");

    _swapChain = sc;
    return std::make_shared<SwapChainDX11>(_swapChain.Get());
}

// Criação e cache de recursos usando o Resource Manager
std::shared_ptr<IBuffer> DeviceDX11::CreateBuffer(const BufferDesc& d) {
    auto& cache = g_resourceManager.GetCache<BufferDesc, IBuffer>(_device.Get());
    return cache.GetOrCreate(d, [this, &d]() {
        return CreateBufferDX11(_device.Get(), _context.Get(), d);
    });
}

std::shared_ptr<IPipelineState> DeviceDX11::CreatePipeline(const PipelineDesc& d) {
    auto& cache = g_resourceManager.GetCache<PipelineDesc, IPipelineState>(_device.Get());
    return cache.GetOrCreate(d, [this, &d]() {
        return CreatePipelineDX11(_device.Get(), d);
    });
}

std::shared_ptr<IShader> DeviceDX11::CreateShader(const ShaderDesc& d) {
    auto& cache = g_resourceManager.GetCache<ShaderDesc, IShader>(_device.Get());
    return cache.GetOrCreate(d, [&d]() {
        return CreateShaderDX11(d);
    });
}

std::shared_ptr<ITexture> DeviceDX11::CreateTexture(const TextureDesc& d) {
    auto& cache = g_resourceManager.GetCache<TextureDesc, ITexture>(_device.Get());
    return cache.GetOrCreate(d, [this, &d]() {
        return CreateTextureDX11(_device.Get(), _context.Get(), d);
    });
}

std::shared_ptr<ISampler> DeviceDX11::CreateSampler(const SamplerDesc& d) {
    auto& cache = g_resourceManager.GetCache<SamplerDesc, ISampler>(_device.Get());
    return cache.GetOrCreate(d, [this, &d]() {
        return CreateSamplerDX11(_device.Get(), d);
    });
}

// Métodos para gerenciamento de recursos
void DeviceDX11::ClearResourceCaches() {
    g_resourceManager.RemoveDevice(_device.Get());
}

ResourceManager::GlobalStats DeviceDX11::GetResourceStats() const {
    return g_resourceManager.GetGlobalStats();
}
