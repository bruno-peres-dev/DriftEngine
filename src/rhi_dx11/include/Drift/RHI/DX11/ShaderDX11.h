#pragma once

#include "Drift/RHI/Shader.h"
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <memory>

namespace Drift::RHI::DX11 {

    /// Implementa��o DX11 de IShader, guarda o blob compilado
    class ShaderDX11 : public IShader {
    public:
        explicit ShaderDX11(ID3DBlob* blob);
        ~ShaderDX11() override;

        // IShader
        const void* GetBytecode()    const override { return _blob->GetBufferPointer(); }
        size_t      GetBytecodeSize() const override { return _blob->GetBufferSize(); }
        void SetReloadCallback(ReloadCallback cb) override { _reloadCb = cb; }
    private:
        Microsoft::WRL::ComPtr<ID3DBlob> _blob;
        ReloadCallback _reloadCb = nullptr;
    };

    /// Compila HLSL para um ShaderDX11 e devolve shared_ptr<IShader>
    std::shared_ptr<IShader> CreateShaderDX11(const ShaderDesc& desc, const D3D_SHADER_MACRO* macros = nullptr);

} // namespace Drift::RHI::DX11
