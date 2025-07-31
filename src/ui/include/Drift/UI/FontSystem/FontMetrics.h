#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace Drift::UI {

/**
 * @brief Direção de escrita do texto
 */
enum class TextDirection {
    LeftToRight,    // LTR (inglês, português, etc.)
    RightToLeft,    // RTL (árabe, hebraico, etc.)
    TopToBottom,    // TTB (chinês tradicional, japonês)
    BottomToTop     // BTT (raro)
};

/**
 * @brief Alinhamento horizontal do texto
 */
enum class TextAlign {
    Left,
    Center,
    Right,
    Justify
};

/**
 * @brief Alinhamento vertical do texto
 */
enum class TextVerticalAlign {
    Top,
    Middle,
    Bottom,
    Baseline
};

/**
 * @brief Configuração de layout de texto
 */
struct TextLayoutConfig {
    TextDirection direction = TextDirection::LeftToRight;
    TextAlign horizontalAlign = TextAlign::Left;
    TextVerticalAlign verticalAlign = TextVerticalAlign::Baseline;
    
    // Margens e espaçamento
    float lineSpacing = 1.2f;         // Espaçamento entre linhas (múltiplo da altura da linha)
    float wordSpacing = 0.0f;         // Espaçamento adicional entre palavras
    float letterSpacing = 0.0f;       // Espaçamento adicional entre letras
    float paragraphSpacing = 1.5f;    // Espaçamento entre parágrafos
    
    // Quebra de linha
    float maxWidth = 0.0f;            // Largura máxima (0 = sem limite)
    bool enableWordWrap = true;       // Quebra de linha por palavras
    bool enableHyphenation = false;   // Hifenização automática
    
    // Renderização
    bool enableKerning = true;        // Kerning entre caracteres
    bool enableLigatures = true;      // Ligaduras tipográficas
    bool enableSubpixelRendering = true; // Renderização subpixel
    
    // Efeitos
    float outlineWidth = 0.0f;        // Largura do outline
    glm::vec4 outlineColor{0.0f};     // Cor do outline
    glm::vec2 shadowOffset{0.0f};     // Offset da sombra
    glm::vec4 shadowColor{0.0f, 0.0f, 0.0f, 0.5f}; // Cor da sombra
    
    bool operator==(const TextLayoutConfig& other) const {
        return direction == other.direction &&
               horizontalAlign == other.horizontalAlign &&
               verticalAlign == other.verticalAlign &&
               lineSpacing == other.lineSpacing &&
               wordSpacing == other.wordSpacing &&
               letterSpacing == other.letterSpacing &&
               paragraphSpacing == other.paragraphSpacing &&
               maxWidth == other.maxWidth &&
               enableWordWrap == other.enableWordWrap &&
               enableHyphenation == other.enableHyphenation &&
               enableKerning == other.enableKerning &&
               enableLigatures == other.enableLigatures &&
               enableSubpixelRendering == other.enableSubpixelRendering &&
               outlineWidth == other.outlineWidth &&
               outlineColor == other.outlineColor &&
               shadowOffset == other.shadowOffset &&
               shadowColor == other.shadowColor;
    }
};

/**
 * @brief Informações de uma linha de texto
 */
struct TextLineInfo {
    std::string text;                 // Texto da linha
    size_t startIndex;                // Índice inicial no texto original
    size_t endIndex;                  // Índice final no texto original
    glm::vec2 size;                   // Tamanho da linha
    glm::vec2 position;               // Posição da linha
    float baseline;                   // Posição da baseline
    std::vector<size_t> wordBreaks;   // Pontos de quebra de palavra
    bool isLastLine;                  // Se é a última linha
};

/**
 * @brief Informações de um caractere no texto
 */
struct TextCharInfo {
    uint32_t codepoint;               // Código Unicode
    glm::vec2 position;               // Posição do caractere
    glm::vec2 size;                   // Tamanho do caractere
    glm::vec2 uv0, uv1;               // Coordenadas UV no atlas
    float advance;                    // Avanço horizontal
    bool isWhitespace;                // Se é espaço em branco
    bool isLineBreak;                 // Se quebra linha
    size_t lineIndex;                 // Índice da linha
    size_t charIndex;                 // Índice na linha
};

/**
 * @brief Resultado do layout de texto
 */
struct TextLayoutResult {
    std::vector<TextLineInfo> lines;  // Linhas do texto
    std::vector<TextCharInfo> chars;  // Caracteres individuais
    glm::vec2 totalSize;              // Tamanho total do texto
    size_t lineCount;                 // Número de linhas
    size_t charCount;                 // Número de caracteres
    float maxLineWidth;               // Largura da linha mais larga
    float totalHeight;                // Altura total
    bool wasTruncated;                // Se o texto foi truncado
};

/**
 * @brief Métricas básicas de uma fonte
 */
struct FontMetrics {
    // Métricas verticais
    float ascent = 0.0f;              // Altura acima da baseline
    float descent = 0.0f;             // Profundidade abaixo da baseline
    float lineGap = 0.0f;             // Espaçamento entre linhas
    float lineHeight = 0.0f;          // Altura total da linha (ascent - descent + lineGap)
    
    // Métricas de altura específicas
    float xHeight = 0.0f;             // Altura da letra 'x' minúscula
    float capHeight = 0.0f;           // Altura das letras maiúsculas
    
    // Métricas de largura
    float avgCharWidth = 0.0f;        // Largura média dos caracteres
    float maxCharWidth = 0.0f;        // Largura máxima dos caracteres
    float minCharWidth = 0.0f;        // Largura mínima dos caracteres
    
    // Métricas de sublinhado
    float underlinePosition = 0.0f;   // Posição do sublinhado
    float underlineThickness = 0.0f;  // Espessura do sublinhado
};

/**
 * @brief Utilitários para texto
 */
namespace TextUtils {
    
    /**
     * @brief Remove acentos de um texto
     */
    std::string RemoveAccents(const std::string& text);
    
    /**
     * @brief Converte texto para minúsculas
     */
    std::string ToLower(const std::string& text);
    
    /**
     * @brief Converte texto para maiúsculas
     */
    std::string ToUpper(const std::string& text);
    
    /**
     * @brief Capitaliza primeira letra de cada palavra
     */
    std::string ToTitleCase(const std::string& text);
    
    /**
     * @brief Trunca texto com elipses
     */
    std::string TruncateWithEllipsis(const std::string& text, 
                                     const std::shared_ptr<class Font>& font,
                                     float maxWidth);
    
    /**
     * @brief Formata número com separadores
     */
    std::string FormatNumber(int64_t number, const std::string& separator = ",");
    
    /**
     * @brief Formata número decimal
     */
    std::string FormatDecimal(double number, int precision = 2);
    
    /**
     * @brief Formata tempo em segundos
     */
    std::string FormatTime(float seconds);
    
    /**
     * @brief Formata tamanho em bytes
     */
    std::string FormatBytes(size_t bytes);
    
} // namespace TextUtils

} // namespace Drift::UI 