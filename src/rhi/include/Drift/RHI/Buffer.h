// src/rhi/include/Drift/RHI/Buffer.h
#pragma once
#include <memory>
#include <cstddef>

namespace Drift::RHI {

// Tipos de buffer suportados
enum class BufferType { Vertex, Index, Constant };

struct BufferDesc {
    BufferType type;
    size_t sizeBytes;
    const void* initData = nullptr;
    bool operator==(const BufferDesc& o) const {
        return type == o.type && sizeBytes == o.sizeBytes && initData == o.initData;
    }
};

class IBuffer {
public:
    virtual ~IBuffer() = default;
    using BackendHandle = void*;
    virtual BackendHandle GetBackendHandle() = 0;
    virtual void* Map() = 0; // Mapeia o buffer para escrita
    virtual void  Unmap() = 0; // Desmapeia o buffer
};

} // namespace Drift::RHI

// Utilitário para atualizar constant buffer a partir de struct C++
namespace Drift::RHI {
    template<typename T>
    inline void UpdateConstantBuffer(IBuffer* buffer, const T& data) {
        void* ptr = buffer->Map();
        memcpy(ptr, &data, sizeof(T));
        buffer->Unmap();
    }
}

// RingBuffer para uploads dinâmicos
namespace Drift::RHI {
    class IRingBuffer {
    public:
        virtual ~IRingBuffer() = default;
        virtual void* Allocate(size_t size, size_t alignment, size_t& outOffset) = 0;
        virtual IBuffer* GetBuffer() = 0;
        virtual void Reset() = 0;
        virtual void NextFrame() = 0;
    };
}

// Batching para UI
namespace Drift::RHI {
    class IUIBatcher {
    public:
        virtual ~IUIBatcher() = default;
        // Inicia um novo batch
        virtual void Begin() = 0;
        // Adiciona um retângulo ao batch
        virtual void AddRect(float x, float y, float w, float h, unsigned color) = 0;
        // Adiciona um texto ao batch (API simplificada)
        virtual void AddText(float x, float y, const char* text, unsigned color) = 0;
        // Envia os dados para o ring buffer e emite draw calls
        virtual void End() = 0;
    };
}

namespace std {
template<>
struct hash<Drift::RHI::BufferDesc> {
    size_t operator()(const Drift::RHI::BufferDesc& b) const noexcept {
        size_t h1 = std::hash<int>{}(static_cast<int>(b.type));
        size_t h2 = std::hash<size_t>{}(b.sizeBytes);
        size_t h3 = std::hash<const void*>{}(b.initData);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
}
