#pragma once

#include "Drift/RHI/Texture.h"   // ITexture, TextureDesc
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Drift::RHI::DX11 {

    // Implementação DX11 de ITexture
    class TextureDX11 : public ITexture {
    public:
        TextureDX11(ID3D11ShaderResourceView* srv, ID3D11Resource* resource, ID3D11DeviceContext* context);
        void* GetBackendHandle() const override { return _srv.Get(); }
        size_t GetMemoryUsage() const override;
        void UpdateSubresource(unsigned mipLevel, unsigned arraySlice, const void* data, size_t rowPitch, size_t slicePitch) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
        Microsoft::WRL::ComPtr<ID3D11Resource> _resource;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
    };

    // Cria uma TextureDX11
    std::shared_ptr<ITexture> CreateTextureDX11(
        ID3D11Device* dev,
        ID3D11DeviceContext* ctx,
        const TextureDesc& desc);

} // namespace Drift::RHI::DX11
