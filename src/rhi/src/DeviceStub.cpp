// src/rhi/src/DeviceStub.cpp

#include "Drift/RHI/Device.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Shader.h"
#include "Drift/RHI/Texture.h"    // <-- necess�rio para ITexture e ISampler

using namespace Drift::RHI;

class DeviceStub : public IDevice {
public:
    explicit DeviceStub(const DeviceDesc&) {}

    // Context / SwapChain n�o suportados no stub
    std::shared_ptr<IContext> CreateContext() override {
        return nullptr;
    }
    std::shared_ptr<ISwapChain> CreateSwapChain(void*) override {
        return nullptr;
    }

    // Buffers de stub
    std::shared_ptr<IBuffer> CreateBuffer(const BufferDesc&) override {
        return nullptr;
    }

    // PipelineState de stub
    std::shared_ptr<IPipelineState> CreatePipeline(const PipelineDesc&) override {
        return nullptr;
    }

    // Shader de stub
    std::shared_ptr<IShader> CreateShader(const ShaderDesc&) override {
        return nullptr;
    }

    // Textura de stub
    std::shared_ptr<ITexture> CreateTexture(const TextureDesc&) override {
        return nullptr;
    }

    // Sampler de stub
    std::shared_ptr<ISampler> CreateSampler(const SamplerDesc&) override {
        return nullptr;
    }

    void* GetNativeDevice() const override { return nullptr; }
};

std::shared_ptr<IDevice> CreateDeviceStub(const DeviceDesc& desc) {
    return std::make_shared<DeviceStub>(desc);
}
