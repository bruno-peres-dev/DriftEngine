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
    int batchSize = 16; // Número de glifos para acumular antes do upload
};

// Estrutura para representar um upload pendente
struct PendingUpload {
    AtlasRegion* region;
    std::vector<uint8_t> data;
    int width;
    int height;
    
    PendingUpload(AtlasRegion* r, const uint8_t* d, int w, int h) 
        : region(r), width(w), height(h) {
        size_t dataSize = static_cast<size_t>(w * h * 4); // RGBA8
        data.assign(d, d + dataSize);
    }
};

class FontAtlas {
public:
    FontAtlas(const AtlasConfig& config = AtlasConfig{}, Drift::RHI::IDevice* device = nullptr);
    ~FontAtlas();

    AtlasRegion* AllocateRegion(int width, int height, uint32_t glyphId);

    // Faz upload dos dados MSDF para o atlas. Retorna false se os dados excederem
    // os limites do atlas ou se os parâmetros forem inválidos.
    bool UploadMSDFData(const AtlasRegion* region, const uint8_t* data, int width, int height);
    
    // Novos métodos para batching
    bool QueueMSDFUpload(const AtlasRegion* region, const uint8_t* data, int width, int height);
    bool FlushPendingUploads();
    bool HasPendingUploads() const { return !m_PendingUploads.empty(); }
    
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
    
    // Verificar se o device está pronto para uploads
    bool IsDeviceReady() const;
    
    // Definir device para uploads futuros
    void SetDevice(Drift::RHI::IDevice* device);

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
    
    // Sistema de batching
    std::vector<PendingUpload> m_PendingUploads;
    bool m_NeedsFullUpload{false};
    
    // Estado do device
    Drift::RHI::IDevice* m_Device{nullptr};
    bool m_DeviceReady{false};
    
    // Método interno para fazer upload de uma região específica
    bool UploadRegionData(const AtlasRegion* region, const uint8_t* data, int width, int height);
    
    // Método interno para verificar se o device está válido
    bool ValidateDevice() const;
};

class MultiAtlasManager {
public:
    explicit MultiAtlasManager(const AtlasConfig& defaultConfig = AtlasConfig{});
    ~MultiAtlasManager();

    FontAtlas* CreateAtlas(const AtlasConfig& config);
    FontAtlas* GetAtlasForGlyph(uint32_t glyphId);
    void Clear();
    
    // Método para flush de todos os atlases
    void FlushAllAtlases();
    
    // Definir device para todos os atlases
    void SetDevice(Drift::RHI::IDevice* device);

    size_t GetAtlasCount() const;
    const std::vector<std::unique_ptr<FontAtlas>>& GetAtlases() const;

private:
    AtlasConfig m_DefaultConfig;
    std::vector<std::unique_ptr<FontAtlas>> m_Atlases;
    Drift::RHI::IDevice* m_Device{nullptr};
};

} // namespace Drift::UI