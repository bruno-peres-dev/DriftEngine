#include "Drift/RHI/DX11/ShaderDX11.h"
#include <stdexcept>
#include <string>
#include <wrl/client.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

namespace Drift::RHI::DX11 {

    // Construtor: armazena blob compilado
    ShaderDX11::ShaderDX11(ID3DBlob* blob)
        : _blob(blob)
    {
    }

    ShaderDX11::~ShaderDX11() = default;

    // Compila shader HLSL a partir de arquivo
    std::shared_ptr<IShader> CreateShaderDX11(const ShaderDesc& desc) {
        ComPtr<ID3DBlob> compiled, errors;
        HRESULT hr = D3DCompileFromFile(
            std::wstring(desc.filePath.begin(), desc.filePath.end()).c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            desc.entryPoint.c_str(),
            desc.target.c_str(),
            0, 0,
            compiled.GetAddressOf(),
            errors.GetAddressOf()
        );
        if (FAILED(hr)) {
            std::string msg = "Shader compile error (" + desc.filePath + "): ";
            if (errors)
                msg += reinterpret_cast<const char*>(errors->GetBufferPointer());
            else {
                char buf[64];
                sprintf_s(buf, "HRESULT=0x%08X", static_cast<unsigned>(hr));
                msg += buf;
            }
            throw std::runtime_error(msg);
        }

        return std::make_shared<ShaderDX11>(compiled.Get());
    }

    // Compila shader HLSL com macros de pré-processador
    std::shared_ptr<IShader> CreateShaderDX11(const ShaderDesc& desc, const D3D_SHADER_MACRO* macros) {
        ComPtr<ID3DBlob> compiled, errors;
        HRESULT hr = D3DCompileFromFile(
            std::wstring(desc.filePath.begin(), desc.filePath.end()).c_str(),
            macros,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            desc.entryPoint.c_str(),
            desc.target.c_str(),
            0, 0,
            compiled.GetAddressOf(),
            errors.GetAddressOf()
        );
        if (FAILED(hr)) {
            std::string msg = "Shader compile error (" + desc.filePath + "): ";
            if (errors)
                msg += reinterpret_cast<const char*>(errors->GetBufferPointer());
            else {
                char buf[64];
                sprintf_s(buf, "HRESULT=0x%08X", static_cast<unsigned>(hr));
                msg += buf;
            }
            throw std::runtime_error(msg);
        }
        return std::make_shared<ShaderDX11>(compiled.Get());
    }

} // namespace Drift::RHI::DX11
