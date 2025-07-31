#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Device.h"

namespace Drift::UI {

/**
 * @brief Tipos de renderização de glyphs
 */
enum class GlyphRenderType {
    Bitmap,         // Renderização bitmap tradicional
    SDF,            // Signed Distance Field
    MSDF,           // Multi-channel Signed Distance Field
    Vector          // Renderização vetorial (futuro)
};

/**
 * @brief Informações detalhadas de um glyph
 */
struct GlyphInfo {
    // Coordenadas no atlas
    glm::vec2 uv0{0.0f};              // UV superior esquerdo
    glm::vec2 uv1{0.0f};              // UV inferior direito
    
    // Dimensões e posicionamento
    glm::vec2 size{0.0f};             // Tamanho do glyph em pixels
    glm::vec2 bearing{0.0f};          // Offset em relação à baseline
    float advance{0.0f};              // Avanço horizontal
    
    // Métricas adicionais
    float leftBearing{0.0f};          // Bearing esquerdo
    float rightBearing{0.0f};         // Bearing direito
    float topBearing{0.0f};           // Bearing superior
    float bottomBearing{0.0f};        // Bearing inferior
    
    // Informações de renderização
    GlyphRenderType renderType{GlyphRenderType::MSDF};
    bool isLoaded{false};             // Se o glyph foi carregado
    bool isFallback{false};           // Se é um glyph de fallback
    
    // Cache de kerning
    std::unordered_map<uint32_t, float> kerningPairs;
};

/**
 * @brief Configuração do atlas de fontes
 */
struct FontAtlasConfig {
    int width = 1024;                 // Largura do atlas
    int height = 1024;                // Altura do atlas
    int padding = 2;                  // Padding entre glyphs
    int border = 1;                   // Borda ao redor de cada glyph
    GlyphRenderType renderType = GlyphRenderType::MSDF;
    bool enableMipmaps = false;       // Gera mipmaps
    bool enableCompression = true;    // Compressão de textura
    int msdfRange = 4;                // Range para MSDF
    float msdfScale = 1.0f;           // Escala para MSDF
};

/**
 * @brief Sistema de Atlas de Fontes Profissional
 * 
 * Gerencia a criação e otimização de atlas de glyphs para renderização
 * de texto de alta qualidade. Suporta múltiplos tipos de renderização
 * e otimizações de memória.
 */
class FontAtlas {
public:
    /**
     * @brief Construtor
     * @param device Dispositivo RHI
     * @param config Configuração do atlas
     */
    FontAtlas(Drift::RHI::IDevice* device, const FontAtlasConfig& config = {});
    
    /**
     * @brief Destrutor
     */
    ~FontAtlas();

    // Gerenciamento de glyphs
    bool AddGlyph(uint32_t codepoint, const std::vector<unsigned char>& bitmap, 
                  int width, int height, const GlyphInfo& info);
    
    bool AddGlyphSDF(uint32_t codepoint, const std::vector<float>& sdfData,
                     int width, int height, const GlyphInfo& info);
    
    bool AddGlyphMSDF(uint32_t codepoint, const std::vector<glm::vec3>& msdfData,
                      int width, int height, const GlyphInfo& info);
    
    const GlyphInfo* GetGlyph(uint32_t codepoint) const;
    bool HasGlyph(uint32_t codepoint) const;
    
    // Gerenciamento do atlas
    void Clear();
    void Rebuild();
    bool IsFull() const;
    float GetUsagePercentage() const;
    
    // Acesso à textura
    std::shared_ptr<Drift::RHI::ITexture> GetTexture() const { return m_Texture; }
    const FontAtlasConfig& GetConfig() const { return m_Config; }
    
    // Estatísticas
    size_t GetGlyphCount() const { return m_Glyphs.size(); }
    size_t GetMemoryUsage() const;
    
    // Otimizações
    void OptimizeLayout();
    void Defragment();

private:
    // Estrutura de nó para packing binário
    struct AtlasNode {
        int x, y, width, height;
        bool used;
        std::unique_ptr<AtlasNode> left, right;
        
        AtlasNode(int x, int y, int w, int h) 
            : x(x), y(y), width(w), height(h), used(false) {}
    };
    
    // Estrutura de região no atlas
    struct AtlasRegion {
        int x, y, width, height;
        uint32_t codepoint;
        
        AtlasRegion(int x, int y, int w, int h, uint32_t cp)
            : x(x), y(y), width(w), height(h), codepoint(cp) {}
    };

    Drift::RHI::IDevice* m_Device;
    FontAtlasConfig m_Config;
    std::shared_ptr<Drift::RHI::ITexture> m_Texture;
    
    // Dados do atlas
    std::vector<unsigned char> m_AtlasData;
    std::unordered_map<uint32_t, GlyphInfo> m_Glyphs;
    std::vector<AtlasRegion> m_Regions;
    
    // Árvore de packing
    std::unique_ptr<AtlasNode> m_Root;
    
    // Métodos auxiliares
    bool AllocateSpace(int width, int height, int& x, int& y);
    AtlasNode* FindNode(AtlasNode* node, int width, int height);
    AtlasNode* SplitNode(AtlasNode* node, int width, int height);
    void UpdateTexture();
    void ClearNode(AtlasNode* node);
    
    // Otimizações
    void SortRegionsByHeight();
    void CompactRegions();
    bool CanFitRegion(const AtlasRegion& region, int x, int y) const;
};

/**
 * @brief Gerenciador de múltiplos atlas
 * 
 * Gerencia múltiplos atlas para diferentes tamanhos de fonte
 * e qualidades de renderização.
 */
class FontAtlasManager {
public:
    static FontAtlasManager& GetInstance();
    
    // Criação e gerenciamento de atlas
    std::shared_ptr<FontAtlas> CreateAtlas(const FontAtlasConfig& config);
    std::shared_ptr<FontAtlas> GetAtlas(const FontAtlasConfig& config);
    void DestroyAtlas(std::shared_ptr<FontAtlas> atlas);
    
    // Otimizações globais
    void OptimizeAllAtlas();
    void ClearUnusedAtlas();
    
    // Estatísticas
    size_t GetAtlasCount() const;
    size_t GetTotalMemoryUsage() const;
    
    // Configuração
    void SetDevice(Drift::RHI::IDevice* device) { m_Device = device; }

private:
    FontAtlasManager() = default;
    
    Drift::RHI::IDevice* m_Device{nullptr};
    std::vector<std::shared_ptr<FontAtlas>> m_Atlas;
    
    // Função de hash para FontAtlasConfig
    struct AtlasConfigHash {
        size_t operator()(const FontAtlasConfig& config) const {
            return std::hash<int>{}(config.width) ^ 
                   (std::hash<int>{}(config.height) << 1) ^
                   (std::hash<int>{}(static_cast<int>(config.renderType)) << 2);
        }
    };
    
    // Função de igualdade para FontAtlasConfig
    struct AtlasConfigEqual {
        bool operator()(const FontAtlasConfig& a, const FontAtlasConfig& b) const {
            return a.width == b.width && 
                   a.height == b.height && 
                   a.renderType == b.renderType &&
                   a.padding == b.padding &&
                   a.border == b.border;
        }
    };
    
    std::unordered_map<FontAtlasConfig, std::shared_ptr<FontAtlas>, 
                       AtlasConfigHash, AtlasConfigEqual> m_AtlasCache;
};

} // namespace Drift::UI 