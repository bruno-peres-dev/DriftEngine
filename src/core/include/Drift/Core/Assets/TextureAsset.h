#pragma once

#include "Drift/Core/AssetsManager.h"
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Device.h"

namespace Drift::Core::Assets {

/**
 * @brief Parâmetros para carregamento de texturas
 */
struct TextureLoadParams {
    RHI::Format format = RHI::Format::Unknown;  // Formato desejado (Unknown = auto)
    bool generateMips = true;                   // Gerar mipmaps
    bool sRGB = false;                         // Usar espaço de cor sRGB
    RHI::SamplerDesc samplerDesc;              // Configuração do sampler
};

/**
 * @brief Asset wrapper para texturas
 */
class TextureAsset : public IAsset {
public:
    TextureAsset(const std::string& path, std::shared_ptr<RHI::ITexture> texture = nullptr);
    virtual ~TextureAsset() = default;

    // IAsset implementation
    size_t GetMemoryUsage() const override;
    const std::string& GetPath() const override { return m_Path; }
    bool IsLoaded() const override { return m_Texture != nullptr; }
    bool Load() override;
    void Unload() override;

    // Texture-specific methods
    std::shared_ptr<RHI::ITexture> GetTexture() const { return m_Texture; }
    const RHI::TextureDesc& GetDesc() const { return m_Desc; }
    
    void SetTexture(std::shared_ptr<RHI::ITexture> texture) { m_Texture = texture; }
    void SetDesc(const RHI::TextureDesc& desc) { m_Desc = desc; }

private:
    std::string m_Path;
    std::shared_ptr<RHI::ITexture> m_Texture;
    RHI::TextureDesc m_Desc;
    size_t m_EstimatedMemoryUsage = 0;
};

/**
 * @brief Loader para texturas
 */
class TextureLoader : public IAssetLoader<TextureAsset> {
public:
    TextureLoader(RHI::IDevice* device) : m_Device(device) {}

    std::shared_ptr<TextureAsset> Load(const std::string& path, const std::any& params = {}) override;
    bool CanLoad(const std::string& path) const override;
    std::vector<std::string> GetSupportedExtensions() const override;

private:
    RHI::IDevice* m_Device;
    
    // Métodos auxiliares
    TextureLoadParams ExtractParams(const std::any& params) const;
    size_t EstimateTextureMemoryUsage(unsigned width, unsigned height, RHI::Format format) const;
    RHI::Format DetermineFormat(const std::string& path, RHI::Format requestedFormat) const;
};

} // namespace Drift::Core::Assets