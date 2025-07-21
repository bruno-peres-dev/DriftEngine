// src/rhi_dx11/src/TextureDX11.cpp

#include "Drift/RHI/DX11/TextureDX11.h"
#include "Drift/RHI/Texture.h"
#include "Drift/Core/Log.h"
#include <stdexcept>
#include <wrl/client.h>
#include <filesystem>

// DirectXTK headers
#include <WICTextureLoader.h>      // CreateWICTextureFromFile
#include <DDSTextureLoader.h>      // CreateDDSTextureFromFile

using namespace Drift::RHI::DX11;

// Converte wstring para UTF-8 usando std::filesystem
static std::string ToUTF8(const std::wstring& wstr) {
    return std::filesystem::path(wstr).u8string();
}

namespace Drift::RHI::DX11 {

// Cria textura DX11 a partir de arquivo (DDS/WIC) ou memória
std::shared_ptr<Drift::RHI::ITexture> CreateTextureDX11(
    ID3D11Device* dev,
    ID3D11DeviceContext* ctx,
    const Drift::RHI::TextureDesc& desc)
{
    Microsoft::WRL::ComPtr<ID3D11Resource>          resource;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    HRESULT hr;

    const std::string pathUtf8 = ToUTF8(desc.path);

    // Checagem de existência do arquivo
    if (!desc.path.empty() && !std::filesystem::exists(desc.path)) {
        Drift::Core::Log("[DX11][ERRO] Arquivo de textura não encontrado: " + pathUtf8);
        throw std::runtime_error("Arquivo de textura não encontrado: " + pathUtf8);
    }

    // 1) DDS: usa DDSTextureLoader
    if (pathUtf8.size() >= 4 &&
        _stricmp(pathUtf8.c_str() + pathUtf8.size() - 4, ".dds") == 0)
    {
        hr = DirectX::CreateDDSTextureFromFile(dev, ctx,
            desc.path.c_str(),
            resource.GetAddressOf(),
            srv.GetAddressOf());
        if (FAILED(hr)) {
            throw std::runtime_error("DDSTextureLoader falhou em '" +
                pathUtf8 + "'");
        }
    }
    // 2) Outros formatos: usa WICTextureLoader
    else if (!desc.path.empty())
    {
        hr = DirectX::CreateWICTextureFromFile(dev, ctx,
            desc.path.c_str(),
            resource.GetAddressOf(),
            srv.GetAddressOf());
        if (FAILED(hr)) {
            throw std::runtime_error("WICTextureLoader falhou em '" +
                pathUtf8 + "'");
        }
    }
    // 3) Textura vazia em memória
    else
    {
        D3D11_TEXTURE2D_DESC td{};
        td.Width = desc.width;
        td.Height = desc.height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        // Mapeia Format para DXGI_FORMAT
        auto toDXGIFormat = [](Drift::RHI::Format fmt) {
            switch (fmt) {
                case Drift::RHI::Format::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
                case Drift::RHI::Format::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
                case Drift::RHI::Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
                case Drift::RHI::Format::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
                case Drift::RHI::Format::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
                default: return DXGI_FORMAT_UNKNOWN;
            }
        };
        td.Format = toDXGIFormat(desc.format);
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        td.CPUAccessFlags = 0;
        td.MiscFlags = 0;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        hr = dev->CreateTexture2D(&td, nullptr, tex.GetAddressOf());
        if (FAILED(hr)) {
            Drift::Core::Log("[DX11] Falha ao criar Texture2D vazia: " + std::to_string(hr) + " (tentando fallback para R8_UNORM)");
            td.Format = DXGI_FORMAT_R8_UNORM;
            hr = dev->CreateTexture2D(&td, nullptr, tex.GetAddressOf());
            if (FAILED(hr)) {
                throw std::runtime_error("Falha ao criar Texture2D vazia (fallback R8_UNORM). HRESULT=" + std::to_string(hr));
            } else {
                Drift::Core::Log("[DX11] Fallback: textura criada como R8_UNORM");
            }
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = td.MipLevels;

        hr = dev->CreateShaderResourceView(tex.Get(), &sd, srv.GetAddressOf());
        if (FAILED(hr)) {
            throw std::runtime_error("Falha ao criar SRV da textura vazia");
        }
    }

    return std::make_shared<TextureDX11>(srv.Get(), resource.Get(), ctx);
}

} // namespace Drift::RHI::DX11

// Construtor: armazena ponteiros do recurso e SRV
TextureDX11::TextureDX11(ID3D11ShaderResourceView* srv, ID3D11Resource* resource, ID3D11DeviceContext* context)
    : _srv(srv), _resource(resource), _context(context)
{
}

// Atualiza subresource da textura (ex: upload de dados)
void TextureDX11::UpdateSubresource(unsigned mipLevel, unsigned arraySlice, const void* data, size_t rowPitch, size_t slicePitch) {
    if (!_context) throw std::runtime_error("TextureDX11: contexto não definido para UpdateSubresource");
    UINT subresource = D3D11CalcSubresource(mipLevel, arraySlice, 1);
    _context->UpdateSubresource(_resource.Get(), subresource, nullptr, data, (UINT)rowPitch, (UINT)slicePitch);
}
