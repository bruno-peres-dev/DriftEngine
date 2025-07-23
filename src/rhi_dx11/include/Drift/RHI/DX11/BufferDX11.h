#pragma once

#include "Drift/RHI/Buffer.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Drift::RHI::DX11 {

    // Implementação DX11 de IBuffer
    class BufferDX11 : public IBuffer {
    public:
        BufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, const BufferDesc& desc);
        explicit BufferDX11(Microsoft::WRL::ComPtr<ID3D11Buffer> buffer, ID3D11DeviceContext* context);
        ~BufferDX11() override = default;
        void* GetBackendHandle() const override { return _buffer.Get(); }
        size_t GetMemoryUsage() const override;
        void* Map() override;
        void  Unmap() override;
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
        void* _mappedPtr = nullptr;
    };

    // Cria um BufferDX11 (vertex, index ou constant) e retorna shared_ptr<IBuffer>
    std::shared_ptr<IBuffer> CreateBufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, const BufferDesc& desc);

} // namespace Drift::RHI::DX11
