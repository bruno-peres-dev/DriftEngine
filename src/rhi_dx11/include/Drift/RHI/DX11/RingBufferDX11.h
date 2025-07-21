#pragma once
#include <d3d11.h>
#include "Drift/RHI/Buffer.h"
#include <memory>
#include <vector>

namespace Drift::RHI::DX11 {

class RingBufferDX11 : public IRingBuffer {
public:
    RingBufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, size_t sizeBytes, int numBuffers = 3);
    void* Allocate(size_t size, size_t alignment, size_t& outOffset) override;
    IBuffer* GetBuffer() override;
    void Reset() override;
    void NextFrame(); // Alterna para o próximo buffer
protected:
    std::vector<std::shared_ptr<IBuffer>> _buffers;
    int _current;
    size_t _size;
    size_t _offset;
    int _numBuffers;
};

std::shared_ptr<IRingBuffer> CreateRingBufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, size_t sizeBytes, int numBuffers = 3);

} // namespace Drift::RHI::DX11 