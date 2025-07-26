#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/ResourceManager.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/RHIException.h"
#include "Drift/RHI/RHIDebug.h"

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
    Drift::Core::LogRHI("Iniciando criação do Device DX11");
    
    // Validar dimensões
    RHIDebug::ValidateDimensions(desc.width, desc.height, "DeviceDX11 constructor");
    
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    Drift::Core::LogRHIDebug("Device DX11 criado com flags de debug");
#endif

    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    
    Drift::Core::LogRHIDebug("Chamando D3D11CreateDevice...");
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        flags, levels, _countof(levels),
        D3D11_SDK_VERSION,
        _device.GetAddressOf(),
        &featureLevel,
        _context.GetAddressOf());
    
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("D3D11CreateDevice", hr);
        throw DeviceException("Falha ao criar D3D11Device");
    }
    
    // Validar device e context criados
    if (!RHIDebug::ValidateDX11Device(_device.Get(), "DeviceDX11 constructor")) {
        throw DeviceException("Device inválido após criação");
    }
    if (!RHIDebug::ValidateDX11Context(_context.Get(), "DeviceDX11 constructor")) {
        throw ContextException("Context inválido após criação");
    }
    
    Drift::Core::LogRHI("Device DX11 criado com sucesso. FeatureLevel: " + std::to_string(featureLevel));
}

DeviceDX11::~DeviceDX11() {
    // Remove este dispositivo do Resource Manager
    g_resourceManager.RemoveDevice(_device.Get());
}

// Cria contexto de renderização associado ao swapchain
std::shared_ptr<Drift::RHI::IContext> DeviceDX11::CreateContext() {
    Drift::Core::LogRHI("Criando Context DX11");
    
    if (!RHIDebug::ValidateDX11Device(_device.Get(), "CreateContext")) {
        throw DeviceException("Device inválido em CreateContext");
    }
    if (!RHIDebug::ValidateDX11Context(_context.Get(), "CreateContext")) {
        throw ContextException("Context inválido em CreateContext");
    }
    
    if (!_swapChain) {
        Drift::Core::LogRHIError("SwapChain não criada antes de CreateContext");
        throw SwapChainException("SwapChain não criada antes de CreateContext");
    }
    
    try {
        auto context = std::make_shared<ContextDX11>(
            _device.Get(), _context.Get(),
            _swapChain.Get(),
            _desc.width, _desc.height,
            _desc.vsync
        );
        
        Drift::Core::LogRHI("Context DX11 criado com sucesso");
        return context;
    } catch (const std::exception& e) {
        Drift::Core::LogException("CreateContext", e);
        throw;
    }
}

// Cria swapchain para a janela especificada
std::shared_ptr<ISwapChain> DeviceDX11::CreateSwapChain(void* hwnd) {
    Drift::Core::LogRHI("Criando SwapChain DX11");
    
    if (!RHIDebug::ValidateDX11Device(_device.Get(), "CreateSwapChain")) {
        throw DeviceException("Device inválido em CreateSwapChain");
    }
    if (!RHIDebug::ValidatePointer(hwnd, "CreateSwapChain - hwnd")) {
        throw RHIException("HWND inválido em CreateSwapChain");
    }
    
    ComPtr<IDXGIDevice>  dxgiDev;
    ComPtr<IDXGIAdapter> dxgiAdap;
    ComPtr<IDXGIFactory> factory;

    // Obter DXGI Device
    HRESULT hr = _device.As(&dxgiDev);
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("Device.As(IDXGIDevice)", hr);
        throw RHIException("Falha ao obter DXGI Device");
    }
    
    // Obter DXGI Adapter
    hr = dxgiDev->GetAdapter(dxgiAdap.GetAddressOf());
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("DXGIDevice.GetAdapter", hr);
        throw RHIException("Falha ao obter DXGI Adapter");
    }
    
    // Obter DXGI Factory
    hr = dxgiAdap->GetParent(__uuidof(IDXGIFactory),
        reinterpret_cast<void**>(factory.GetAddressOf()));
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("DXGIAdapter.GetParent(IDXGIFactory)", hr);
        throw RHIException("Falha ao obter DXGI Factory");
    }

    // Configurar descrição da SwapChain
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

    Drift::Core::LogRHIDebug("Criando SwapChain: " + std::to_string(_desc.width) + "x" + std::to_string(_desc.height));
    
    ComPtr<IDXGISwapChain> sc;
    hr = factory->CreateSwapChain(_device.Get(), &scd, sc.GetAddressOf());
    if (FAILED(hr)) {
        Drift::Core::LogHRESULT("IDXGIFactory.CreateSwapChain", hr);
        throw SwapChainException("Falha ao criar SwapChain");
    }

    _swapChain = sc;
    
    try {
        auto swapChain = std::make_shared<SwapChainDX11>(_swapChain.Get());
        Drift::Core::LogRHI("SwapChain DX11 criada com sucesso");
        return swapChain;
    } catch (const std::exception& e) {
        Drift::Core::LogException("CreateSwapChain", e);
        throw;
    }
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
