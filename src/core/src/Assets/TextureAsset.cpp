#include "Drift/Core/Assets/TextureAsset.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/Format.h"
#include <filesystem>
#include <algorithm>

namespace Drift::Core::Assets {

// =============================================================================
// TextureAsset Implementation
// =============================================================================

TextureAsset::TextureAsset(const std::string& path, std::shared_ptr<RHI::ITexture> texture)
    : m_Path(path), m_Texture(texture) {
    if (m_Texture) {
        m_EstimatedMemoryUsage = m_Texture->GetMemoryUsage();
    }
}

size_t TextureAsset::GetMemoryUsage() const {
    if (m_Texture) {
        return m_Texture->GetMemoryUsage();
    }
    return m_EstimatedMemoryUsage;
}

bool TextureAsset::Load() {
    if (IsLoaded()) {
        return true;
    }
    
    // Em uma implementação real, recarregaria a textura do disco
    // Por enquanto, só logamos
    Log("[TextureAsset] Recarregando textura: " + m_Path);
    return false;
}

void TextureAsset::Unload() {
    if (m_Texture) {
        Log("[TextureAsset] Descarregando textura: " + m_Path);
        m_Texture.reset();
    }
}

// =============================================================================
// TextureLoader Implementation
// =============================================================================

std::shared_ptr<TextureAsset> TextureLoader::Load(const std::string& path, const std::any& params) {
    if (!m_Device) {
        Log("[TextureLoader] ERRO: Device não configurado");
        return nullptr;
    }
    
    if (!CanLoad(path)) {
        Log("[TextureLoader] ERRO: Não é possível carregar o arquivo: " + path);
        return nullptr;
    }
    
    TextureLoadParams loadParams = ExtractParams(params);
    
    // Converte path para wstring para compatibilidade com TextureDesc
    std::wstring wpath(path.begin(), path.end());
    
    // Cria a descrição da textura
    RHI::TextureDesc desc;
    desc.path = wpath;
    desc.format = DetermineFormat(path, loadParams.format);
    
    // Tenta carregar a textura através do device
    auto texture = m_Device->CreateTexture(desc);
    if (!texture) {
        Log("[TextureLoader] ERRO: Falha ao criar textura para: " + path);
        return nullptr;
    }
    
    // Cria o asset wrapper
    auto asset = std::make_shared<TextureAsset>(path, texture);
    asset->SetDesc(desc);
    
    Log("[TextureLoader] Textura carregada com sucesso: " + path);
    return asset;
}

bool TextureLoader::CanLoad(const std::string& path) const {
    std::filesystem::path fsPath(path);
    std::string extension = fsPath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    auto supportedExts = GetSupportedExtensions();
    return std::find(supportedExts.begin(), supportedExts.end(), extension) != supportedExts.end();
}

std::vector<std::string> TextureLoader::GetSupportedExtensions() const {
    return {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".hdr", ".exr",
        ".tiff", ".tif", ".webp", ".ktx", ".ktx2"
    };
}

TextureLoadParams TextureLoader::ExtractParams(const std::any& params) const {
    TextureLoadParams defaultParams;
    
    if (params.has_value()) {
        try {
            return std::any_cast<TextureLoadParams>(params);
        } catch (const std::bad_any_cast& e) {
            Log("[TextureLoader] AVISO: Parâmetros inválidos, usando padrões");
        }
    }
    
    return defaultParams;
}

size_t TextureLoader::EstimateTextureMemoryUsage(unsigned width, unsigned height, RHI::Format format) const {
    size_t bytesPerPixel = 4; // Default para RGBA8
    
    switch (format) {
        case RHI::Format::R8_UNORM:
            bytesPerPixel = 1;
            break;
        case RHI::Format::R8G8_UNORM:
            bytesPerPixel = 2;
            break;
        case RHI::Format::R8G8B8A8_UNORM:
        case RHI::Format::R8G8B8A8_SNORM:
            bytesPerPixel = 4;
            break;
        case RHI::Format::R16G16B16A16_UNORM:
            bytesPerPixel = 8;
            break;
        case RHI::Format::R32G32B32A32_FLOAT:
            bytesPerPixel = 16;
            break;
        default:
            bytesPerPixel = 4; // Assume RGBA8 como padrão
            break;
    }
    
    size_t baseMemory = width * height * bytesPerPixel;
    
    // Adiciona estimativa para mipmaps (aproximadamente 33% a mais)
    return static_cast<size_t>(baseMemory * 1.33f);
}

RHI::Format TextureLoader::DetermineFormat(const std::string& path, RHI::Format requestedFormat) const {
    if (requestedFormat != RHI::Format::Unknown) {
        return requestedFormat;
    }
    
    // Determina formato baseado na extensão
    std::filesystem::path fsPath(path);
    std::string extension = fsPath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".hdr" || extension == ".exr") {
        return RHI::Format::R16G16B16A16_UNORM;
    } else if (extension == ".dds") {
        // Para DDS, seria necessário analisar o header do arquivo
        return RHI::Format::R8G8B8A8_UNORM;
    } else {
        // Para formatos comuns (PNG, JPG, etc.)
        return RHI::Format::R8G8B8A8_UNORM;
    }
}

} // namespace Drift::Core::Assets