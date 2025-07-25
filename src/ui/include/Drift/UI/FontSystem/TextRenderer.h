#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Drift/RHI/Buffer.h"
#include "FontManager.h"

namespace Drift::UI {

// Estrutura para representar um comando de renderização de texto
struct TextRenderCommand {
    std::string text;
    glm::vec2 position;
    std::shared_ptr<Font> font;
    TextRenderSettings settings;
    unsigned color;
    float scale;
    glm::vec2 origin;
    float rotation;
    bool isVisible;
};

// Estrutura para representar um glyph renderizado
struct RenderedGlyph {
    uint32_t codepoint;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 uvMin;
    glm::vec2 uvMax;
    unsigned color;
    float scale;
    glm::vec2 origin;
    float rotation;
    Drift::RHI::ITexture* texture;
};

// Configurações de renderização em lote
struct BatchRenderSettings {
    bool enableBatching = true;
    bool enableInstancing = true;
    int maxBatchSize = 1000;
    bool enableTextureAtlas = true;
    bool enableDistanceField = true;
};

// Renderizador de texto integrado ao UIBatcher
class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    // Inicialização
    bool Initialize(Drift::RHI::IDevice* device);
    void Shutdown();

    // Renderização de texto
    void BeginTextRendering();
    void EndTextRendering();
    
    void RenderText(const std::string& text, const glm::vec2& position,
                   const std::string& fontName = "default", float size = 16.0f,
                   unsigned color = 0xFFFFFFFF, const TextRenderSettings& settings = TextRenderSettings{});
    
    void RenderTextFormatted(const std::string& text, const glm::vec2& position,
                            const std::string& fontName = "default", float size = 16.0f,
                            unsigned color = 0xFFFFFFFF, const TextRenderSettings& settings = TextRenderSettings{});
    
    // Renderização com transformações
    void RenderTextTransformed(const std::string& text, const glm::vec2& position,
                              float scale = 1.0f, float rotation = 0.0f, const glm::vec2& origin = glm::vec2(0.0f),
                              const std::string& fontName = "default", float size = 16.0f,
                              unsigned color = 0xFFFFFFFF, const TextRenderSettings& settings = TextRenderSettings{});

    // Medição de texto
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName = "default", float size = 16.0f);
    std::vector<glm::vec2> MeasureTextLines(const std::string& text, const std::string& fontName = "default", float size = 16.0f);
    
    // Quebra de linha
    std::vector<std::string> WordWrap(const std::string& text, float maxWidth,
                                     const std::string& fontName = "default", float size = 16.0f);
    
    // Configurações
    void SetBatchSettings(const BatchRenderSettings& settings) { m_BatchSettings = settings; }
    void SetScreenSize(float width, float height);
    void SetViewport(const glm::vec4& viewport);

    // Cache e otimização
    void PreloadText(const std::string& text, const std::string& fontName = "default", float size = 16.0f);
    void ClearCache();
    void Optimize();

private:
    // Estado de renderização
    bool m_IsInitialized;
    bool m_IsRendering;
    glm::vec2 m_ScreenSize;
    glm::vec4 m_Viewport;
    
    // Configurações
    BatchRenderSettings m_BatchSettings;
    
    // Comandos de renderização
    std::vector<TextRenderCommand> m_RenderCommands;
    std::vector<RenderedGlyph> m_RenderedGlyphs;
    
    // Pipeline de renderização
    std::unique_ptr<Drift::RHI::IPipelineState> m_TextPipeline;
    std::unique_ptr<Drift::RHI::IBuffer> m_VertexBuffer;
    std::unique_ptr<Drift::RHI::IBuffer> m_IndexBuffer;
    std::unique_ptr<Drift::RHI::IBuffer> m_ConstantBuffer;
    
    // Shaders
    std::unique_ptr<Drift::RHI::IShader> m_VertexShader;
    std::unique_ptr<Drift::RHI::IShader> m_PixelShader;
    
    // Métodos internos
    bool CreateShaders();
    bool CreateBuffers();
    bool CreatePipeline();
    
    void ProcessRenderCommands();
    void BatchGlyphs();
    void RenderGlyphBatch(const std::vector<RenderedGlyph>& batch);
    
    void UpdateVertexBuffer(const std::vector<RenderedGlyph>& glyphs);
    void UpdateConstantBuffer(const glm::mat4& viewProjection);
    
    // Utilitários
    glm::vec2 TransformPosition(const glm::vec2& position, float scale, float rotation, const glm::vec2& origin);
    glm::vec4 TransformColor(unsigned color);
    bool IsGlyphVisible(const RenderedGlyph& glyph);
};

// Integração com UIBatcher
class UIBatcherTextRenderer {
public:
    UIBatcherTextRenderer(Drift::RHI::IUIBatcher* batcher);
    ~UIBatcherTextRenderer();

    // Renderização integrada
    void AddText(float x, float y, const char* text, unsigned color);
    void AddTextFormatted(float x, float y, const char* text, unsigned color, 
                         const TextRenderSettings& settings = TextRenderSettings{});
    
    // Configuração
    void SetFont(const std::string& fontName, float size = 16.0f);
    void SetQuality(FontQuality quality);
    void SetAntiAliasing(bool enabled);

private:
    Drift::RHI::IUIBatcher* m_Batcher;
    std::unique_ptr<TextRenderer> m_TextRenderer;
    
    // Configurações atuais
    std::string m_CurrentFont;
    float m_CurrentSize;
    FontQuality m_CurrentQuality;
    bool m_AntiAliasingEnabled;
    
    // Cache de glyphs para o batcher
    struct CachedGlyph {
        uint32_t codepoint;
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 uvMin;
        glm::vec2 uvMax;
        Drift::RHI::ITexture* texture;
    };
    
    std::unordered_map<uint32_t, CachedGlyph> m_GlyphCache;
    
    // Métodos internos
    void RenderTextToBatcher(const std::string& text, float x, float y, unsigned color);
    CachedGlyph* GetCachedGlyph(uint32_t codepoint);
    void CacheGlyph(uint32_t codepoint, const std::shared_ptr<Font>& font);
};

// Utilitários para renderização de texto
namespace TextUtils {
    // Formatação de texto
    std::string FormatText(const std::string& format, ...);
    std::string TruncateText(const std::string& text, float maxWidth, const std::string& fontName = "default", float size = 16.0f);
    std::string EllipsizeText(const std::string& text, float maxWidth, const std::string& fontName = "default", float size = 16.0f);
    
    // Análise de texto
    std::vector<std::string> SplitText(const std::string& text, char delimiter);
    std::string RemoveFormatting(const std::string& text);
    bool IsValidUTF8(const std::string& text);
    
    // Conversão de codepoints
    std::vector<uint32_t> StringToCodepoints(const std::string& text);
    std::string CodepointsToString(const std::vector<uint32_t>& codepoints);
    
    // Otimização
    void OptimizeTextRendering(const std::vector<std::string>& texts, const std::string& fontName = "default", float size = 16.0f);
}

} // namespace Drift::UI 