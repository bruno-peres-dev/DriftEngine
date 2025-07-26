#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <glm/glm.hpp>
#include "stb_truetype.h"
#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/RHI/Device.h"

namespace Drift::UI {

/**
 * @brief Estrutura otimizada para representar um glyph individual
 * 
 * Esta estrutura contém todas as informações necessárias para renderizar
 * um caractere específico de uma fonte, incluindo posição no atlas,
 * métricas de renderização e coordenadas UV.
 */
struct Glyph {
    uint32_t codepoint;      ///< Código Unicode do caractere
    glm::vec2 position;      ///< Posição no atlas de textura
    glm::vec2 size;          ///< Tamanho do glyph em pixels
    glm::vec2 offset;        ///< Offset para posicionamento correto
    float advance;           ///< Avanço horizontal para próximo caractere
    glm::vec2 uvMin;         ///< Coordenadas UV mínimas no atlas
    glm::vec2 uvMax;         ///< Coordenadas UV máximas no atlas
    bool isValid;            ///< Indica se o glyph é válido
    uint32_t atlasId;        ///< ID do atlas que contém este glyph
    
    Glyph() : codepoint(0), position(0), size(0), offset(0), advance(0), 
              uvMin(0), uvMax(0), isValid(false), atlasId(0) {}
};

/**
 * @brief Níveis de qualidade de renderização de fonte
 * 
 * Define diferentes níveis de qualidade para renderização de texto,
 * afetando a resolução do MSDF e a qualidade visual.
 */
enum class FontQuality {
    Low = 0,        ///< 8x MSDF, sem suavização adicional (performance máxima)
    Medium = 1,     ///< 16x MSDF, suavização básica (equilibrio)
    High = 2,       ///< 32x MSDF, suavização avançada (qualidade padrão)
    Ultra = 3       ///< 64x MSDF, suavização máxima + subpixel (qualidade máxima)
};

/**
 * @brief Configurações avançadas para renderização de texto
 * 
 * Contém todas as configurações que afetam a qualidade e aparência
 * do texto renderizado, incluindo anti-aliasing, kerning e efeitos.
 */
struct TextRenderSettings {
    FontQuality quality = FontQuality::High;    ///< Qualidade da fonte
    bool enableSubpixel = true;                 ///< Habilitar renderização subpixel
    bool enableLigatures = true;                ///< Habilitar ligaduras
    bool enableKerning = true;                  ///< Habilitar kerning
    bool enableHinting = true;                  ///< Habilitar hinting
    float gamma = 2.2f;                         ///< Correção gamma
    float contrast = 0.1f;                      ///< Contraste adicional
    float smoothing = 0.1f;                     ///< Suavização
    float outlineWidth = 0.0f;                  ///< Largura do contorno
    glm::vec4 outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); ///< Cor do contorno
    
    bool operator==(const TextRenderSettings& other) const {
        return quality == other.quality &&
               enableSubpixel == other.enableSubpixel &&
               enableLigatures == other.enableLigatures &&
               enableKerning == other.enableKerning &&
               enableHinting == other.enableHinting &&
               gamma == other.gamma &&
               contrast == other.contrast &&
               smoothing == other.smoothing &&
               outlineWidth == other.outlineWidth &&
               outlineColor == other.outlineColor;
    }
};

/**
 * @brief Métricas de fonte otimizadas
 * 
 * Contém informações sobre as dimensões e características
 * de uma fonte, usadas para posicionamento e layout de texto.
 */
struct FontMetrics {
    float ascender;              ///< Altura do ascender
    float descender;             ///< Profundidade do descender
    float lineHeight;            ///< Altura da linha
    float xHeight;               ///< Altura do 'x' minúsculo
    float capHeight;             ///< Altura das maiúsculas
    float underlinePosition;     ///< Posição do sublinhado
    float underlineThickness;    ///< Espessura do sublinhado
    float maxAdvance;            ///< Avanço máximo
    glm::vec2 boundingBox;       ///< Caixa delimitadora
    
    FontMetrics() : ascender(0), descender(0), lineHeight(0), xHeight(0),
                   capHeight(0), underlinePosition(0), underlineThickness(0),
                   maxAdvance(0), boundingBox(0) {}
};

/**
 * @brief Configurações de cache de fonte
 * 
 * Define limites e comportamentos do sistema de cache
 * para otimizar uso de memória e performance.
 */
struct FontCacheConfig {
    size_t maxFonts = 64;           ///< Máximo de fontes em cache
    size_t maxGlyphsPerFont = 4096; ///< Máximo de glyphs por fonte
    size_t maxAtlasSize = 4096;     ///< Tamanho máximo do atlas
    bool enablePreloading = true;   ///< Habilitar pré-carregamento
    bool enableLazyLoading = true;  ///< Habilitar carregamento lazy
    float memoryBudgetMB = 256.0f;  ///< Orçamento de memória em MB
};

/**
 * @brief Estatísticas de uso do sistema de fontes
 * 
 * Fornece informações sobre o uso atual do sistema,
 * útil para debugging e otimização.
 */
struct FontStats {
    size_t totalFonts = 0;          ///< Total de fontes carregadas
    size_t totalGlyphs = 0;         ///< Total de glyphs carregados
    size_t totalAtlases = 0;        ///< Total de atlases criados
    size_t memoryUsageBytes = 0;    ///< Uso de memória em bytes
    size_t cacheHits = 0;           ///< Acertos no cache
    size_t cacheMisses = 0;         ///< Falhas no cache
    float cacheHitRate = 0.0f;      ///< Taxa de acerto do cache
    
    void Reset() {
        totalFonts = 0;
        totalGlyphs = 0;
        totalAtlases = 0;
        memoryUsageBytes = 0;
        cacheHits = 0;
        cacheMisses = 0;
        cacheHitRate = 0.0f;
    }
};

/**
 * @brief Classe para representar uma fonte carregada
 * 
 * Gerencia o carregamento, cache e acesso aos glyphs de uma fonte específica.
 * Implementa lazy loading e otimizações de memória para performance AAA.
 */
class Font {
public:
    /**
     * @brief Construtor da fonte
     * @param name Nome da fonte
     * @param filePath Caminho para o arquivo da fonte
     * @param size Tamanho da fonte em pixels
     * @param quality Qualidade de renderização
     */
    Font(const std::string& name, const std::string& filePath, float size, FontQuality quality);
    
    /**
     * @brief Destrutor
     */
    ~Font();

    /**
     * @brief Carrega a fonte do arquivo com device específico
     * @param device Device para criar texturas
     * @return true se carregado com sucesso
     */
    bool Load(Drift::RHI::IDevice* device);
    
    /**
     * @brief Descarrega a fonte da memória
     */
    void Unload();
    
    /**
     * @brief Verifica se a fonte está carregada
     * @return true se carregada
     */
    bool IsLoaded() const;

    // === Métodos de acesso a glyphs ===
    
    /**
     * @brief Obtém um glyph específico
     * @param character Código Unicode do caractere
     * @return Ponteiro para o glyph ou nullptr se não encontrado
     */
    const Glyph* GetGlyph(uint32_t character) const;
    
    /**
     * @brief Verifica se um glyph existe
     * @param character Código Unicode do caractere
     * @return true se o glyph existe
     */
    bool HasGlyph(uint32_t character) const;
    
    /**
     * @brief Pré-carrega glyphs específicos
     * @param characters Lista de códigos Unicode para pré-carregar
     */
    void PreloadGlyphs(const std::vector<uint32_t>& characters);
    
    // === Métricas e medidas ===
    
    /**
     * @brief Obtém o kerning entre dois caracteres
     * @param left Caractere à esquerda
     * @param right Caractere à direita
     * @return Valor do kerning em pixels
     */
    float GetKerning(uint32_t left, uint32_t right) const;
    
    /**
     * @brief Mede o tamanho de um texto
     * @param text Texto a ser medido
     * @return Dimensões do texto (largura, altura)
     */
    glm::vec2 MeasureText(const std::string& text) const;
    
    /**
     * @brief Mede o tamanho de um texto wide
     * @param text Texto wide a ser medido
     * @return Dimensões do texto (largura, altura)
     */
    glm::vec2 MeasureText(const std::wstring& text) const;
    
    /**
     * @brief Obtém posições de glyphs para um texto
     * @param text Texto para calcular posições
     * @param x Posição X inicial
     * @param y Posição Y inicial
     * @return Lista de posições dos glyphs
     */
    std::vector<glm::vec2> GetGlyphPositions(const std::string& text, float x, float y) const;
    
    // === Propriedades da fonte ===
    
    /**
     * @brief Obtém a altura da linha
     * @return Altura da linha em pixels
     */
    float GetLineHeight() const;
    
    /**
     * @brief Obtém o ascender
     * @return Valor do ascender em pixels
     */
    float GetAscender() const;
    
    /**
     * @brief Obtém o descender
     * @return Valor do descender em pixels
     */
    float GetDescender() const;
    
    /**
     * @brief Obtém as métricas da fonte
     * @return Referência para as métricas
     */
    const FontMetrics& GetMetrics() const { return m_Metrics; }
    
    /**
     * @brief Obtém o nome da fonte
     * @return Nome da fonte
     */
    const std::string& GetName() const;
    
    /**
     * @brief Obtém o tamanho da fonte
     * @return Tamanho em pixels
     */
    float GetSize() const;
    
    /**
     * @brief Obtém a qualidade da fonte
     * @return Qualidade de renderização
     */
    FontQuality GetQuality() const;
    
    /**
     * @brief Obtém o caminho do arquivo
     * @return Caminho do arquivo da fonte
     */
    const std::string& GetFilePath() const;
    
    // === Atlas e texturas ===
    
    /**
     * @brief Obtém o atlas da fonte
     * @return Referência para o atlas
     */
    const std::unique_ptr<class FontAtlas>& GetAtlas() const;
    
    /**
     * @brief Obtém o ID da textura do atlas
     * @return ID da textura ou 0 se não disponível
     */
    uint32_t GetAtlasTextureId() const;
    
    // === Cache e otimizações ===
    
    /**
     * @brief Marca a fonte como usada recentemente
     */
    void Touch();
    
    /**
     * @brief Obtém o timestamp do último uso
     * @return Timestamp do último uso
     */
    size_t GetLastUsed() const { return m_LastUsed; }
    
    /**
     * @brief Calcula o uso de memória da fonte
     * @return Uso de memória em bytes
     */
    size_t GetMemoryUsage() const;

    // === Métodos privados para carregamento ===
    
    /**
     * @brief Carrega um glyph específico
     * @param character Código Unicode do caractere
     */
    void LoadGlyph(uint32_t character);
    
    /**
     * @brief Carrega glyphs básicos (ASCII)
     */
    void LoadBasicGlyphs();
    
    /**
     * @brief Calcula métricas da fonte
     */
    void CalculateMetrics();

private:
    std::string m_Name;                    ///< Nome da fonte
    std::string m_FilePath;                ///< Caminho do arquivo
    float m_Size;                          ///< Tamanho da fonte
    FontQuality m_Quality;                 ///< Qualidade de renderização
    std::atomic<bool> m_IsLoaded{false};   ///< Estado de carregamento
    
    FontMetrics m_Metrics;                 ///< Métricas da fonte
    std::unordered_map<uint32_t, Glyph> m_Glyphs; ///< Cache de glyphs
    std::unordered_map<uint64_t, float> m_Kerning; ///< Cache de kerning
    std::unique_ptr<class FontAtlas> m_Atlas; ///< Atlas de textura
    
    // Dados da fonte TTF
    std::vector<unsigned char> m_TTFBuffer; ///< Buffer com dados TTF
    stbtt_fontinfo m_FontInfo{};           ///< Informações da fonte STB
    
    // Thread safety e cache
    mutable std::mutex m_GlyphMutex;       ///< Mutex para acesso thread-safe aos glyphs
    size_t m_LastUsed{0};                  ///< Timestamp do último uso
    float m_Scale{1.0f};                   ///< Escala da fonte
    
    // Permitir que FontManager acesse membros privados
    friend class FontManager;
};

/**
 * @brief Gerenciador principal de fontes otimizado para AAA
 * 
 * Implementa um sistema completo de gerenciamento de fontes com:
 * - Cache LRU otimizado
 * - Carregamento lazy e pré-carregamento
 * - Gerenciamento de memória inteligente
 * - Suporte a múltiplas qualidades
 * - Thread safety
 * - Estatísticas detalhadas
 */
class FontManager {
public:
    /**
     * @brief Obtém a instância singleton do FontManager
     * @return Referência para a instância
     */
    static FontManager& GetInstance();
    
    // === Gerenciamento de fontes ===
    
    /**
     * @brief Carrega uma fonte do arquivo
     * @param name Nome da fonte
     * @param filePath Caminho para o arquivo
     * @param size Tamanho da fonte
     * @param quality Qualidade de renderização
     * @return Ponteiro compartilhado para a fonte
     */
    std::shared_ptr<Font> LoadFont(const std::string& name, const std::string& filePath, 
                                  float size, FontQuality quality = FontQuality::High);
    
    /**
     * @brief Obtém uma fonte do cache
     * @param name Nome da fonte
     * @param size Tamanho da fonte
     * @param quality Qualidade de renderização
     * @return Ponteiro compartilhado para a fonte
     */
    std::shared_ptr<Font> GetFont(const std::string& name, float size, 
                                 FontQuality quality = FontQuality::High);
    
    /**
     * @brief Descarrega uma fonte específica
     * @param name Nome da fonte
     * @param size Tamanho da fonte
     * @param quality Qualidade de renderização
     */
    void UnloadFont(const std::string& name, float size, FontQuality quality = FontQuality::High);
    
    /**
     * @brief Descarrega todas as fontes
     */
    void UnloadAllFonts();
    
    // === Configurações ===
    
    /**
     * @brief Define a qualidade padrão
     * @param quality Qualidade padrão
     */
    void SetDefaultQuality(FontQuality quality);
    
    /**
     * @brief Define o tamanho padrão
     * @param size Tamanho padrão
     */
    void SetDefaultSize(float size);
    
    /**
     * @brief Define o nome da fonte padrão
     * @param name Nome da fonte padrão
     */
    void SetDefaultFontName(const std::string& name);
    
    /**
     * @brief Define configurações de cache
     * @param config Configurações de cache
     */
    void SetCacheConfig(const FontCacheConfig& config);
    
    /**
     * @brief Obtém configurações de cache
     * @return Configurações atuais
     */
    FontCacheConfig GetCacheConfig() const { return m_CacheConfig; }
    
    // === Pré-carregamento e otimizações ===
    
    /**
     * @brief Pré-carrega uma fonte em múltiplos tamanhos
     * @param name Nome da fonte
     * @param filePath Caminho para o arquivo
     * @param sizes Lista de tamanhos para carregar
     * @param quality Qualidade de renderização
     */
    void PreloadFont(const std::string& name, const std::string& filePath, 
                    const std::vector<float>& sizes, FontQuality quality = FontQuality::High);
    
    /**
     * @brief Pré-carrega caracteres específicos
     * @param fontName Nome da fonte
     * @param characters Lista de códigos Unicode
     * @param size Tamanho da fonte
     * @param quality Qualidade de renderização
     */
    void PreloadCharacters(const std::string& fontName, const std::vector<uint32_t>& characters,
                          float size = 16.0f, FontQuality quality = FontQuality::High);
    
    // === Fonte padrão embutida ===
    
    /**
     * @brief Cria uma fonte padrão embutida
     * @param size Tamanho da fonte
     * @param quality Qualidade de renderização
     * @return Ponteiro compartilhado para a fonte embutida
     */
    std::shared_ptr<Font> CreateEmbeddedDefaultFont(float size, FontQuality quality = FontQuality::High);
    
    // === Ciclo de renderização ===
    
    /**
     * @brief Inicia o ciclo de renderização
     */
    void BeginTextRendering();
    
    /**
     * @brief Finaliza o ciclo de renderização
     */
    void EndTextRendering();
    
    // === Batching e uploads ===
    
    /**
     * @brief Força o flush de todos os uploads pendentes
     */
    void FlushAllPendingUploads();
    
    /**
     * @brief Verifica se há uploads pendentes
     * @return true se há uploads pendentes
     */
    bool HasPendingUploads() const;
    
    // === Cache e memória ===
    
    /**
     * @brief Atualiza o cache (remove fontes não utilizadas)
     */
    void UpdateCache();
    
    /**
     * @brief Limpa todo o cache
     */
    void ClearCache();
    
    /**
     * @brief Remove fontes antigas do cache
     */
    void TrimCache();
    
    /**
     * @brief Obtém estatísticas do sistema
     * @return Estatísticas atuais
     */
    FontStats GetStats() const;
    
    /**
     * @brief Reseta as estatísticas
     */
    void ResetStats();
    
    // === Métodos utilitários ===
    
    /**
     * @brief Obtém o número de fontes carregadas
     * @return Número de fontes
     */
    size_t GetLoadedFontCount() const;
    
    /**
     * @brief Obtém nomes das fontes carregadas
     * @return Lista de nomes de fontes
     */
    std::vector<std::string> GetLoadedFontNames() const;
    
    /**
     * @brief Verifica se uma fonte está carregada
     * @param name Nome da fonte
     * @param size Tamanho da fonte
     * @param quality Qualidade de renderização
     * @return true se carregada
     */
    bool IsFontLoaded(const std::string& name, float size, FontQuality quality = FontQuality::High) const;
    
    // === Otimizações avançadas ===
    
    /**
     * @brief Define orçamento de memória
     * @param budgetMB Orçamento em MB
     */
    void SetMemoryBudget(float budgetMB);
    
    /**
     * @brief Habilita/desabilita carregamento assíncrono
     * @param enabled true para habilitar
     */
    void EnableAsyncLoading(bool enabled);
    
    /**
     * @brief Define número de threads de trabalho
     * @param count Número de threads
     */
    void SetWorkerThreadCount(size_t count);
    
    /**
     * @brief Define o device para criação de texturas
     * @param device Ponteiro para o device RHI
     */
    void SetDevice(Drift::RHI::IDevice* device);

private:
    FontManager();
    ~FontManager();
    
    /**
     * @brief Chave única para identificar uma fonte no cache
     */
    struct FontKey {
        std::string name;
        float size;
        FontQuality quality;
        
        bool operator==(const FontKey& other) const {
            return name == other.name && size == other.size && quality == other.quality;
        }
    };
    
    /**
     * @brief Hash function para FontKey
     */
    struct FontKeyHash {
        size_t operator()(const FontKey& key) const {
            return std::hash<std::string>{}(key.name) ^ 
                   std::hash<float>{}(key.size) ^ 
                   std::hash<int>{}(static_cast<int>(key.quality));
        }
    };
    
    // Cache de fontes com LRU
    std::unordered_map<FontKey, std::shared_ptr<Font>, FontKeyHash> m_Fonts;
    std::queue<FontKey> m_FontUsageQueue;
    
    // Configurações
    FontQuality m_DefaultQuality;
    float m_DefaultSize;
    std::string m_DefaultFontName;
    FontCacheConfig m_CacheConfig;
    FontStats m_Stats;
    
    // Estado de renderização
    bool m_IsRendering;
    size_t m_FrameCounter;
    
    // Threading e sincronização
    mutable std::mutex m_FontMutex;
    std::atomic<bool> m_AsyncLoadingEnabled{false};
    size_t m_WorkerThreadCount{4};
    
    // Device para criação de texturas
    Drift::RHI::IDevice* m_Device{nullptr};
    
    // Sistema de batching
    std::unique_ptr<MultiAtlasManager> m_AtlasManager;
    
    // Métodos internos
    void UpdateFontUsage(const FontKey& key);
    size_t CalculateFontMemoryUsage(const std::shared_ptr<Font>& font) const;
    void UpdateStats();
};

// === Utilitários para renderização de texto ===

/**
 * @brief Namespace com utilitários para renderização de texto
 */
namespace TextUtils {
    /**
     * @brief Mede o tamanho de um texto
     * @param text Texto a medir
     * @param fontName Nome da fonte
     * @param size Tamanho da fonte
     * @param quality Qualidade da fonte
     * @return Dimensões do texto
     */
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName = "default", 
                         float size = 16.0f, FontQuality quality = FontQuality::High);
    
    /**
     * @brief Quebra texto em linhas
     * @param text Texto para quebrar
     * @param maxWidth Largura máxima
     * @param fontName Nome da fonte
     * @param size Tamanho da fonte
     * @param quality Qualidade da fonte
     * @return Lista de linhas
     */
    std::vector<std::string> WordWrap(const std::string& text, float maxWidth, 
                                     const std::string& fontName = "default", 
                                     float size = 16.0f, FontQuality quality = FontQuality::High);
    
    /**
     * @brief Trunca texto para caber em uma largura
     * @param text Texto para truncar
     * @param maxWidth Largura máxima
     * @param fontName Nome da fonte
     * @param size Tamanho da fonte
     * @param quality Qualidade da fonte
     * @return Texto truncado
     */
    std::string TruncateText(const std::string& text, float maxWidth, 
                            const std::string& fontName = "default", 
                            float size = 16.0f, FontQuality quality = FontQuality::High);
    
    /**
     * @brief Converte string para códigos Unicode
     * @param text Texto para converter
     * @return Lista de códigos Unicode
     */
    std::vector<uint32_t> StringToCodepoints(const std::string& text);
    
    /**
     * @brief Converte códigos Unicode para string
     * @param codepoints Lista de códigos Unicode
     * @return String resultante
     */
    std::string CodepointsToString(const std::vector<uint32_t>& codepoints);
}

} // namespace Drift::UI 