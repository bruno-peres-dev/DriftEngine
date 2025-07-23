// src/rhi/include/Drift/RHI/Shader.h
#pragma once
#include <string>
#include <memory>
#include "Drift/RHI/Resource.h"

namespace Drift::RHI {

    // Descrição de um shader HLSL
    struct ShaderDesc {
        std::string filePath;      // Caminho do arquivo HLSL
        std::string entryPoint;    // Entry point (ex: "VSMain", "PSMain")
        std::string target;        // Target profile (ex: "vs_5_0", "ps_5_0")
        bool operator==(ShaderDesc const& o) const {
            return filePath == o.filePath
                && entryPoint == o.entryPoint
                && target == o.target;
        }
    };

    // Interface para shader compilado
    class IShader : public IResource {
    public:
        virtual ~IShader() = default;
        virtual const void* GetBytecode() const = 0;
        virtual size_t      GetBytecodeSize() const = 0;
        // Callback para hot-reload do shader
        using ReloadCallback = void(*)(IShader*);
        virtual void SetReloadCallback(ReloadCallback cb) = 0;
    };

} // namespace Drift::RHI

// Especialização de hash para ShaderDesc
#include <functional>
namespace std {
    template<>
    struct hash<Drift::RHI::ShaderDesc> {
        size_t operator()(Drift::RHI::ShaderDesc const& s) const noexcept {
            auto h1 = hash<string>{}(s.filePath);
            auto h2 = hash<string>{}(s.entryPoint);
            auto h3 = hash<string>{}(s.target);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}
