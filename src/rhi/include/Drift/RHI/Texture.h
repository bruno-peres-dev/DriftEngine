// src/rhi/include/Drift/RHI/Texture.h
#pragma once

#include <memory>
#include <string>
#include "Drift/RHI/Types.h"

namespace Drift::RHI {

// Descrição de uma textura
struct TextureDesc {
    std::wstring path;         // Caminho do arquivo (opcional)
    unsigned width = 0, height = 0; // Dimensões
    Format format = Format::Unknown; // Formato de pixel
};

inline bool operator==(const TextureDesc& a, const TextureDesc& b) {
    return a.path == b.path && a.width == b.width && a.height == b.height && a.format == b.format;
}

// Interface para textura de GPU
class ITexture {
public:
    virtual ~ITexture() = default;
    using BackendHandle = void*;
    virtual BackendHandle GetBackendHandle() const = 0;
    virtual void UpdateSubresource(unsigned mipLevel, unsigned arraySlice, const void* data, size_t rowPitch, size_t slicePitch) = 0;
};

// Interface para sampler state
class ISampler {
public:
    virtual ~ISampler() = default;
    using BackendHandle = void*;
    virtual BackendHandle GetBackendHandle() const = 0;
};

// Descrição de um sampler state
struct SamplerDesc {
    enum class Filter { Point, Linear, Anisotropic };
    enum class AddressMode { Wrap, Mirror, Clamp, Border };
    Filter      filter = Filter::Linear;
    AddressMode addressU = AddressMode::Wrap;
    AddressMode addressV = AddressMode::Wrap;
    AddressMode addressW = AddressMode::Wrap;
    float       mipLODBias = 0.0f;
    unsigned    maxAnisotropy = 1;
    float       minLOD = 0.0f;
    float       maxLOD = 1000.0f;
    bool operator==(const SamplerDesc& o) const {
        return filter == o.filter &&
               addressU == o.addressU &&
               addressV == o.addressV &&
               addressW == o.addressW &&
               mipLODBias == o.mipLODBias &&
               maxAnisotropy == o.maxAnisotropy &&
               minLOD == o.minLOD &&
               maxLOD == o.maxLOD;
    }
};

} // namespace Drift::RHI

// --- Especializações de hash para uso em unordered_map ---

#include <functional>
namespace std {
    template<>
    struct hash<Drift::RHI::TextureDesc> {
        size_t operator()(const Drift::RHI::TextureDesc& t) const noexcept {
            size_t h1 = std::hash<std::wstring>{}(t.path);
            size_t h2 = std::hash<unsigned>{}(t.width);
            size_t h3 = std::hash<unsigned>{}(t.height);
            size_t h4 = std::hash<int>{}(static_cast<int>(t.format));
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };

    template<>
    struct hash<Drift::RHI::SamplerDesc> {
        size_t operator()(const Drift::RHI::SamplerDesc& s) const {
            size_t h = 0;
            h ^= std::hash<int>()(int(s.filter));
            h ^= std::hash<int>()(int(s.addressU)) << 1;
            h ^= std::hash<int>()(int(s.addressV)) << 2;
            h ^= std::hash<int>()(int(s.addressW)) << 3;
            h ^= std::hash<float>()(s.mipLODBias);
            h ^= std::hash<unsigned>()(s.maxAnisotropy);
            h ^= std::hash<float>()(s.minLOD);
            h ^= std::hash<float>()(s.maxLOD);
            return h;
        }
    };
}
