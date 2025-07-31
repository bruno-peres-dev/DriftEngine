#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Profiler.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Texture.h"
#include <algorithm>
#include <cstring>

namespace Drift::UI {

// Singleton instance
static FontAtlasManager* s_AtlasManagerInstance = nullptr;

FontAtlas::FontAtlas(Drift::RHI::IDevice* device, const FontAtlasConfig& config)
    : m_Device(device)
    , m_Config(config) {
    
    DRIFT_PROFILE_FUNCTION();
    
    // Inicializar dados do atlas
    m_AtlasData.resize(config.width * config.height * 4); // RGBA
    std::fill(m_AtlasData.begin(), m_AtlasData.end(), 0);
    
    // Criar árvore de packing
    m_Root = std::make_unique<AtlasNode>(0, 0, config.width, config.height);
    
    // Criar textura RHI
    CreateTexture();
    
    DRIFT_LOG_INFO("FontAtlas criado: {}x{}", config.width, config.height);
}

FontAtlas::~FontAtlas() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("FontAtlas destruído");
}

bool FontAtlas::AddGlyph(uint32_t codepoint, const std::vector<unsigned char>& bitmap, 
                         int width, int height, const GlyphInfo& info) {
    DRIFT_PROFILE_FUNCTION();
    
    if (IsFull()) {
        DRIFT_LOG_WARNING("FontAtlas está cheio, não é possível adicionar mais glyphs");
        return false;
    }
    
    // Calcular tamanho necessário incluindo padding e borda
    int requiredWidth = width + m_Config.padding * 2 + m_Config.border * 2;
    int requiredHeight = height + m_Config.padding * 2 + m_Config.border * 2;
    
    // Alocar espaço no atlas
    int x, y;
    if (!AllocateSpace(requiredWidth, requiredHeight, x, y)) {
        DRIFT_LOG_WARNING("Não foi possível alocar espaço para glyph {} no atlas", codepoint);
        return false;
    }
    
    // Copiar dados do bitmap para o atlas
    CopyBitmapToAtlas(bitmap, width, height, x + m_Config.padding + m_Config.border, 
                      y + m_Config.padding + m_Config.border);
    
    // Criar informações do glyph
    GlyphInfo atlasInfo = info;
    atlasInfo.uv0 = glm::vec2(
        static_cast<float>(x + m_Config.padding + m_Config.border) / m_Config.width,
        static_cast<float>(y + m_Config.padding + m_Config.border) / m_Config.height
    );
    atlasInfo.uv1 = glm::vec2(
        static_cast<float>(x + m_Config.padding + m_Config.border + width) / m_Config.width,
        static_cast<float>(y + m_Config.padding + m_Config.border + height) / m_Config.height
    );
    
    // Armazenar glyph
    m_Glyphs[codepoint] = atlasInfo;
    m_Regions.emplace_back(x, y, requiredWidth, requiredHeight, codepoint);
    
    // Atualizar textura
    UpdateTexture();
    
    return true;
}

bool FontAtlas::AddGlyphSDF(uint32_t codepoint, const std::vector<float>& sdfData,
                            int width, int height, const GlyphInfo& info) {
    DRIFT_PROFILE_FUNCTION();
    
    if (IsFull()) {
        return false;
    }
    
    // Calcular tamanho necessário
    int requiredWidth = width + m_Config.padding * 2 + m_Config.border * 2;
    int requiredHeight = height + m_Config.padding * 2 + m_Config.border * 2;
    
    // Alocar espaço
    int x, y;
    if (!AllocateSpace(requiredWidth, requiredHeight, x, y)) {
        return false;
    }
    
    // Converter SDF para RGBA
    std::vector<unsigned char> rgbaData(width * height * 4);
    for (int i = 0; i < width * height; ++i) {
        float sdfValue = sdfData[i];
        unsigned char alpha = static_cast<unsigned char>(std::clamp(sdfValue * 255.0f, 0.0f, 255.0f));
        rgbaData[i * 4 + 0] = 255; // R
        rgbaData[i * 4 + 1] = 255; // G
        rgbaData[i * 4 + 2] = 255; // B
        rgbaData[i * 4 + 3] = alpha; // A
    }
    
    // Copiar para atlas
    CopyRGBAToAtlas(rgbaData, width, height, x + m_Config.padding + m_Config.border, 
                    y + m_Config.padding + m_Config.border);
    
    // Criar informações do glyph
    GlyphInfo atlasInfo = info;
    atlasInfo.renderType = GlyphRenderType::SDF;
    atlasInfo.uv0 = glm::vec2(
        static_cast<float>(x + m_Config.padding + m_Config.border) / m_Config.width,
        static_cast<float>(y + m_Config.padding + m_Config.border) / m_Config.height
    );
    atlasInfo.uv1 = glm::vec2(
        static_cast<float>(x + m_Config.padding + m_Config.border + width) / m_Config.width,
        static_cast<float>(y + m_Config.padding + m_Config.border + height) / m_Config.height
    );
    
    m_Glyphs[codepoint] = atlasInfo;
    m_Regions.emplace_back(x, y, requiredWidth, requiredHeight, codepoint);
    
    UpdateTexture();
    return true;
}

bool FontAtlas::AddGlyphMSDF(uint32_t codepoint, const std::vector<glm::vec3>& msdfData,
                             int width, int height, const GlyphInfo& info) {
    DRIFT_PROFILE_FUNCTION();
    
    if (IsFull()) {
        return false;
    }
    
    // Calcular tamanho necessário
    int requiredWidth = width + m_Config.padding * 2 + m_Config.border * 2;
    int requiredHeight = height + m_Config.padding * 2 + m_Config.border * 2;
    
    // Alocar espaço
    int x, y;
    if (!AllocateSpace(requiredWidth, requiredHeight, x, y)) {
        return false;
    }
    
    // Converter MSDF para RGBA
    std::vector<unsigned char> rgbaData(width * height * 4);
    for (int i = 0; i < width * height; ++i) {
        const glm::vec3& msdf = msdfData[i];
        rgbaData[i * 4 + 0] = static_cast<unsigned char>(std::clamp(msdf.r * 255.0f, 0.0f, 255.0f));
        rgbaData[i * 4 + 1] = static_cast<unsigned char>(std::clamp(msdf.g * 255.0f, 0.0f, 255.0f));
        rgbaData[i * 4 + 2] = static_cast<unsigned char>(std::clamp(msdf.b * 255.0f, 0.0f, 255.0f));
        rgbaData[i * 4 + 3] = 255; // Alpha sempre 255 para MSDF
    }
    
    // Copiar para atlas
    CopyRGBAToAtlas(rgbaData, width, height, x + m_Config.padding + m_Config.border, 
                    y + m_Config.padding + m_Config.border);
    
    // Criar informações do glyph
    GlyphInfo atlasInfo = info;
    atlasInfo.renderType = GlyphRenderType::MSDF;
    atlasInfo.uv0 = glm::vec2(
        static_cast<float>(x + m_Config.padding + m_Config.border) / m_Config.width,
        static_cast<float>(y + m_Config.padding + m_Config.border) / m_Config.height
    );
    atlasInfo.uv1 = glm::vec2(
        static_cast<float>(x + m_Config.padding + m_Config.border + width) / m_Config.width,
        static_cast<float>(y + m_Config.padding + m_Config.border + height) / m_Config.height
    );
    
    m_Glyphs[codepoint] = atlasInfo;
    m_Regions.emplace_back(x, y, requiredWidth, requiredHeight, codepoint);
    
    UpdateTexture();
    return true;
}

const GlyphInfo* FontAtlas::GetGlyph(uint32_t codepoint) const {
    auto it = m_Glyphs.find(codepoint);
    if (it != m_Glyphs.end()) {
        return &it->second;
    }
    return nullptr;
}

bool FontAtlas::HasGlyph(uint32_t codepoint) const {
    return m_Glyphs.find(codepoint) != m_Glyphs.end();
}

void FontAtlas::Clear() {
    DRIFT_PROFILE_FUNCTION();
    
    m_Glyphs.clear();
    m_Regions.clear();
    std::fill(m_AtlasData.begin(), m_AtlasData.end(), 0);
    
    // Reconstruir árvore de packing
    m_Root = std::make_unique<AtlasNode>(0, 0, m_Config.width, m_Config.height);
    
    UpdateTexture();
}

void FontAtlas::Rebuild() {
    DRIFT_PROFILE_FUNCTION();
    
    // Limpar dados
    std::fill(m_AtlasData.begin(), m_AtlasData.end(), 0);
    
    // Reconstruir árvore
    m_Root = std::make_unique<AtlasNode>(0, 0, m_Config.width, m_Config.height);
    
    // Re-adicionar todos os glyphs
    auto glyphs = m_Glyphs;
    m_Glyphs.clear();
    m_Regions.clear();
    
    for (const auto& pair : glyphs) {
        // TODO: Re-adicionar glyphs com seus dados originais
        // Isso requereria armazenar os dados originais dos bitmaps
    }
    
    UpdateTexture();
}

bool FontAtlas::IsFull() const {
    return m_Root->used;
}

float FontAtlas::GetUsagePercentage() const {
    if (m_Regions.empty()) {
        return 0.0f;
    }
    
    size_t usedPixels = 0;
    for (const auto& region : m_Regions) {
        usedPixels += region.width * region.height;
    }
    
    return static_cast<float>(usedPixels) / (m_Config.width * m_Config.height) * 100.0f;
}

size_t FontAtlas::GetMemoryUsage() const {
    size_t usage = 0;
    
    // Dados do atlas
    usage += m_AtlasData.size();
    
    // Glyphs
    usage += m_Glyphs.size() * sizeof(GlyphInfo);
    
    // Regiões
    usage += m_Regions.size() * sizeof(AtlasRegion);
    
    // Textura RHI
    if (m_Texture) {
        usage += m_Config.width * m_Config.height * 4; // Estimativa
    }
    
    return usage;
}

void FontAtlas::OptimizeLayout() {
    DRIFT_PROFILE_FUNCTION();
    
    // Ordenar regiões por altura (para melhor packing)
    SortRegionsByHeight();
    
    // Compactar regiões
    CompactRegions();
}

void FontAtlas::Defragment() {
    DRIFT_PROFILE_FUNCTION();
    
    // TODO: Implementar defragmentação do atlas
    // Isso envolveria mover glyphs para posições mais compactas
    DRIFT_LOG_INFO("Defragmentação do atlas não implementada ainda");
}

// Métodos privados
bool FontAtlas::AllocateSpace(int width, int height, int& x, int& y) {
    DRIFT_PROFILE_FUNCTION();
    
    AtlasNode* node = FindNode(m_Root.get(), width, height);
    if (!node) {
        return false;
    }
    
    // Dividir o nó se necessário
    if (node->width != width || node->height != height) {
        node = SplitNode(node, width, height);
    }
    
    node->used = true;
    x = node->x;
    y = node->y;
    
    return true;
}

AtlasNode* FontAtlas::FindNode(AtlasNode* node, int width, int height) {
    if (node->used) {
        return nullptr;
    }
    
    if (node->width < width || node->height < height) {
        return nullptr;
    }
    
    if (node->width == width && node->height == height) {
        return node;
    }
    
    // Tentar dividir o nó
    if (node->width - width > node->height - height) {
        // Dividir horizontalmente
        if (!node->left) {
            node->left = std::make_unique<AtlasNode>(node->x, node->y, width, node->height);
            node->right = std::make_unique<AtlasNode>(node->x + width, node->y, node->width - width, node->height);
        }
        return FindNode(node->left.get(), width, height);
    } else {
        // Dividir verticalmente
        if (!node->left) {
            node->left = std::make_unique<AtlasNode>(node->x, node->y, node->width, height);
            node->right = std::make_unique<AtlasNode>(node->x, node->y + height, node->width, node->height - height);
        }
        return FindNode(node->left.get(), width, height);
    }
}

AtlasNode* FontAtlas::SplitNode(AtlasNode* node, int width, int height) {
    if (node->width - width > node->height - height) {
        // Dividir horizontalmente
        node->left = std::make_unique<AtlasNode>(node->x, node->y, width, node->height);
        node->right = std::make_unique<AtlasNode>(node->x + width, node->y, node->width - width, node->height);
        return node->left.get();
    } else {
        // Dividir verticalmente
        node->left = std::make_unique<AtlasNode>(node->x, node->y, node->width, height);
        node->right = std::make_unique<AtlasNode>(node->x, node->y + height, node->width, node->height - height);
        return node->left.get();
    }
}

void FontAtlas::CopyBitmapToAtlas(const std::vector<unsigned char>& bitmap, int width, int height, int x, int y) {
    DRIFT_PROFILE_FUNCTION();
    
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            int atlasIndex = ((y + row) * m_Config.width + (x + col)) * 4;
            int bitmapIndex = row * width + col;
            
            unsigned char alpha = bitmap[bitmapIndex];
            m_AtlasData[atlasIndex + 0] = 255; // R
            m_AtlasData[atlasIndex + 1] = 255; // G
            m_AtlasData[atlasIndex + 2] = 255; // B
            m_AtlasData[atlasIndex + 3] = alpha; // A
        }
    }
}

void FontAtlas::CopyRGBAToAtlas(const std::vector<unsigned char>& rgbaData, int width, int height, int x, int y) {
    DRIFT_PROFILE_FUNCTION();
    
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            int atlasIndex = ((y + row) * m_Config.width + (x + col)) * 4;
            int rgbaIndex = (row * width + col) * 4;
            
            m_AtlasData[atlasIndex + 0] = rgbaData[rgbaIndex + 0]; // R
            m_AtlasData[atlasIndex + 1] = rgbaData[rgbaIndex + 1]; // G
            m_AtlasData[atlasIndex + 2] = rgbaData[rgbaIndex + 2]; // B
            m_AtlasData[atlasIndex + 3] = rgbaData[rgbaIndex + 3]; // A
        }
    }
}

void FontAtlas::CreateTexture() {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Device) {
        DRIFT_LOG_ERROR("Dispositivo RHI não disponível para criar textura do atlas");
        return;
    }
    
    // Criar descrição da textura
    Drift::RHI::TextureDesc desc;
    desc.width = m_Config.width;
    desc.height = m_Config.height;
    desc.depth = 1;
    desc.mipLevels = m_Config.enableMipmaps ? 0 : 1;
    desc.arraySize = 1;
    desc.format = Drift::RHI::Format::RGBA8_UNORM;
    desc.usage = Drift::RHI::TextureUsage::ShaderResource | Drift::RHI::TextureUsage::RenderTarget;
    desc.bindFlags = Drift::RHI::TextureBindFlags::ShaderResource;
    
    // Criar textura
    m_Texture = m_Device->CreateTexture(desc);
    if (!m_Texture) {
        DRIFT_LOG_ERROR("Falha ao criar textura do atlas");
        return;
    }
    
    // Atualizar dados da textura
    UpdateTexture();
}

void FontAtlas::UpdateTexture() {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Texture || m_AtlasData.empty()) {
        return;
    }
    
    // Atualizar dados da textura
    Drift::RHI::TextureUpdateDesc updateDesc;
    updateDesc.data = m_AtlasData.data();
    updateDesc.dataSize = m_AtlasData.size();
    updateDesc.mipLevel = 0;
    updateDesc.arraySlice = 0;
    
    m_Texture->Update(updateDesc);
}

void FontAtlas::ClearNode(AtlasNode* node) {
    if (!node) {
        return;
    }
    
    ClearNode(node->left.get());
    ClearNode(node->right.get());
    
    node->used = false;
    node->left.reset();
    node->right.reset();
}

void FontAtlas::SortRegionsByHeight() {
    std::sort(m_Regions.begin(), m_Regions.end(), 
              [](const AtlasRegion& a, const AtlasRegion& b) {
                  return a.height > b.height;
              });
}

void FontAtlas::CompactRegions() {
    // TODO: Implementar compactação de regiões
    // Isso envolveria mover regiões para posições mais próximas
}

bool FontAtlas::CanFitRegion(const AtlasRegion& region, int x, int y) const {
    return x + region.width <= m_Config.width && y + region.height <= m_Config.height;
}

// FontAtlasManager Implementation
FontAtlasManager& FontAtlasManager::GetInstance() {
    if (!s_AtlasManagerInstance) {
        s_AtlasManagerInstance = new FontAtlasManager();
    }
    return *s_AtlasManagerInstance;
}

std::shared_ptr<FontAtlas> FontAtlasManager::CreateAtlas(const FontAtlasConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_Device) {
        DRIFT_LOG_ERROR("Dispositivo RHI não configurado para FontAtlasManager");
        return nullptr;
    }
    
    auto atlas = std::make_shared<FontAtlas>(m_Device, config);
    m_Atlas.push_back(atlas);
    
    return atlas;
}

std::shared_ptr<FontAtlas> FontAtlasManager::GetAtlas(const FontAtlasConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    // Verificar cache primeiro
    auto it = m_AtlasCache.find(config);
    if (it != m_AtlasCache.end()) {
        return it->second;
    }
    
    // Criar novo atlas
    auto atlas = CreateAtlas(config);
    if (atlas) {
        m_AtlasCache[config] = atlas;
    }
    
    return atlas;
}

void FontAtlasManager::DestroyAtlas(std::shared_ptr<FontAtlas> atlas) {
    DRIFT_PROFILE_FUNCTION();
    
    // Remover do cache
    for (auto it = m_AtlasCache.begin(); it != m_AtlasCache.end(); ++it) {
        if (it->second == atlas) {
            m_AtlasCache.erase(it);
            break;
        }
    }
    
    // Remover da lista
    auto it = std::find(m_Atlas.begin(), m_Atlas.end(), atlas);
    if (it != m_Atlas.end()) {
        m_Atlas.erase(it);
    }
}

void FontAtlasManager::OptimizeAllAtlas() {
    DRIFT_PROFILE_FUNCTION();
    
    for (auto& atlas : m_Atlas) {
        atlas->OptimizeLayout();
    }
}

void FontAtlasManager::ClearUnusedAtlas() {
    DRIFT_PROFILE_FUNCTION();
    
    // Remover atlas vazios
    m_Atlas.erase(
        std::remove_if(m_Atlas.begin(), m_Atlas.end(),
                      [](const std::shared_ptr<FontAtlas>& atlas) {
                          return atlas->GetGlyphCount() == 0;
                      }),
        m_Atlas.end()
    );
}

size_t FontAtlasManager::GetAtlasCount() const {
    return m_Atlas.size();
}

size_t FontAtlasManager::GetTotalMemoryUsage() const {
    size_t totalUsage = 0;
    for (const auto& atlas : m_Atlas) {
        totalUsage += atlas->GetMemoryUsage();
    }
    return totalUsage;
}

} // namespace Drift::UI 