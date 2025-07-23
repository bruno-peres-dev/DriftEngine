// src/rhi/include/Drift/RHI/Buffer.h
#pragma once
#include <memory>
#include <cstddef>
#include "Drift/RHI/Resource.h"

namespace Drift::RHI {

// Tipos de buffer suportados pela API de renderização
enum class BufferType { Vertex, Index, Constant };

// Descrição de um buffer genérico
struct BufferDesc {
    BufferType type;      // Tipo do buffer (vertex, index, constant)
    size_t sizeBytes;     // Tamanho em bytes
    const void* initData = nullptr; // Dados iniciais (opcional)
    bool operator==(const BufferDesc& o) const {
        return type == o.type && sizeBytes == o.sizeBytes && initData == o.initData;
    }
};

// Interface para buffers de GPU
class IBuffer : public IResource {
public:
    virtual ~IBuffer() = default;
    virtual void* Map() = 0;   // Mapeia o buffer para acesso CPU
    virtual void  Unmap() = 0; // Desfaz o mapeamento
};

} // namespace Drift::RHI

// Utilitário para atualizar constant buffer a partir de uma struct C++
namespace Drift::RHI {
    template<typename T>
    inline void UpdateConstantBuffer(IBuffer* buffer, const T& data) {
        void* ptr = buffer->Map();
        memcpy(ptr, &data, sizeof(T));
        buffer->Unmap();
    }
}

// Interface para ring buffer dinâmico (ex: uploads frequentes)
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

// Interface para batching de UI (ex: primitivas 2D)
namespace Drift::RHI {
    class IUIBatcher {
    public:
        virtual ~IUIBatcher() = default;
        virtual void Begin() = 0; // Inicia um novo batch
        virtual void AddRect(float x, float y, float w, float h, unsigned color) = 0; // Adiciona retângulo
        virtual void AddText(float x, float y, const char* text, unsigned color) = 0;  // Adiciona texto
        virtual void End() = 0;   // Finaliza e envia draw calls

        // Dimensões da tela (pode ser ignorado por implementações que usam matriz proj)
        virtual void SetScreenSize(float /*w*/, float /*h*/) {}
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
