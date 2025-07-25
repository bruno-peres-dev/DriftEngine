// src/rhi_dx11/src/TextureDX11.cpp

#include "Drift/RHI/DX11/TextureDX11.h"
#include "Drift/RHI/Texture.h"
#include "Drift/Core/Log.h"
#include <stdexcept>
#include <wrl/client.h>
#include <filesystem>
#include <vector>

#include <gli/gli.hpp>
#include <gli/dx.hpp>

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

    if (!desc.path.empty())
    {
        gli::texture tex;
        if (pathUtf8.size() >= 4 &&
            _stricmp(pathUtf8.c_str() + pathUtf8.size() - 4, ".dds") == 0)
        {
            tex = gli::load_dds(pathUtf8);
        }
        else if (pathUtf8.size() >= 4 &&
                 (_stricmp(pathUtf8.c_str() + pathUtf8.size() - 4, ".ktx") == 0 ||
                  _stricmp(pathUtf8.c_str() + pathUtf8.size() - 5, ".ktx2") == 0))
        {
            tex = gli::load_ktx(pathUtf8);
        }
        else
        {
            throw std::runtime_error("Formato de textura não suportado: " + pathUtf8);
        }

        if (tex.empty())
            throw std::runtime_error("Falha ao carregar textura: " + pathUtf8);

        gli::dx translator;
        auto dxFmt = translator.translate(tex.format());
        DXGI_FORMAT dxgiFmt = static_cast<DXGI_FORMAT>(dxFmt.DXGIFormat.DDS);

        D3D11_TEXTURE2D_DESC td{};
        td.Width = tex.extent().x;
        td.Height = tex.extent().y;
        td.MipLevels = static_cast<UINT>(tex.levels());
        td.ArraySize = 1;
        td.Format = dxgiFmt;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        td.CPUAccessFlags = 0;
        td.MiscFlags = tex.target() == gli::TARGET_CUBE ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

        std::vector<D3D11_SUBRESOURCE_DATA> initData(td.MipLevels);
        for (UINT level = 0; level < td.MipLevels; ++level) {
            auto extent = tex.extent(level);
            initData[level].pSysMem = tex.data(0, 0, level);
            size_t blockSize = gli::block_size(tex.format());
            auto blockExtent = gli::block_extent(tex.format());
            initData[level].SysMemPitch = (extent.x / blockExtent.x) * blockSize;
            initData[level].SysMemSlicePitch = (extent.y / blockExtent.y) * initData[level].SysMemPitch;
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texObj;
        hr = dev->CreateTexture2D(&td, initData.data(), texObj.GetAddressOf());
        if (FAILED(hr)) {
            throw std::runtime_error("Falha ao criar textura de '" + pathUtf8 + "'");
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = (td.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) ?
            D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = td.MipLevels;

        hr = dev->CreateShaderResourceView(texObj.Get(), &sd, srv.GetAddressOf());
        if (FAILED(hr)) {
            throw std::runtime_error("Falha ao criar SRV para '" + pathUtf8 + "'");
        }

        resource = texObj;
    }
    // Textura vazia em memória
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

// Retorna o uso de memória da textura
size_t TextureDX11::GetMemoryUsage() const {
    if (!_resource) return 0;
    
    // Para texturas 2D, calcula o tamanho baseado nas dimensões e formato
    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
    if (SUCCEEDED(_resource.As(&tex2D))) {
        D3D11_TEXTURE2D_DESC desc;
        tex2D->GetDesc(&desc);
        
        // Cálculo aproximado do tamanho da textura
        size_t bytesPerPixel = 4; // Assumindo RGBA8 como padrão
        switch (desc.Format) {
            case DXGI_FORMAT_R8_UNORM: bytesPerPixel = 1; break;
            case DXGI_FORMAT_R8G8_UNORM: bytesPerPixel = 2; break;
            case DXGI_FORMAT_R8G8B8A8_UNORM: bytesPerPixel = 4; break;
            case DXGI_FORMAT_BC1_UNORM: bytesPerPixel = 8; break; // Compressed
            case DXGI_FORMAT_BC3_UNORM: bytesPerPixel = 16; break; // Compressed
            default: bytesPerPixel = 4; break;
        }
        
        return desc.Width * desc.Height * desc.ArraySize * bytesPerPixel;
    }
    
    return 0;
}
