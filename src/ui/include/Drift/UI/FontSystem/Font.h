#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Device.h"

// Forward declaration para evitar incluir stb_truetype.h no header
struct stbtt_fontinfo;

namespace Drift::UI {

/**
 * @brief Qualidade de renderização da fonte
 */
enum class FontQuality { 
    Low,    ///< Qualidade baixa (atlas 256x256)
    Medium, ///< Qualidade média (atlas 512x512)
    High,   ///< Qualidade alta (atlas 1024x1024)
    Ultra   ///< Qualidade ultra (atlas 2048x2048)
};

/**
 * @brief Informações de um glyph individual
 */
struct GlyphInfo {
    glm::vec2 uv0{0.0f};      ///< Coordenadas UV do canto superior esquerdo
    glm::vec2 uv1{0.0f};      ///< Coordenadas UV do canto inferior direito
    glm::vec2 size{0.0f};     ///< Tamanho do glyph em pixels
    glm::vec2 bearing{0.0f};  ///< Offset do glyph em relação à baseline
    float advance{0.0f};      ///< Avanço horizontal para o próximo caractere
};

/**
 * @brief Representa uma fonte TTF carregada com seus glyphs
 * 
 * Esta classe gerencia o carregamento e cache de uma fonte TTF,
 * incluindo a geração do atlas de textura e informações de glyphs.
 */
class Font {
public:
    /**
     * @brief Construtor da fonte
     * @param name Nome da fonte
     * @param size Tamanho da fonte em pixels
     * @param quality Qualidade de renderização
     */
    Font(std::string name, float size, FontQuality quality);
    
    /**
     * @brief Carrega a fonte a partir de um arquivo TTF
     * @param path Caminho para o arquivo TTF
     * @param device Dispositivo RHI para criar texturas
     * @return true se o carregamento foi bem-sucedido
     */
    bool LoadFromFile(const std::string& path, Drift::RHI::IDevice* device);
    
    /**
     * @brief Carrega a fonte a partir de dados em memória
     * @param data Dados TTF em memória
     * @param size Tamanho dos dados
     * @param device Dispositivo RHI para criar texturas
     * @return true se o carregamento foi bem-sucedido
     */
    bool LoadFromMemory(const unsigned char* data, size_t size, Drift::RHI::IDevice* device);

    /**
     * @brief Obtém informações de um glyph específico
     * @param codepoint Código Unicode do caractere
     * @return Ponteiro para GlyphInfo ou nullptr se não encontrado
     */
    const GlyphInfo* GetGlyph(uint32_t codepoint) const;
    
    /**
     * @brief Obtém a textura do atlas de glyphs
     * @return Textura compartilhada do atlas
     */
    std::shared_ptr<Drift::RHI::ITexture> GetAtlasTexture() const { return m_Texture; }

    // Getters para propriedades da fonte
    float GetSize() const { return m_Size; }
    const std::string& GetName() const { return m_Name; }
    float GetAscent() const { return m_Ascent; }
    float GetDescent() const { return m_Descent; }
    FontQuality GetQuality() const { return m_Quality; }

private:
    std::string m_Name;
    float m_Size;
    FontQuality m_Quality;
    
    // Otimização: usar vector para glyphs comuns (ASCII 32-127)
    std::vector<GlyphInfo> m_GlyphsASCII;  // Para caracteres ASCII comuns
    std::unordered_map<uint32_t, GlyphInfo> m_GlyphsExtended;  // Para caracteres Unicode
    
    std::shared_ptr<Drift::RHI::ITexture> m_Texture;
    float m_Ascent{0.0f};
    float m_Descent{0.0f};
    
    /**
     * @brief Obtém o tamanho do atlas baseado na qualidade
     * @return Tamanho do atlas em pixels
     */
    int GetAtlasSize() const;
    
    /**
     * @brief Carrega glyphs Unicode adicionais
     * @param data Dados TTF
     * @param atlasSize Tamanho do atlas
     * @param bitmap Bitmap do atlas
     */
    void LoadUnicodeGlyphs(const unsigned char* data, int atlasSize, std::vector<unsigned char>& bitmap);
};

} // namespace Drift::UI
