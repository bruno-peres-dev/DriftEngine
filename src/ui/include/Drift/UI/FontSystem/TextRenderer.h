#pragma once

#include "FontManager.h"
#include "Drift/RHI/UIBatcher.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <unordered_map>

namespace Drift::UI {

/**
 * @brief Configuração de renderização de texto
 */
struct TextRenderConfig {
    bool enableKerning = true;       ///< Habilita kerning entre caracteres
    bool enableSubpixelRendering = true; ///< Habilita renderização subpixel
    bool enableTextCache = true;     ///< Habilita cache de medidas de texto
    size_t maxCachedStrings = 1000;  ///< Número máximo de strings em cache
};

/**
 * @brief Renderizador de texto otimizado
 * 
 * Esta classe gerencia a renderização de texto na tela, incluindo
 * cache de medidas, otimizações de renderização e suporte a diferentes fontes.
 */
class TextRenderer {
public:
    /**
     * @brief Construtor padrão
     */
    TextRenderer();
    
    /**
     * @brief Destrutor
     */
    ~TextRenderer();

    // Configuração
    void SetBatcher(Drift::RHI::IUIBatcher* batcher) { m_Batcher = batcher; }
    void SetScreenSize(int w, int h) { m_ScreenWidth = w; m_ScreenHeight = h; }
    void SetConfig(const TextRenderConfig& config) { m_Config = config; }

    // Renderização
    void BeginTextRendering();
    void EndTextRendering();

    /**
     * @brief Adiciona texto para renderização
     * @param text Texto a ser renderizado
     * @param pos Posição na tela
     * @param fontName Nome da fonte
     * @param fontSize Tamanho da fonte
     * @param color Cor do texto
     */
    void AddText(const std::string& text, const glm::vec2& pos,
                 const std::string& fontName, float fontSize, const glm::vec4& color);

    /**
     * @brief Mede o tamanho de um texto
     * @param text Texto a ser medido
     * @param fontName Nome da fonte
     * @param size Tamanho da fonte
     * @return Dimensões do texto (largura, altura)
     */
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName, float size);

    // Otimizações
    void ClearTextCache();
    size_t GetTextCacheSize() const;

private:
    // Função utilitária para decodificar UTF-8
    static std::vector<uint32_t> DecodeUTF8(const std::string& utf8_string);

    Drift::RHI::IUIBatcher* m_Batcher{nullptr};
    int m_ScreenWidth{0};
    int m_ScreenHeight{0};
    TextRenderConfig m_Config;
    
    // Cache de medidas de texto para otimização
    struct TextMeasureCache {
        glm::vec2 size;
        size_t lastUsed = 0;
    };
    
    struct TextCacheKey {
        std::string text;
        std::string fontName;
        float fontSize;
        
        bool operator==(const TextCacheKey& other) const {
            return text == other.text && fontName == other.fontName && fontSize == other.fontSize;
        }
    };
    
    struct TextCacheKeyHash {
        size_t operator()(const TextCacheKey& k) const noexcept {
            return std::hash<std::string>{}(k.text) ^ 
                   (std::hash<std::string>{}(k.fontName) << 1) ^ 
                   (std::hash<int>{}(static_cast<int>(k.fontSize * 10)) << 2);
        }
    };
    
    std::unordered_map<TextCacheKey, TextMeasureCache, TextCacheKeyHash> m_TextCache;
    
    /**
     * @brief Renderiza um caractere individual
     * @param c Caractere a ser renderizado
     * @param font Fonte a ser usada
     * @param x Posição X atual
     * @param baseline Linha base Y
     * @param color Cor do texto
     * @return Avanço horizontal para o próximo caractere
     */
    float RenderGlyph(uint32_t codepoint, const std::shared_ptr<Font>& font, float x, float baseline, const glm::vec4& color);
    
    /**
     * @brief Obtém a medida de texto do cache ou calcula
     * @param text Texto a ser medido
     * @param fontName Nome da fonte
     * @param size Tamanho da fonte
     * @return Dimensões do texto
     */
    glm::vec2 GetCachedTextMeasure(const std::string& text, const std::string& fontName, float size);
    
    /**
     * @brief Calcula a medida de texto sem cache
     * @param text Texto a ser medido
     * @param font Fonte a ser usada
     * @return Dimensões do texto
     */
    glm::vec2 CalculateTextMeasure(const std::string& text, const std::shared_ptr<Font>& font);
    
    /**
     * @brief Limpa o cache de texto se necessário
     */
    void TrimTextCache();
};

/**
 * @brief Renderizador de texto especializado para UIBatcher
 */
class UIBatcherTextRenderer : public TextRenderer {
public:
    /**
     * @brief Construtor com batcher específico
     * @param batcher Batcher UI a ser usado
     */
    explicit UIBatcherTextRenderer(Drift::RHI::IUIBatcher* batcher) {
        SetBatcher(batcher);
    }
};

} // namespace Drift::UI
