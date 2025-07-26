#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Device.h"

namespace Drift::UI {

struct AtlasRegion {
    int x{0};
    int y{0};
    int width{0};
    int height{0};
    uint32_t glyphId{0};
};

struct AtlasConfig {
    int width = 2048;
    int height = 2048;
    int padding = 2;
    int channels = 4;
    bool useMSDF = true;
    int msdfSize = 32;
};

class FontAtlas {
public:
    FontAtlas(const AtlasConfig& config = AtlasConfig{}, Drift::RHI::IDevice* device = nullptr);
    ~FontAtlas();

    AtlasRegion* AllocateRegion(int width, int height, uint32_t glyphId);
    bool UploadMSDFData(const AtlasRegion* region, const uint8_t* data, int width, int height);
    AtlasRegion* GetRegion(uint32_t glyphId) const;
    bool HasRegion(uint32_t glyphId) const;
    void Clear();

    size_t GetRegionCount() const;
    float GetUsagePercentage() const;

    Drift::RHI::ITexture* GetTexture() const;
    int GetWidth() const;
    int GetHeight() const;
    const AtlasConfig& GetConfig() const;
    
    // Criar textura se ainda não foi criada
    bool CreateTexture(Drift::RHI::IDevice* device);

private:
    AtlasConfig m_Config;
    std::unique_ptr<Drift::RHI::ITexture> m_Texture;
    std::shared_ptr<Drift::RHI::ITexture> m_SharedTexture; // Manter referência compartilhada
    std::vector<uint8_t> m_TextureData;
    int m_Width{0};
    int m_Height{0};
    int m_CurrentX{0};
    int m_CurrentY{0};
    int m_LineHeight{0};

    std::unordered_map<uint32_t, std::unique_ptr<AtlasRegion>> m_Regions;
};

class MultiAtlasManager {
public:
    explicit MultiAtlasManager(const AtlasConfig& defaultConfig = AtlasConfig{});
    ~MultiAtlasManager();

    FontAtlas* CreateAtlas(const AtlasConfig& config);
    FontAtlas* GetAtlasForGlyph(uint32_t glyphId);
    void Clear();

    size_t GetAtlasCount() const;
    const std::vector<std::unique_ptr<FontAtlas>>& GetAtlases() const;

private:
    AtlasConfig m_DefaultConfig;
    std::vector<std::unique_ptr<FontAtlas>> m_Atlases;
};

} // namespace Drift::UI