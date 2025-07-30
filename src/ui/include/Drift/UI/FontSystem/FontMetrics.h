#pragma once

#include <string>
#include <vector>
#include <unordered_map>
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
 * @brief Sistema de Métricas de Fontes Profissional
 * 
 * Fornece cálculos precisos de layout de texto, incluindo:
 * - Quebra de linha inteligente
 * - Kerning e ligaduras
 * - Suporte a múltiplas direções de texto
 * - Alinhamento e justificação
 * - Cálculos de métricas tipográficas
 */
class FontMetrics {
public:
    /**
     * @brief Construtor
     */
    FontMetrics();
    
    /**
     * @brief Destrutor
     */
    ~FontMetrics();

    // Layout de texto
    TextLayoutResult CalculateLayout(const std::string& text, 
                                    const std::shared_ptr<class Font>& font,
                                    const TextLayoutConfig& config = {});
    
    TextLayoutResult CalculateLayout(const std::wstring& text,
                                    const std::shared_ptr<class Font>& font,
                                    const TextLayoutConfig& config = {});
    
    // Medidas de texto
    glm::vec2 MeasureText(const std::string& text, 
                          const std::shared_ptr<class Font>& font,
                          const TextLayoutConfig& config = {});
    
    float GetTextWidth(const std::string& text,
                       const std::shared_ptr<class Font>& font);
    
    float GetTextHeight(const std::string& text,
                        const std::shared_ptr<class Font>& font,
                        const TextLayoutConfig& config = {});
    
    // Quebra de linha
    std::vector<std::string> BreakTextIntoLines(const std::string& text,
                                                const std::shared_ptr<class Font>& font,
                                                float maxWidth,
                                                const TextLayoutConfig& config = {});
    
    // Kerning e ligaduras
    float GetKerning(uint32_t left, uint32_t right, 
                     const std::shared_ptr<class Font>& font);
    
    std::vector<uint32_t> ApplyLigatures(const std::vector<uint32_t>& codepoints,
                                         const std::shared_ptr<class Font>& font);
    
    // Utilitários
    bool IsWhitespace(uint32_t codepoint) const;
    bool IsLineBreak(uint32_t codepoint) const;
    bool IsWordBreak(uint32_t codepoint) const;
    std::vector<uint32_t> DecodeUTF8(const std::string& text) const;
    std::string EncodeUTF8(const std::vector<uint32_t>& codepoints) const;

private:
    // Cache de layouts para otimização
    struct LayoutCacheEntry {
        TextLayoutResult result;
        std::chrono::steady_clock::time_point lastUsed;
        size_t accessCount;
    };
    
    struct LayoutCacheKey {
        std::string text;
        std::string fontName;
        float fontSize;
        TextLayoutConfig config;
        
        bool operator==(const LayoutCacheKey& other) const;
    };
    
    struct LayoutCacheKeyHash {
        size_t operator()(const LayoutCacheKey& key) const;
    };
    
    std::unordered_map<LayoutCacheKey, LayoutCacheEntry, LayoutCacheKeyHash> m_LayoutCache;
    
    // Métodos auxiliares
    TextLayoutResult CalculateLayoutInternal(const std::vector<uint32_t>& codepoints,
                                            const std::shared_ptr<class Font>& font,
                                            const TextLayoutConfig& config);
    
    void ApplyKerning(std::vector<TextCharInfo>& chars,
                      const std::shared_ptr<class Font>& font);
    
    void ApplyLigatures(std::vector<TextCharInfo>& chars,
                        const std::shared_ptr<class Font>& font);
    
    void AlignText(std::vector<TextLineInfo>& lines,
                   const TextLayoutConfig& config);
    
    void JustifyText(std::vector<TextLineInfo>& lines,
                     const TextLayoutConfig& config);
    
    float CalculateLineWidth(const std::vector<TextCharInfo>& chars);
    
    void TrimLayoutCache();
    
    // Tabelas de caracteres especiais
    static const std::unordered_set<uint32_t> s_WhitespaceChars;
    static const std::unordered_set<uint32_t> s_LineBreakChars;
    static const std::unordered_set<uint32_t> s_WordBreakChars;
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