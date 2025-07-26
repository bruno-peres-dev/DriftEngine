#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Texture.h"
#include <algorithm>
#include <cstring>

namespace Drift::UI {

FontAtlas::FontAtlas(const AtlasConfig& config, Drift::RHI::IDevice* device)
    : m_Config(config)
    , m_Width(config.width)
    , m_Height(config.height)
    , m_CurrentX(0)
    , m_CurrentY(0)
    , m_LineHeight(0) {
    
    LOG_INFO("FontAtlas: criando atlas " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
    
    // Criar buffer de dados da textura em memória (textura branca por enquanto)
    m_TextureData.resize(m_Width * m_Height * config.channels, 255);
    
    LOG_INFO("FontAtlas: buffer de dados criado (" + std::to_string(m_TextureData.size()) + " bytes)");
    
    // Criar a textura real usando o RHI apenas se o device estiver disponível
    if (device) {
        try {
            LOG_INFO("FontAtlas: criando textura real usando RHI");
            
            // Criar descrição da textura
            Drift::RHI::TextureDesc textureDesc;
            textureDesc.width = m_Width;
            textureDesc.height = m_Height;
            textureDesc.format = Drift::RHI::Format::R8G8B8A8_UNORM;
            
            auto sharedTexture = device->CreateTexture(textureDesc);
            
            if (sharedTexture) {
                // Manter apenas a referência compartilhada para evitar double-free
                m_SharedTexture = sharedTexture;
                LOG_INFO("FontAtlas: textura criada com sucesso!");
            } else {
                LOG_ERROR("FontAtlas: falha ao criar textura - retornou nullptr");
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create atlas texture: " + std::string(e.what()));
        }
    } else {
        LOG_WARNING("FontAtlas: device não disponível - textura será criada quando o device for configurado");
    }
    
    LOG_INFO("FontAtlas: atlas criado com sucesso");
}

FontAtlas::~FontAtlas() {
    // Cleanup será feito automaticamente pelo unique_ptr
}

AtlasRegion* FontAtlas::AllocateRegion(int width, int height, uint32_t glyphId) {
    // Verificar se há espaço suficiente
    if (m_CurrentX + width > m_Width) {
        // Mover para a próxima linha
        m_CurrentX = 0;
        m_CurrentY += m_LineHeight;
        m_LineHeight = 0;
    }
    
    // Verificar se ainda há espaço vertical
    if (m_CurrentY + height > m_Height) {
        LOG_WARNING("Font atlas full, cannot allocate region for glyph: " + std::to_string(glyphId));
        return nullptr;
    }
    
    // Criar nova região
    auto region = std::make_unique<AtlasRegion>();
    region->x = m_CurrentX;
    region->y = m_CurrentY;
    region->width = width;
    region->height = height;
    region->glyphId = glyphId;
    
    // Atualizar posição atual
    m_CurrentX += width;
    m_LineHeight = std::max(m_LineHeight, static_cast<int>(height));
    
    // Armazenar a região
    AtlasRegion* regionPtr = region.get();
    m_Regions[glyphId] = std::move(region);
    
    return regionPtr;
}

bool FontAtlas::UploadMSDFData(const AtlasRegion* region, const uint8_t* data, int width, int height) {
    if (!region || !data) {
        LOG_ERROR("Invalid parameters for MSDF data upload: region=" + std::to_string(region != nullptr) + 
                  ", data=" + std::to_string(data != nullptr));
        return false;
    }
    
    if (!m_SharedTexture) {
        LOG_ERROR("FontAtlas::UploadMSDFData: textura não disponível (m_SharedTexture é nullptr)");
        return false;
    }
    
    // Verificar se a região é válida
    if (region->x + width > m_Width || region->y + height > m_Height) {
        LOG_ERROR("Region outside atlas bounds: region(" + std::to_string(region->x) + "," + std::to_string(region->y) + 
                  ") size(" + std::to_string(width) + "x" + std::to_string(height) + 
                  ") atlas(" + std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");
        return false;
    }
    
    LOG_INFO("FontAtlas::UploadMSDFData: fazendo upload de glifo " + std::to_string(region->glyphId) + 
             " em região (" + std::to_string(region->x) + ", " + std::to_string(region->y) + 
             ") tamanho (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    
    try {
        // Garante que o buffer em CPU exista
        if (m_TextureData.empty()) {
            LOG_INFO("FontAtlas::UploadMSDFData: criando buffer de textura (" + 
                     std::to_string(m_Width) + "x" + std::to_string(m_Height) + "x" + std::to_string(m_Config.channels) + ")");
            m_TextureData.resize(static_cast<size_t>(m_Width * m_Height * m_Config.channels), 255);
        }
        
        // Copia linha a linha para a região do atlas
        for (int y = 0; y < height; ++y) {
            size_t dstOffset = ((region->y + y) * m_Width + region->x) * m_Config.channels;
            size_t srcOffset = y * width * m_Config.channels;
            std::memcpy(m_TextureData.data() + dstOffset,
                        data + srcOffset,
                        width * m_Config.channels);
        }
        
        // Envia o atlas todo para a GPU
        size_t rowPitch = static_cast<size_t>(m_Width * m_Config.channels);
        LOG_INFO("FontAtlas::UploadMSDFData: enviando para GPU - rowPitch=" + std::to_string(rowPitch) + 
                 ", slicePitch=" + std::to_string(rowPitch * m_Height));
        
        m_SharedTexture->UpdateSubresource(0, 0,
                                           m_TextureData.data(),
                                           rowPitch,
                                           rowPitch * m_Height);
        
        LOG_INFO("FontAtlas::UploadMSDFData: upload concluído para glifo " + std::to_string(region->glyphId));
        
    } catch (const std::exception& e) {
        LOG_ERROR("FontAtlas::UploadMSDFData: exceção durante upload: " + std::string(e.what()));
        return false;
    } catch (...) {
        LOG_ERROR("FontAtlas::UploadMSDFData: exceção desconhecida durante upload");
        return false;
    }
    
    return true;
}

bool FontAtlas::CreateTexture(Drift::RHI::IDevice* device) {
    if (!device) {
        LOG_ERROR("FontAtlas::CreateTexture: device é nullptr");
        return false;
    }
    
    if (m_SharedTexture) {
        LOG_WARNING("FontAtlas::CreateTexture: textura já existe");
        return true;
    }
    
    try {
        LOG_INFO("FontAtlas::CreateTexture: criando textura " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
        
        // Criar descrição da textura
        Drift::RHI::TextureDesc textureDesc;
        textureDesc.width = m_Width;
        textureDesc.height = m_Height;
        textureDesc.format = Drift::RHI::Format::R8G8B8A8_UNORM;
        
        auto sharedTexture = device->CreateTexture(textureDesc);
        
        if (sharedTexture) {
            m_SharedTexture = sharedTexture;
            LOG_INFO("FontAtlas::CreateTexture: textura criada com sucesso!");
            
            // Se já temos dados no buffer, fazer upload inicial
            if (!m_TextureData.empty()) {
                LOG_INFO("FontAtlas::CreateTexture: fazendo upload inicial dos dados");
                size_t rowPitch = static_cast<size_t>(m_Width * m_Config.channels);
                m_SharedTexture->UpdateSubresource(0, 0,
                                                   m_TextureData.data(),
                                                   rowPitch,
                                                   rowPitch * m_Height);
            }
            
            return true;
        } else {
            LOG_ERROR("FontAtlas::CreateTexture: falha ao criar textura - retornou nullptr");
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("FontAtlas::CreateTexture: exceção durante criação: " + std::string(e.what()));
        return false;
    }
}

AtlasRegion* FontAtlas::GetRegion(uint32_t glyphId) const {
    auto it = m_Regions.find(glyphId);
    if (it != m_Regions.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool FontAtlas::HasRegion(uint32_t glyphId) const {
    return m_Regions.find(glyphId) != m_Regions.end();
}

void FontAtlas::Clear() {
    m_Regions.clear();
    m_CurrentX = 0;
    m_CurrentY = 0;
    m_LineHeight = 0;
    
    LOG_INFO("Font atlas cleared");
}

size_t FontAtlas::GetRegionCount() const {
    return m_Regions.size();
}

float FontAtlas::GetUsagePercentage() const {
    if (m_Width <= 0 || m_Height <= 0) {
        return 0.0f;
    }
    
    int totalArea = m_Width * m_Height;
    int usedArea = 0;
    
    for (const auto& pair : m_Regions) {
        const auto& region = pair.second;
        usedArea += region->width * region->height;
    }
    
    return static_cast<float>(usedArea) / static_cast<float>(totalArea) * 100.0f;
}

Drift::RHI::ITexture* FontAtlas::GetTexture() const {
    if (m_SharedTexture) {
        return m_SharedTexture.get();
    } else {
        LOG_WARNING("FontAtlas::GetTexture: textura é nullptr!");
        return nullptr;
    }
}

int FontAtlas::GetWidth() const {
    return m_Width;
}

int FontAtlas::GetHeight() const {
    return m_Height;
}

const AtlasConfig& FontAtlas::GetConfig() const {
    return m_Config;
}

// MultiAtlasManager implementation

MultiAtlasManager::MultiAtlasManager(const AtlasConfig& defaultConfig)
    : m_DefaultConfig(defaultConfig) {
}

MultiAtlasManager::~MultiAtlasManager() {
    Clear();
}

FontAtlas* MultiAtlasManager::CreateAtlas(const AtlasConfig& config) {
    auto atlas = std::make_unique<FontAtlas>(config);
    FontAtlas* atlasPtr = atlas.get();
    m_Atlases.push_back(std::move(atlas));
    
    LOG_INFO("Created new font atlas: " + std::to_string(config.width) + "x" + std::to_string(config.height));
    
    return atlasPtr;
}

FontAtlas* MultiAtlasManager::GetAtlasForGlyph(uint32_t glyphId) {
    // Primeiro, procurar em atlases existentes
    for (auto& atlas : m_Atlases) {
        if (atlas->HasRegion(glyphId)) {
            return atlas.get();
        }
    }
    
    // Se não encontrado, procurar um atlas com espaço
    for (auto& atlas : m_Atlases) {
        if (atlas->GetUsagePercentage() < 90.0f) { // 90% de uso máximo
            return atlas.get();
        }
    }
    
    // Se não há atlas disponível, criar um novo
    return CreateAtlas(m_DefaultConfig);
}

void MultiAtlasManager::Clear() {
    m_Atlases.clear();
    LOG_INFO("All font atlases cleared");
}

size_t MultiAtlasManager::GetAtlasCount() const {
    return m_Atlases.size();
}

const std::vector<std::unique_ptr<FontAtlas>>& MultiAtlasManager::GetAtlases() const {
    return m_Atlases;
}

} // namespace Drift::UI 