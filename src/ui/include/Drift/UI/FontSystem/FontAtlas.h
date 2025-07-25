#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Drift/RHI/Texture.h"

namespace Drift::UI {

// Estrutura para representar uma região no atlas
struct AtlasRegion {
    glm::vec2 position;      // Posição no atlas (pixels)
    glm::vec2 size;          // Tamanho da região (pixels)
    glm::vec2 uvMin;         // Coordenadas UV mínimas
    glm::vec2 uvMax;         // Coordenadas UV máximas
    bool isOccupied;
    uint32_t glyphId;        // ID do glyph associado
};

// Configurações do atlas
struct AtlasConfig {
    int width = 2048;        // Largura do atlas em pixels
    int height = 2048;       // Altura do atlas em pixels
    int padding = 2;         // Padding entre glyphs
    int channels = 4;        // Canais de cor (RGBA)
    bool useMSDF = true;     // Usar MSDF ou bitmap normal
    int msdfSize = 32;       // Tamanho do MSDF em pixels
};

// Classe para gerenciar o atlas de fontes
class FontAtlas {
public:
    FontAtlas(const AtlasConfig& config = AtlasConfig{});
    ~FontAtlas();

    // Gerenciamento de regiões
    AtlasRegion* AllocateRegion(int width, int height, uint32_t glyphId);
    void FreeRegion(uint32_t glyphId);
    AtlasRegion* FindRegion(uint32_t glyphId);
    
    // Upload de dados
    bool UploadGlyphData(const AtlasRegion* region, const uint8_t* data, int width, int height, int channels);
    bool UploadMSDFData(const AtlasRegion* region, const uint8_t* data, int width, int height);
    
    // Acesso à textura
    Drift::RHI::ITexture* GetTexture() const { return m_Texture.get(); }
    bool IsDirty() const { return m_IsDirty; }
    void MarkClean() { m_IsDirty = false; }
    
    // Informações do atlas
    const AtlasConfig& GetConfig() const { return m_Config; }
    float GetUsage() const; // Percentual de uso do atlas
    
    // Otimização
    void Defragment();
    void Resize(int newWidth, int newHeight);

private:
    // Configuração do atlas
    AtlasConfig m_Config;
    
    // Textura do atlas
    std::unique_ptr<Drift::RHI::ITexture> m_Texture;
    std::vector<uint8_t> m_TextureData;
    bool m_IsDirty;
    
    // Gerenciamento de regiões
    std::vector<AtlasRegion> m_Regions;
    std::vector<uint32_t> m_FreeRegions;
    
    // Métodos internos
    bool InitializeTexture();
    bool FindBestFit(int width, int height, AtlasRegion& region);
    void SplitRegion(AtlasRegion& region, int width, int height);
    void MergeAdjacentRegions();
    void UpdateTextureData();
    
    // Algoritmo de empacotamento
    struct PackingNode {
        glm::vec2 position;
        glm::vec2 size;
        bool isLeaf;
        std::unique_ptr<PackingNode> left;
        std::unique_ptr<PackingNode> right;
    };
    
    std::unique_ptr<PackingNode> m_PackingTree;
    bool InsertIntoTree(PackingNode* node, int width, int height, AtlasRegion& region);
    void RebuildPackingTree();
};

// Gerenciador de múltiplos atlas (para fontes grandes)
class MultiAtlasManager {
public:
    MultiAtlasManager(const AtlasConfig& baseConfig = AtlasConfig{});
    ~MultiAtlasManager();

    // Alocação de glyphs
    AtlasRegion* AllocateGlyph(int width, int height, uint32_t glyphId);
    void FreeGlyph(uint32_t glyphId);
    AtlasRegion* FindGlyph(uint32_t glyphId);
    
    // Upload de dados
    bool UploadGlyphData(uint32_t glyphId, const uint8_t* data, int width, int height, int channels);
    bool UploadMSDFData(uint32_t glyphId, const uint8_t* data, int width, int height);
    
    // Acesso às texturas
    std::vector<Drift::RHI::ITexture*> GetTextures() const;
    bool HasDirtyTextures() const;
    void MarkAllClean();
    
    // Otimização
    void Optimize();
    void Clear();

private:
    AtlasConfig m_BaseConfig;
    std::vector<std::unique_ptr<FontAtlas>> m_Atlases;
    std::unordered_map<uint32_t, std::pair<FontAtlas*, AtlasRegion*>> m_GlyphMap;
    
    // Métodos internos
    FontAtlas* FindAtlasWithSpace(int width, int height);
    FontAtlas* CreateNewAtlas();
    void RebalanceAtlases();
};

} // namespace Drift::UI 