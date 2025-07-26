#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/Core/Log.h"
#include <algorithm>

namespace Drift::UI {

FontAtlas::FontAtlas(const AtlasConfig& config)
    : m_Config(config)
    , m_Width(config.width)
    , m_Height(config.height)
    , m_CurrentX(0)
    , m_CurrentY(0)
    , m_LineHeight(0) {
    
    Core::Log("[FontAtlas] Construtor chamado");
    Core::Log("[FontAtlas]   - Configuracao: " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
    Core::Log("[FontAtlas]   - Padding: " + std::to_string(config.padding));
    Core::Log("[FontAtlas]   - Canais: " + std::to_string(config.channels));
    Core::Log("[FontAtlas]   - Usar MSDF: " + std::string(config.useMSDF ? "sim" : "nao"));
    Core::Log("[FontAtlas]   - Tamanho MSDF: " + std::to_string(config.msdfSize));
    
    // Inicializar a textura (implementação stub)
    LOG_INFO("Creating font atlas: " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
    
    Core::Log("[FontAtlas] Construtor concluido com sucesso");
}

FontAtlas::~FontAtlas() {
    Core::Log("[FontAtlas] Destrutor chamado");
    Core::Log("[FontAtlas]   - Total de regioes: " + std::to_string(m_Regions.size()));
    // Cleanup será feito automaticamente pelo unique_ptr
    Core::Log("[FontAtlas] Destrutor concluido");
}

AtlasRegion* FontAtlas::AllocateRegion(int width, int height, uint32_t glyphId) {
    Core::Log("[FontAtlas] AllocateRegion chamado para glyph " + std::to_string(glyphId) + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    Core::Log("[FontAtlas]   - Posicao atual: (" + std::to_string(m_CurrentX) + ", " + std::to_string(m_CurrentY) + ")");
    Core::Log("[FontAtlas]   - Tamanho do atlas: " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
    
    // Verificar se há espaço suficiente
    if (m_CurrentX + width > m_Width) {
        Core::Log("[FontAtlas]   - Movendo para proxima linha");
        // Mover para a próxima linha
        m_CurrentX = 0;
        m_CurrentY += m_LineHeight;
        m_LineHeight = 0;
        Core::Log("[FontAtlas]   - Nova posicao: (" + std::to_string(m_CurrentX) + ", " + std::to_string(m_CurrentY) + ")");
    }
    
    // Verificar se ainda há espaço vertical
    if (m_CurrentY + height > m_Height) {
        Core::Log("[FontAtlas] ERRO: Atlas cheio, nao pode alocar regiao para glyph: " + std::to_string(glyphId));
        LOG_WARNING("Font atlas full, cannot allocate region for glyph: " + std::to_string(glyphId));
        return nullptr;
    }
    
    Core::Log("[FontAtlas]   - Criando nova regiao...");
    // Criar nova região
    auto region = std::make_unique<AtlasRegion>();
    region->x = m_CurrentX;
    region->y = m_CurrentY;
    region->width = width;
    region->height = height;
    region->glyphId = glyphId;
    
    Core::Log("[FontAtlas]   - Regiao criada: (" + std::to_string(region->x) + ", " + std::to_string(region->y) + ") " + std::to_string(region->width) + "x" + std::to_string(region->height));
    
    // Atualizar posição atual
    m_CurrentX += width;
    m_LineHeight = std::max(m_LineHeight, static_cast<int>(height));
    
    Core::Log("[FontAtlas]   - Nova posicao atual: (" + std::to_string(m_CurrentX) + ", " + std::to_string(m_CurrentY) + ")");
    Core::Log("[FontAtlas]   - LineHeight atualizado: " + std::to_string(m_LineHeight));
    
    // Armazenar a região
    AtlasRegion* regionPtr = region.get();
    m_Regions[glyphId] = std::move(region);
    
    Core::Log("[FontAtlas]   - Regiao armazenada no mapa. Total de regioes: " + std::to_string(m_Regions.size()));
    Core::Log("[FontAtlas]   - Regiao alocada com sucesso para glyph " + std::to_string(glyphId));
    
    LOG_DEBUG("Allocated atlas region for glyph " + std::to_string(glyphId) + ": (" + std::to_string(region->x) + ", " + std::to_string(region->y) + ") " + std::to_string(region->width) + "x" + std::to_string(region->height));
    
    return regionPtr;
}

bool FontAtlas::UploadMSDFData(const AtlasRegion* region, const uint8_t* data, int width, int height) {
    if (!region || !data) {
        LOG_ERROR("Invalid parameters for MSDF data upload");
        return false;
    }
    
    // Verificar se a região é válida
    if (region->x + region->width > m_Width || region->y + region->height > m_Height) {
        LOG_ERROR("Region outside atlas bounds");
        return false;
    }
    
    // Aqui seria implementada a lógica real de upload para a textura
    // Por enquanto, vamos apenas simular o upload
    
    LOG_DEBUG("Uploading MSDF data for glyph " + std::to_string(region->glyphId) + ": " + std::to_string(width) + "x" + std::to_string(height) + " at (" + std::to_string(region->x) + ", " + std::to_string(region->y) + ")");
    
    return true;
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
    return m_Texture.get();
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