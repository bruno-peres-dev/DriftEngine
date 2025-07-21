// src/rhi_dx11/src/BufferDX11.cpp
#include "Drift/RHI/DX11/BufferDX11.h"
#include "Drift/Core/Log.h"
#include <stdexcept>
#include <wrl/client.h>   // para Microsoft::WRL::ComPtr
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

namespace Drift::RHI::DX11 {

    BufferDX11::BufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, const BufferDesc& desc)
        : _context(context)
    {
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = static_cast<UINT>(desc.sizeBytes);
        bd.BindFlags = (desc.type == BufferType::Vertex)
            ? D3D11_BIND_VERTEX_BUFFER
            : D3D11_BIND_INDEX_BUFFER;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = desc.initData;

        if (FAILED(device->CreateBuffer(
            &bd,
            desc.initData ? &initData : nullptr,
            _buffer.GetAddressOf())))
        {
            Drift::Core::Log(std::string("[DX11] Erro ao criar buffer: size=") + std::to_string(desc.sizeBytes));
            // Fallback: tentar criar como DEFAULT (read-only)
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.CPUAccessFlags = 0;
            if (FAILED(device->CreateBuffer(&bd, desc.initData ? &initData : nullptr, _buffer.GetAddressOf()))) {
                throw std::runtime_error("Failed to create D3D11 buffer (dynamic e default). Size: " + std::to_string(desc.sizeBytes));
            } else {
                Drift::Core::Log("[DX11] Fallback: buffer criado como D3D11_USAGE_DEFAULT");
            }
        }
    }

    BufferDX11::BufferDX11(ComPtr<ID3D11Buffer> buffer, ID3D11DeviceContext* context)
        : _buffer(std::move(buffer)), _context(context)
    {
    }

    std::shared_ptr<IBuffer> CreateBufferDX11(ID3D11Device* device, ID3D11DeviceContext* context, const BufferDesc& desc) {
        if (desc.type == BufferType::Constant) {
            // cria constant buffer dinâmico
            D3D11_BUFFER_DESC bd{};
            bd.ByteWidth = static_cast<UINT>(desc.sizeBytes);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bd.MiscFlags = 0;

            ComPtr<ID3D11Buffer> buf;
            if (FAILED(device->CreateBuffer(&bd, nullptr, buf.GetAddressOf())))
                throw std::runtime_error("Failed to create ConstantBuffer");
            return std::make_shared<BufferDX11>(buf, context);
        }
        else {
            // vertex ou index
            return std::make_shared<BufferDX11>(device, context, desc);
        }
    }

    void* BufferDX11::Map() {
        if (!_context) throw std::runtime_error("BufferDX11: contexto não definido para Map");
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(_context->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            throw std::runtime_error("BufferDX11: falha ao mapear buffer");
        _mappedPtr = mapped.pData;
        return _mappedPtr;
    }

    void BufferDX11::Unmap() {
        if (!_context) throw std::runtime_error("BufferDX11: contexto não definido para Unmap");
        _context->Unmap(_buffer.Get(), 0);
        _mappedPtr = nullptr;
    }

} // namespace Drift::RHI::DX11
