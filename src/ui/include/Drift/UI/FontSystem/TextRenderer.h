#pragma once

#include "FontManager.h"
#include "Drift/RHI/UIBatcher.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace Drift::UI {

class TextRenderer {
public:
    TextRenderer();
    void SetBatcher(Drift::RHI::IUIBatcher* batcher) { m_Batcher = batcher; }
    void SetScreenSize(int w, int h) { m_ScreenWidth = w; m_ScreenHeight = h; }

    void BeginTextRendering() {}
    void EndTextRendering() {}

    void AddText(const std::string& text, const glm::vec2& pos,
                 const std::string& fontName, float fontSize, const glm::vec4& color);
    glm::vec2 MeasureText(const std::string& text, const std::string& fontName, float size);

private:
    Drift::RHI::IUIBatcher* m_Batcher{nullptr};
    int m_ScreenWidth{0};
    int m_ScreenHeight{0};
};

class UIBatcherTextRenderer : public TextRenderer {
public:
    explicit UIBatcherTextRenderer(Drift::RHI::IUIBatcher* batcher) {
        SetBatcher(batcher);
    }
};

} // namespace Drift::UI
