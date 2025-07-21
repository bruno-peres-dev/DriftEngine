#include <d3d11.h>
#include "Drift/RHI/DX11/RingBufferDX11.h"
#include "Drift/RHI/DX11/BufferDX11.h"
#include "Drift/RHI/Buffer.h"
#include <cstring>
#include <vector>

using namespace Drift::RHI::DX11;

class RingBufferDX11Impl : public RingBufferDX11 {
public:
    RingBufferDX11Impl(ID3D11Device* device, ID3D11DeviceContext* context, size_t sizeBytes, int numBuffers)
        : RingBufferDX11(device, context, sizeBytes, numBuffers), _context(context) {
        _mappedPtr.resize(_numBuffers, nullptr);
    }
    void* _basePtr = nullptr;
    std::vector<void*> _mappedPtr;
    ID3D11DeviceContext* _context;
};

RingBufferDX11::RingBufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, size_t sizeBytes, int numBuffers)
    : _size(sizeBytes), _offset(0), _numBuffers(numBuffers), _current(0) {
    _buffers.resize(_numBuffers);
    for (int i = 0; i < _numBuffers; ++i) {
        Drift::RHI::BufferDesc desc;
        desc.type = Drift::RHI::BufferType::Vertex;
        desc.sizeBytes = sizeBytes;
        desc.initData = nullptr;
        _buffers[i] = CreateBufferDX11(device, context, desc);
    }
    // Mapeia o primeiro buffer
    NextFrame();
}

void* RingBufferDX11::Allocate(size_t size, size_t alignment, size_t& outOffset) {
    size_t aligned = (_offset + alignment - 1) & ~(alignment - 1);
    if (aligned + size > _size) {
        // Buffer cheio, reiniciar (na prática, use NextFrame)
        _offset = 0;
        aligned = 0;
    }
    outOffset = aligned;
    // Não faz Map/Unmap aqui, só retorna ponteiro
    void* basePtr = static_cast<RingBufferDX11Impl*>(this)->_basePtr;
    void* result = static_cast<char*>(basePtr) + aligned;
    _offset = aligned + size;
    return result;
}

Drift::RHI::IBuffer* RingBufferDX11::GetBuffer() {
    return _buffers[_current].get();
}

void RingBufferDX11::Reset() {
    _offset = 0;
}

void RingBufferDX11::NextFrame() {
    RingBufferDX11Impl* impl = static_cast<RingBufferDX11Impl*>(this);
    // Unmap do buffer anterior, se necessário
    if (impl->_basePtr) {
        _buffers[_current]->Unmap();
        impl->_basePtr = nullptr;
    }
    _current = (_current + 1) % _numBuffers;
    _offset = 0;
    // Map do novo buffer
    impl->_basePtr = _buffers[_current]->Map();
}

std::shared_ptr<Drift::RHI::IRingBuffer> Drift::RHI::DX11::CreateRingBufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, size_t sizeBytes, int numBuffers) {
    return std::make_shared<RingBufferDX11Impl>(device, context, sizeBytes, numBuffers);
} 