#include "Drift/RHI/DX11/ShaderDX11.h"
#include "Drift/Core/Log.h"
#include <stdexcept>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <filesystem>

using Microsoft::WRL::ComPtr;

namespace Drift::RHI::DX11 {

    // Função para resolver o caminho do shader
    std::wstring ResolveShaderPath(const std::string& filePath) {
        std::vector<std::wstring> searchPaths = {
            L"",  // Caminho atual
            L"shaders/",
            L"../shaders/",
            L"../../shaders/",
            L"../../../shaders/",
            L"../../../../shaders/"
        };

        for (const auto& searchPath : searchPaths) {
            std::wstring fullPath = searchPath + std::wstring(filePath.begin(), filePath.end());
            if (std::filesystem::exists(fullPath)) {
                std::string logPath(fullPath.begin(), fullPath.end());
                Drift::Core::Log("[ShaderDX11] Shader encontrado em: " + logPath);
                return fullPath;
            }
        }

        // Se não encontrou, retorna o caminho original
        Drift::Core::Log("[ShaderDX11] AVISO: Shader não encontrado, tentando caminho original: " + filePath);
        return std::wstring(filePath.begin(), filePath.end());
    }

    // Construtor: armazena blob compilado
    ShaderDX11::ShaderDX11(ID3DBlob* blob)
        : _blob(blob)
    {
    }

    ShaderDX11::~ShaderDX11() = default;

    // Compila shader HLSL a partir de arquivo
    std::shared_ptr<IShader> CreateShaderDX11(const ShaderDesc& desc) {
        ComPtr<ID3DBlob> compiled, errors;
        
        std::wstring resolvedPath = ResolveShaderPath(desc.filePath);
        
        HRESULT hr = D3DCompileFromFile(
            resolvedPath.c_str(),
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
        
        std::wstring resolvedPath = ResolveShaderPath(desc.filePath);
        
        HRESULT hr = D3DCompileFromFile(
            resolvedPath.c_str(),
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

    // Retorna o uso de memória do shader
    size_t ShaderDX11::GetMemoryUsage() const {
        if (!_blob) return 0;
        return _blob->GetBufferSize();
    }

} // namespace Drift::RHI::DX11
