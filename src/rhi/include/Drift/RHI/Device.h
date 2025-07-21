#pragma once

#include <memory>
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Shader.h"
#include "Drift/RHI/Texture.h"

namespace Drift::RHI {

    struct DeviceDesc {
        unsigned width, height;
        bool vsync;
    };

    class IDevice {
    public:
        virtual ~IDevice() = default;
        virtual std::shared_ptr<IContext> CreateContext() = 0;
        virtual std::shared_ptr<ISwapChain> CreateSwapChain(void* windowHandle) = 0;
        virtual std::shared_ptr<IBuffer> CreateBuffer(const BufferDesc& desc) = 0;
        virtual std::shared_ptr<IPipelineState> CreatePipeline(const PipelineDesc& desc) = 0;
        virtual std::shared_ptr<IShader> CreateShader(const ShaderDesc& desc) = 0;
        virtual std::shared_ptr<ITexture> CreateTexture(const TextureDesc& desc) = 0;
        virtual std::shared_ptr<ISampler> CreateSampler(const SamplerDesc& desc) = 0;
        virtual void* GetNativeDevice() const = 0;
    };

    /// stub (no usado em DX11)
    std::shared_ptr<IDevice> CreateDeviceStub(const DeviceDesc& desc);

} // namespace Drift::RHI
