#pragma once

#include "Drift/RHI/Shader.h"
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <memory>

namespace Drift::RHI::DX11 {

    // Implementação DX11 de IShader, armazena o blob compilado
    class ShaderDX11 : public IShader {
    public:
        explicit ShaderDX11(ID3DBlob* blob);
        ~ShaderDX11() override;

        // Métodos de acesso ao bytecode
        const void* GetBytecode()    const override { return _blob->GetBufferPointer(); }
        size_t      GetBytecodeSize() const override { return _blob->GetBufferSize(); }
        void SetReloadCallback(ReloadCallback cb) override { _reloadCb = cb; }
        
        // Métodos da interface IResource
        void* GetBackendHandle() const override { return _blob.Get(); }
        size_t GetMemoryUsage() const override;
    private:
        Microsoft::WRL::ComPtr<ID3DBlob> _blob;
        ReloadCallback _reloadCb = nullptr;
    };

    // Compila HLSL para um ShaderDX11 e retorna shared_ptr<IShader>
    std::shared_ptr<IShader> CreateShaderDX11(const ShaderDesc& desc, const D3D_SHADER_MACRO* macros = nullptr);

} // namespace Drift::RHI::DX11
