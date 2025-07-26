#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Texture.h"
#include <algorithm>

namespace Drift::UI {

FontAtlas::FontAtlas(const AtlasConfig& config, Drift::RHI::IDevice* device)
    : m_Config(config)
    , m_Width(config.width)
    , m_Height(config.height)
    , m_CurrentX(0)
    , m_CurrentY(0)
    , m_LineHeight(0)
    , m_Device(device)
    , m_DeviceReady(false) {
    
    LOG_INFO("FontAtlas: criando atlas " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
    
    // Validações de segurança para as dimensões
    if (m_Width <= 0 || m_Height <= 0 || m_Config.channels <= 0) {
        LOG_ERROR("FontAtlas: dimensões inválidas - Width: " + std::to_string(m_Width) + 
                 ", Height: " + std::to_string(m_Height) + 
                 ", Channels: " + std::to_string(m_Config.channels));
        throw std::invalid_argument("FontAtlas: dimensões inválidas");
    }

    // Verificar se o tamanho total não causará overflow
    size_t totalSize = static_cast<size_t>(m_Width) * static_cast<size_t>(m_Height) * static_cast<size_t>(m_Config.channels);
    if (totalSize == 0 || totalSize > SIZE_MAX / 4) { // Verificação de overflow
        LOG_ERROR("FontAtlas: tamanho total inválido ou muito grande: " + std::to_string(totalSize));
        throw std::invalid_argument("FontAtlas: tamanho total inválido");
    }
    
    // Criar dados de textura com verificação de alocação
    try {
        m_TextureData.resize(totalSize, 255); // Inicializar com branco
        LOG_INFO("FontAtlas: dados de textura criados (" + std::to_string(m_TextureData.size()) + " bytes)");
    } catch (const std::exception& e) {
        LOG_ERROR("FontAtlas: falha ao alocar dados de textura: " + std::string(e.what()));
        throw std::runtime_error("FontAtlas: falha na alocação de memória");
    }

    // Verificar se o vetor foi alocado corretamente
    if (m_TextureData.size() != totalSize) {
        LOG_ERROR("FontAtlas: tamanho do vetor incorreto após alocação. Esperado: " + 
                 std::to_string(totalSize) + ", Atual: " + std::to_string(m_TextureData.size()));
        throw std::runtime_error("FontAtlas: falha na alocação de memória");
    }
    
    // Verificar se o device está disponível antes de criar a textura
    if (ValidateDevice()) {
        CreateTexture(device);
    } else {
        LOG_WARNING("FontAtlas: device não disponível - textura será criada quando o device estiver pronto");
    }
    
    LOG_INFO("FontAtlas: atlas criado com sucesso");
}

FontAtlas::~FontAtlas() {
    // Cleanup será feito automaticamente pelo unique_ptr
}

bool FontAtlas::ValidateDevice() const {
    if (!m_Device) {
        return false;
    }
    
    // Tentar criar uma textura de teste para verificar se o device está funcionando
    try {
        Drift::RHI::TextureDesc testDesc;
        testDesc.width = 1;
        testDesc.height = 1;
        testDesc.format = Drift::RHI::Format::R8G8B8A8_UNORM;
        
        auto testTexture = m_Device->CreateTexture(testDesc);
        return testTexture != nullptr;
    } catch (const std::exception& e) {
        LOG_WARNING("FontAtlas: device não está pronto: " + std::string(e.what()));
        return false;
    }
}

bool FontAtlas::IsDeviceReady() const {
    return m_DeviceReady && ValidateDevice();
}

void FontAtlas::SetDevice(Drift::RHI::IDevice* device) {
    m_Device = device;
    
    if (device && ValidateDevice()) {
        m_DeviceReady = true;
        LOG_INFO("FontAtlas: device configurado e validado");
        
        // Tentar criar a textura se ainda não foi criada
        if (!m_SharedTexture) {
            CreateTexture(device);
        }
    } else {
        m_DeviceReady = false;
        LOG_WARNING("FontAtlas: device inválido ou não pronto");
    }
}

AtlasRegion* FontAtlas::AllocateRegion(int width, int height, uint32_t glyphId) {
    // Validações robustas para prevenir overflow
    if (width <= 0 || height <= 0) {
        LOG_ERROR("AllocateRegion: dimensões inválidas para glyph " + std::to_string(glyphId) + 
                 ": " + std::to_string(width) + "x" + std::to_string(height));
        return nullptr;
    }

    // Verificar se as dimensões do atlas são válidas
    if (m_Width <= 0 || m_Height <= 0) {
        LOG_ERROR("AllocateRegion: dimensões do atlas inválidas: " + 
                 std::to_string(m_Width) + "x" + std::to_string(m_Height));
        return nullptr;
    }

    // Verificar se o glyph já foi alocado
    if (m_Regions.find(glyphId) != m_Regions.end()) {
        LOG_WARNING("AllocateRegion: glyph " + std::to_string(glyphId) + " já foi alocado");
        return m_Regions[glyphId].get();
    }

    // Verificar se há espaço suficiente com margem de segurança
    if (m_CurrentX + width > m_Width) {
        // Mover para a próxima linha
        m_CurrentX = 0;
        m_CurrentY += m_LineHeight;
        m_LineHeight = 0;
    }
    
    // Verificar se ainda há espaço vertical
    if (m_CurrentY + height > m_Height) {
        LOG_WARNING("Font atlas full, cannot allocate region for glyph: " + std::to_string(glyphId) + 
                   " (" + std::to_string(width) + "x" + std::to_string(height) + 
                   ") - Atlas: " + std::to_string(m_Width) + "x" + std::to_string(m_Height) + 
                   ", Posição atual: (" + std::to_string(m_CurrentX) + "," + std::to_string(m_CurrentY) + ")");
        return nullptr;
    }

    // Verificação adicional: garantir que a região não cause overflow
    size_t regionSize = static_cast<size_t>(width * height * m_Config.channels);
    size_t atlasSize = static_cast<size_t>(m_Width * m_Height * m_Config.channels);
    
    if (regionSize > atlasSize) {
        LOG_ERROR("AllocateRegion: região muito grande para o atlas. Região: " + 
                 std::to_string(regionSize) + " bytes, Atlas: " + std::to_string(atlasSize) + " bytes");
        return nullptr;
    }

    // Verificar se a posição final não causará overflow
    size_t finalX = static_cast<size_t>(m_CurrentX + width);
    size_t finalY = static_cast<size_t>(m_CurrentY + height);
    
    if (finalX > static_cast<size_t>(m_Width) || finalY > static_cast<size_t>(m_Height)) {
        LOG_ERROR("AllocateRegion: posição final causaria overflow. Final: (" + 
                 std::to_string(finalX) + "," + std::to_string(finalY) + 
                 "), Atlas: " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
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
    
    LOG_DEBUG("AllocateRegion: região alocada para glyph " + std::to_string(glyphId) + 
             " em (" + std::to_string(regionPtr->x) + "," + std::to_string(regionPtr->y) + 
             ") " + std::to_string(width) + "x" + std::to_string(height));
    
    return regionPtr;
}

bool FontAtlas::UploadMSDFData(const AtlasRegion* region, const uint8_t* data, int width, int height) {
    // Método legado - agora usa o sistema de batching
    return QueueMSDFUpload(region, data, width, height);
}

bool FontAtlas::QueueMSDFUpload(const AtlasRegion* region, const uint8_t* data, int width, int height) {
    // Validações robustas dos parâmetros de entrada
    if (!region || !data || width <= 0 || height <= 0) {
        LOG_ERROR("QueueMSDFUpload: parâmetros inválidos - region: " + 
                 std::to_string(region != nullptr) + ", data: " + 
                 std::to_string(data != nullptr) + ", width: " + 
                 std::to_string(width) + ", height: " + std::to_string(height));
        return false;
    }

    // Verificar se o atlas foi inicializado corretamente
    if (m_Width <= 0 || m_Height <= 0 || m_Config.channels <= 0) {
        LOG_ERROR("QueueMSDFUpload: atlas não inicializado corretamente");
        return false;
    }

    // Verificar se o vetor de dados foi alocado
    if (m_TextureData.empty()) {
        LOG_ERROR("QueueMSDFUpload: vetor de dados não alocado");
        return false;
    }

    // Verificar se o device está pronto antes de fazer upload
    if (!IsDeviceReady()) {
        LOG_WARNING("FontAtlas: device não está pronto para upload - dados serão enfileirados");
        // Mesmo sem device, podemos enfileirar os dados para upload posterior
    }

    // Validações de limites da região
    if (region->x < 0 || region->y < 0) {
        LOG_ERROR("QueueMSDFUpload: coordenadas negativas da região: (" + 
                 std::to_string(region->x) + "," + std::to_string(region->y) + ")");
        return false;
    }

    if (region->x + width > m_Width || region->y + height > m_Height) {
        LOG_ERROR("QueueMSDFUpload: região fora dos limites do atlas. Região: (" + 
                 std::to_string(region->x) + "," + std::to_string(region->y) + 
                 ") " + std::to_string(width) + "x" + std::to_string(height) + 
                 ", Atlas: " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
        return false;
    }

    // Verificar se os dados de entrada têm o tamanho correto
    size_t expectedDataSize = static_cast<size_t>(width * height * m_Config.channels);
    if (expectedDataSize == 0) {
        LOG_ERROR("QueueMSDFUpload: tamanho dos dados de entrada é zero");
        return false;
    }

    // Verificar se a região não causará overflow no atlas
    size_t regionSize = static_cast<size_t>(width * height * m_Config.channels);
    size_t atlasSize = static_cast<size_t>(m_Width * m_Height * m_Config.channels);
    
    if (regionSize > atlasSize) {
        LOG_ERROR("QueueMSDFUpload: região muito grande para o atlas. Região: " + 
                 std::to_string(regionSize) + " bytes, Atlas: " + std::to_string(atlasSize) + " bytes");
        return false;
    }

    // Verificar se a posição final não causará overflow
    size_t finalX = static_cast<size_t>(region->x + width);
    size_t finalY = static_cast<size_t>(region->y + height);
    
    if (finalX > static_cast<size_t>(m_Width) || finalY > static_cast<size_t>(m_Height)) {
        LOG_ERROR("QueueMSDFUpload: posição final causaria overflow. Final: (" + 
                 std::to_string(finalX) + "," + std::to_string(finalY) + 
                 "), Atlas: " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
        return false;
    }

    // Adicionar à fila de uploads pendentes com verificação de alocação
    try {
        m_PendingUploads.emplace_back(const_cast<AtlasRegion*>(region), data, width, height);
    } catch (const std::exception& e) {
        LOG_ERROR("QueueMSDFUpload: falha ao adicionar upload à fila: " + std::string(e.what()));
        return false;
    }
    
    // Se atingiu o tamanho do batch e o device está pronto, fazer flush
    if (m_PendingUploads.size() >= static_cast<size_t>(m_Config.batchSize) && IsDeviceReady()) {
        return FlushPendingUploads();
    }
    
    LOG_DEBUG("QueueMSDFUpload: upload enfileirado para região (" + 
             std::to_string(region->x) + "," + std::to_string(region->y) + 
             ") " + std::to_string(width) + "x" + std::to_string(height));
    
    return true;
}

bool FontAtlas::FlushPendingUploads() {
    if (m_PendingUploads.empty()) {
        return true;
    }

    // Verificar se o device está pronto
    if (!IsDeviceReady()) {
        LOG_WARNING("FontAtlas: tentativa de flush com device não pronto - uploads permanecerão pendentes");
        return false;
    }

    if (!m_SharedTexture) {
        LOG_ERROR("FlushPendingUploads called with null texture");
        return false;
    }

    LOG_INFO("FontAtlas: fazendo flush de " + std::to_string(m_PendingUploads.size()) + " uploads pendentes");

    // Aplicar todos os uploads pendentes aos dados da textura
    for (const auto& upload : m_PendingUploads) {
        if (!UploadRegionData(upload.region, upload.data.data(), upload.width, upload.height)) {
            LOG_ERROR("Failed to upload region data during batch flush");
            m_PendingUploads.clear();
            return false;
        }
    }

    // Fazer upload completo dos dados para a GPU com tratamento de exceções
    size_t rowPitch = static_cast<size_t>(m_Width * m_Config.channels);
    size_t slicePitch = rowPitch * static_cast<size_t>(m_Height);
    
    try {
        m_SharedTexture->UpdateSubresource(0, 0, m_TextureData.data(), rowPitch, slicePitch);
        LOG_INFO("FontAtlas: batch upload concluído com sucesso");
    } catch (const std::exception& e) {
        LOG_ERROR("FontAtlas: exceção durante batch upload: " + std::string(e.what()));
        LOG_ERROR("FontAtlas: isso pode indicar que o contexto DX11 não está pronto");
        m_PendingUploads.clear();
        return false;
    }

    // Limpar a fila de uploads pendentes
    m_PendingUploads.clear();
    return true;
}

bool FontAtlas::UploadRegionData(const AtlasRegion* region, const uint8_t* data, int width, int height) {
    // Validações robustas para prevenir buffer overflow
    if (!region || !data || width <= 0 || height <= 0) {
        LOG_ERROR("UploadRegionData: parâmetros inválidos");
        return false;
    }

    // Verificar se o atlas tem dimensões válidas
    if (m_Width <= 0 || m_Height <= 0 || m_Config.channels <= 0) {
        LOG_ERROR("UploadRegionData: dimensões do atlas inválidas");
        return false;
    }

    // Verificar se o vetor de dados tem o tamanho correto
    size_t expectedSize = static_cast<size_t>(m_Width * m_Height * m_Config.channels);
    if (m_TextureData.size() != expectedSize) {
        LOG_ERROR("UploadRegionData: tamanho do vetor de dados incorreto. Esperado: " + 
                 std::to_string(expectedSize) + ", Atual: " + std::to_string(m_TextureData.size()));
        return false;
    }

    // Verificar limites da região com margem de segurança
    if (region->x < 0 || region->y < 0) {
        LOG_ERROR("UploadRegionData: coordenadas negativas da região");
        return false;
    }

    if (region->x + width > m_Width || region->y + height > m_Height) {
        LOG_ERROR("UploadRegionData: região fora dos limites do atlas. Atlas: " + 
                 std::to_string(m_Width) + "x" + std::to_string(m_Height) + 
                 ", Região: (" + std::to_string(region->x) + "," + std::to_string(region->y) + 
                 ") " + std::to_string(width) + "x" + std::to_string(height));
        return false;
    }

    // Verificar se os dados de entrada têm o tamanho correto
    size_t expectedDataSize = static_cast<size_t>(width * height * m_Config.channels);
    if (expectedDataSize == 0) {
        LOG_ERROR("UploadRegionData: tamanho dos dados de entrada é zero");
        return false;
    }

    // Copiar dados recebidos para o armazenamento CPU com verificações adicionais
    for (int row = 0; row < height; ++row) {
        // Calcular offset de destino com verificações
        size_t destRow = static_cast<size_t>(region->y + row);
        if (destRow >= static_cast<size_t>(m_Height)) {
            LOG_ERROR("UploadRegionData: linha de destino fora dos limites: " + std::to_string(destRow));
            return false;
        }

        size_t destOffset = (destRow * static_cast<size_t>(m_Width) + static_cast<size_t>(region->x)) * static_cast<size_t>(m_Config.channels);
        size_t rowSize = static_cast<size_t>(width * m_Config.channels);
        size_t endOffset = destOffset + rowSize;

        // Verificação adicional de limites
        if (endOffset > m_TextureData.size()) {
            LOG_ERROR("UploadRegionData: buffer overflow detectado. Offset final: " + 
                     std::to_string(endOffset) + ", Tamanho do buffer: " + std::to_string(m_TextureData.size()));
            return false;
        }

        // Verificar se o offset de origem é válido
        size_t srcOffset = static_cast<size_t>(row * rowSize);
        if (srcOffset + rowSize > expectedDataSize) {
            LOG_ERROR("UploadRegionData: offset de origem inválido");
            return false;
        }

        // Verificar se os ponteiros são válidos antes da cópia
        if (destOffset >= m_TextureData.size() || srcOffset >= expectedDataSize) {
            LOG_ERROR("UploadRegionData: ponteiros inválidos antes da cópia");
            return false;
        }

        // Fazer a cópia com verificação adicional
        try {
            std::copy_n(data + srcOffset, rowSize, m_TextureData.begin() + destOffset);
        } catch (const std::exception& e) {
            LOG_ERROR("UploadRegionData: exceção durante cópia: " + std::string(e.what()));
            return false;
        }
    }
    
    LOG_DEBUG("UploadRegionData: upload concluído com sucesso para região (" + 
             std::to_string(region->x) + "," + std::to_string(region->y) + 
             ") " + std::to_string(width) + "x" + std::to_string(height));
    
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
    
    // Limpar uploads pendentes também
    m_PendingUploads.clear();
    
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

bool FontAtlas::CreateTexture(Drift::RHI::IDevice* device) {
    if (!device) {
        LOG_ERROR("FontAtlas::CreateTexture: device é nullptr");
        return false;
    }
    
    if (m_SharedTexture) {
        LOG_INFO("FontAtlas::CreateTexture: textura já existe");
        return true;
    }
    
    // Verificar se o device está válido antes de criar a textura
    if (!ValidateDevice()) {
        LOG_WARNING("FontAtlas::CreateTexture: device não está pronto - textura não será criada");
        return false;
    }
    
    try {
        LOG_INFO("FontAtlas::CreateTexture: criando textura " + std::to_string(m_Width) + "x" + std::to_string(m_Height));
        
        // Criar descrição da textura
        Drift::RHI::TextureDesc textureDesc;
        textureDesc.width = m_Width;
        textureDesc.height = m_Height;
        textureDesc.format = Drift::RHI::Format::R8G8B8A8_UNORM;
        
        // Criar a textura real usando o RHI
        auto sharedTexture = device->CreateTexture(textureDesc);
        
        if (sharedTexture) {
            m_SharedTexture = sharedTexture;
            m_DeviceReady = true;
            
            // Se temos dados de textura, fazer upload
            if (!m_TextureData.empty()) {
                size_t rowPitch = static_cast<size_t>(m_Width * m_Config.channels);
                size_t slicePitch = rowPitch * static_cast<size_t>(m_Height);
                
                try {
                    m_SharedTexture->UpdateSubresource(0, 0, m_TextureData.data(), rowPitch, slicePitch);
                } catch (const std::exception& e) {
                    LOG_WARNING("FontAtlas::CreateTexture: falha no upload inicial: " + std::string(e.what()));
                    // Não falhar completamente - a textura foi criada, apenas o upload falhou
                }
            }
            
            LOG_INFO("FontAtlas::CreateTexture: textura criada com sucesso!");
            return true;
        } else {
            LOG_ERROR("FontAtlas::CreateTexture: falha ao criar textura - retornou nullptr");
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("FontAtlas::CreateTexture: exceção ao criar textura: " + std::string(e.what()));
        return false;
    }
}

// MultiAtlasManager implementation

MultiAtlasManager::MultiAtlasManager(const AtlasConfig& defaultConfig)
    : m_DefaultConfig(defaultConfig)
    , m_Device(nullptr) {
}

MultiAtlasManager::~MultiAtlasManager() {
    Clear();
}

FontAtlas* MultiAtlasManager::CreateAtlas(const AtlasConfig& config) {
    auto atlas = std::make_unique<FontAtlas>(config, m_Device);
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

void MultiAtlasManager::FlushAllAtlases() {
    LOG_INFO("MultiAtlasManager: fazendo flush de todos os atlases");
    for (auto& atlas : m_Atlases) {
        if (atlas->HasPendingUploads()) {
            atlas->FlushPendingUploads();
        }
    }
}

void MultiAtlasManager::SetDevice(Drift::RHI::IDevice* device) {
    m_Device = device;
    LOG_INFO("MultiAtlasManager: device configurado");
    
    // Configurar device para todos os atlases existentes
    for (auto& atlas : m_Atlases) {
        atlas->SetDevice(device);
    }
}

size_t MultiAtlasManager::GetAtlasCount() const {
    return m_Atlases.size();
}

const std::vector<std::unique_ptr<FontAtlas>>& MultiAtlasManager::GetAtlases() const {
    return m_Atlases;
}

} // namespace Drift::UI 