#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <memory>

namespace Drift::UI {

// Estrutura para representar um contorno de glyph
struct ContourPoint {
    glm::vec2 position;
    bool isOnCurve;
    glm::vec2 controlPoint1; // Para curvas Bézier
    glm::vec2 controlPoint2; // Para curvas Bézier cúbicas
};

struct Contour {
    std::vector<ContourPoint> points;
    bool isClosed;
    int windingOrder;
};

// Configurações do MSDF
struct MSDFConfig {
    int width = 64;          // Largura do MSDF em pixels
    int height = 64;         // Altura do MSDF em pixels
    float scale = 1.0f;      // Escala do glyph
    float range = 4.0f;      // Range do campo de distância
    bool enableSubpixel = true; // Habilitar anti-aliasing subpixel
    bool enableSupersampling = true; // Habilitar supersampling
    int supersampleFactor = 4; // Fator de supersampling
};

// Estrutura para armazenar dados MSDF
struct MSDFData {
    std::vector<float> red;      // Canal vermelho (distância)
    std::vector<float> green;    // Canal verde (distância)
    std::vector<float> blue;     // Canal azul (distância)
    std::vector<float> alpha;    // Canal alpha (opacidade)
    int width;
    int height;
    float range;
};

// Gerador de MSDF para fontes
class MSDFGenerator {
public:
    MSDFGenerator(const MSDFConfig& config = MSDFConfig{});
    ~MSDFGenerator();

    // Geração de MSDF a partir de contornos
    bool GenerateFromContours(const std::vector<Contour>& contours, MSDFData& output);
    bool GenerateFromGlyph(const void* fontData, uint32_t codepoint, MSDFData& output);
    
    // Conversão de formatos
    bool ConvertToRGBA8(const MSDFData& msdf, std::vector<uint8_t>& output);
    bool ConvertToRGBA32F(const MSDFData& msdf, std::vector<float>& output);
    
    // Aplicação de filtros e melhorias
    void ApplyAntiAliasing(MSDFData& msdf, float smoothing = 0.1f);
    void ApplySubpixelRendering(MSDFData& msdf, float gamma = 2.2f);
    void ApplyContrastEnhancement(MSDFData& msdf, float contrast = 0.1f);
    
    // Configuração
    void SetConfig(const MSDFConfig& config) { m_Config = config; }
    const MSDFConfig& GetConfig() const { return m_Config; }

private:
    MSDFConfig m_Config;
    
    // Algoritmos internos
    struct EdgeSegment;
    struct Edge;
    struct Shape;
    
    // Geração de campo de distância
    void ComputeDistanceField(const Shape& shape, MSDFData& output);
    float ComputeSignedDistance(const glm::vec2& point, const Shape& shape);
    float ComputeEdgeDistance(const glm::vec2& point, const Edge& edge);
    
    // Processamento de contornos
    void ProcessContours(const std::vector<Contour>& contours, Shape& shape);
    void SimplifyContours(std::vector<Contour>& contours);
    void OptimizeContours(std::vector<Contour>& contours);
    
    // Algoritmos de anti-aliasing
    void ApplyMSDFAntiAliasing(MSDFData& msdf);
    void ApplySupersampling(const MSDFData& input, MSDFData& output);
    void ApplyGammaCorrection(MSDFData& msdf, float gamma);
    
    // Utilitários matemáticos
    glm::vec2 ClosestPointOnEdge(const glm::vec2& point, const Edge& edge);
    float SignedDistanceToEdge(const glm::vec2& point, const Edge& edge);
    bool PointInShape(const glm::vec2& point, const Shape& shape);
    
    // Otimizações
    void OptimizeDistanceField(MSDFData& msdf);
    void SmoothDistanceField(MSDFData& msdf, float smoothing);
    void EnhanceContrast(MSDFData& msdf, float contrast);
};

// Processador de fontes TTF/OTF
class FontProcessor {
public:
    FontProcessor();
    ~FontProcessor();

    // Carregamento de fontes
    bool LoadFont(const std::string& filePath);
    bool LoadFontFromMemory(const void* data, size_t dataSize);
    
    // Extração de glyphs
    bool ExtractGlyph(uint32_t codepoint, std::vector<Contour>& contours);
    bool ExtractGlyphMetrics(uint32_t codepoint, float& width, float& height, 
                           float& bearingX, float& bearingY, float& advance);
    
    // Informações da fonte
    float GetAscender() const;
    float GetDescender() const;
    float GetLineHeight() const;
    float GetBaseline() const;
    
    // Configurações
    void SetSize(float size) { m_Size = size; }
    void SetHinting(bool enabled) { m_Hinting = enabled; }
    void SetKerning(bool enabled) { m_Kerning = enabled; }

private:
    struct FontData;
    std::unique_ptr<FontData> m_FontData;
    
    float m_Size;
    bool m_Hinting;
    bool m_Kerning;
    
    // Métodos internos
    bool InitializeFont();
    void ExtractContoursFromGlyph(void* glyph, std::vector<Contour>& contours);
    void ConvertToContours(const void* outline, std::vector<Contour>& contours);
};

// Pipeline completo de processamento de fontes
class FontProcessingPipeline {
public:
    FontProcessingPipeline();
    ~FontProcessingPipeline();

    // Pipeline completo
    bool ProcessFont(const std::string& fontPath, const std::string& outputPath,
                    const MSDFConfig& config = MSDFConfig{});
    
    // Processamento individual de glyphs
    bool ProcessGlyph(uint32_t codepoint, MSDFData& output,
                     const MSDFConfig& config = MSDFConfig{});
    
    // Configurações
    void SetQuality(FontQuality quality);
    void SetAntiAliasingSettings(float smoothing, float contrast, float gamma);

private:
    std::unique_ptr<FontProcessor> m_Processor;
    std::unique_ptr<MSDFGenerator> m_Generator;
    
    // Configurações de qualidade
    FontQuality m_Quality;
    float m_Smoothing;
    float m_Contrast;
    float m_Gamma;
    
    // Métodos internos
    MSDFConfig GetConfigForQuality(FontQuality quality);
    void ApplyQualitySettings(MSDFData& msdf);
};

} // namespace Drift::UI 