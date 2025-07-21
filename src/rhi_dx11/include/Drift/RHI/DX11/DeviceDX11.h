#pragma once

#include <memory>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

#include "Drift/RHI/Device.h"         // IDevice, DeviceDesc
#include "Drift/RHI/ResourceCache.h"  // ResourceCache<>
#include "Drift/RHI/DX11/RingBufferDX11.h"
#include "Drift/RHI/DX11/UIBatcherDX11.h"

namespace Drift::RHI {
    struct DeviceDesc;
    class IContext;
    class ISwapChain;
    struct BufferDesc;
    class IBuffer;
    struct PipelineDesc;
    class IPipelineState;
    struct ShaderDesc;
    class IShader;
    struct TextureDesc;
    class ITexture;
    struct SamplerDesc;
    class ISampler;
}

namespace Drift::RHI::DX11 {

    // Implementação DX11 de IDevice
    class DeviceDX11 : public Drift::RHI::IDevice {
    public:
        explicit DeviceDX11(const Drift::RHI::DeviceDesc& desc);
        ~DeviceDX11() override;

        // Métodos de criação de recursos (IDevice)
        std::shared_ptr<Drift::RHI::IContext>       CreateContext() override;
        std::shared_ptr<Drift::RHI::ISwapChain>     CreateSwapChain(void* hwnd) override;
        std::shared_ptr<Drift::RHI::IBuffer>        CreateBuffer(const Drift::RHI::BufferDesc& d) override;
        std::shared_ptr<Drift::RHI::IPipelineState> CreatePipeline(const Drift::RHI::PipelineDesc& d) override;
        std::shared_ptr<Drift::RHI::IShader>        CreateShader(const Drift::RHI::ShaderDesc& d) override;
        std::shared_ptr<Drift::RHI::ITexture>       CreateTexture(const Drift::RHI::TextureDesc& d) override;
        std::shared_ptr<Drift::RHI::ISampler>       CreateSampler(const Drift::RHI::SamplerDesc& d) override;
        void* GetNativeDevice() const override { return _device.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Device>        _device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
        Microsoft::WRL::ComPtr<IDXGISwapChain>      _swapChain;
        Drift::RHI::DeviceDesc                      _desc;

        ResourceCache<Drift::RHI::ShaderDesc, Drift::RHI::IShader>        _shaderCache;
        ResourceCache<Drift::RHI::BufferDesc, Drift::RHI::IBuffer>        _bufferCache;
        ResourceCache<Drift::RHI::PipelineDesc, Drift::RHI::IPipelineState> _pipelineCache;
        ResourceCache<Drift::RHI::TextureDesc, Drift::RHI::ITexture>       _textureCache;
        ResourceCache<Drift::RHI::SamplerDesc, Drift::RHI::ISampler>       _samplerCache;
    };

    // Fábrica inline para criar DeviceDX11
    inline std::shared_ptr<Drift::RHI::IDevice> CreateDeviceDX11(const Drift::RHI::DeviceDesc& desc) {
        return std::make_shared<DeviceDX11>(desc);
    }

} // namespace Drift::RHI::DX11
