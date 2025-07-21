// src/rhi/include/Drift/RHI/Shader.h
#pragma once
#include <string>
#include <memory>

namespace Drift::RHI {

    struct ShaderDesc {
        std::string filePath;      // Caminho do HLSL
        std::string entryPoint;    // e.g. "VSMain" ou "PSMain"
        std::string target;        // e.g. "vs_5_0" ou "ps_5_0"
        bool operator==(ShaderDesc const& o) const {
            return filePath == o.filePath
                && entryPoint == o.entryPoint
                && target == o.target;
        }
    };

    class IShader {
    public:
        virtual ~IShader() = default;
        virtual const void* GetBytecode() const = 0;
        virtual size_t      GetBytecodeSize() const = 0;
        // Callback para hot-reload
        using ReloadCallback = void(*)(IShader*);
        virtual void SetReloadCallback(ReloadCallback cb) = 0;
    };

    // Removido: std::shared_ptr<IShader> CreateShader(const ShaderDesc&);

} // namespace Drift::RHI

// specialize hash for ShaderDesc
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
