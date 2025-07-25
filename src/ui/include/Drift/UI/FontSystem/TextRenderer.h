#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Drift/RHI/Buffer.h"
#include "FontManager.h"

namespace Drift::UI {

struct TextRenderCommand {
    std::string text;
    glm::vec2 position;
    std::shared_ptr<Font> font;
    TextRenderSettings settings;
    glm::vec4 color;
};

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    void BeginTextRendering();
    void EndTextRendering();

    void AddText(const std::string& text, const glm::vec2& position,
                 const std::string& fontName = "default", float fontSize = 16.0f,
                 const glm::vec4& color = glm::vec4(1.0f),
                 const TextRenderSettings& settings = TextRenderSettings{});
    void AddText(const std::string& text, float x, float y,
                 const std::string& fontName = "default", float fontSize = 16.0f,
                 unsigned color = 0xFFFFFFFF,
                 const TextRenderSettings& settings = TextRenderSettings{});

    void NextBatch();
    void ClearBatches();
    size_t GetBatchCount() const;
    size_t GetCommandCount() const;

    void DrawText(const std::string& text, const glm::vec2& position,
                  const std::string& fontName = "default", float fontSize = 16.0f);
    void DrawText(const std::string& text, const glm::vec2& position,
                  const std::string& fontName, float fontSize,
                  const glm::vec4& color);
    void DrawText(const std::string& text, const glm::vec2& position,
                  const std::string& fontName, float fontSize,
                  const glm::vec4& color, const TextRenderSettings& settings);

    glm::vec2 MeasureText(const std::string& text,
                          const std::string& fontName = "default",
                          float fontSize = 16.0f);

private:
    bool m_IsRendering{false};
    size_t m_CurrentBatch{0};
    std::vector<std::vector<TextRenderCommand>> m_Batches;
};

class UIBatcherTextRenderer {
public:
    explicit UIBatcherTextRenderer(Drift::RHI::IUIBatcher* batcher);
    ~UIBatcherTextRenderer();

    void AddText(float x, float y, const char* text, unsigned color);
    void AddText(const std::string& text, const glm::vec2& position,
                 const std::string& fontName, float fontSize,
                 const glm::vec4& color, const TextRenderSettings& settings);
    void BeginTextRendering();
    void EndTextRendering();
    void SetScreenSize(int width, int height);
    TextRenderer* GetTextRenderer() const;

private:
    Drift::RHI::IUIBatcher* m_Batcher{};
    std::unique_ptr<TextRenderer> m_TextRenderer;
    int m_ScreenWidth{0};
    int m_ScreenHeight{0};
};

} // namespace Drift::UI
