#include "Drift/UI/FontSystem/TextRenderer.h"
#include "Drift/Core/Log.h"
#include <algorithm>

namespace Drift::UI {

// TextRenderer implementation

TextRenderer::TextRenderer()
    : m_IsRendering(false)
    , m_CurrentBatch(0) {
}

TextRenderer::~TextRenderer() {
    ClearBatches();
}

void TextRenderer::BeginTextRendering() {
    if (m_IsRendering) {
        LOG_WARNING("Text rendering already in progress");
        return;
    }
    
    m_IsRendering = true;
    m_CurrentBatch = 0;
    
    LOG_DEBUG("Text rendering started");
}

void TextRenderer::EndTextRendering() {
    if (!m_IsRendering) {
        LOG_WARNING("Text rendering not in progress");
        return;
    }
    
    m_IsRendering = false;
    
    // Processar todos os batches
    ProcessBatches();
    
    LOG_DEBUG("Text rendering ended, processed {} batches", m_Batches.size());
}

void TextRenderer::AddText(const std::string& text, const glm::vec2& position, 
                          const std::string& fontName, float fontSize, 
                          const glm::vec4& color, const TextRenderSettings& settings) {
    if (!m_IsRendering) {
        LOG_WARNING("Text rendering not in progress, call BeginTextRendering() first");
        return;
    }
    
    // Obter a fonte
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont(fontName, fontSize, settings.quality);
    if (!font) {
        LOG_ERROR("Font not found: {} (size: {})", fontName, fontSize);
        return;
    }
    
    // Criar comando de renderização
    TextRenderCommand command;
    command.text = text;
    command.position = position;
    command.font = font;
    command.color = color;
    command.settings = settings;
    
    // Adicionar ao batch atual
    if (m_Batches.empty() || m_CurrentBatch >= m_Batches.size()) {
        m_Batches.push_back(std::vector<TextRenderCommand>());
    }
    
    m_Batches[m_CurrentBatch].push_back(command);
    
    LOG_DEBUG("Added text command: '{}' at ({}, {})", text, position.x, position.y);
}

void TextRenderer::AddText(const std::string& text, float x, float y, 
                          const std::string& fontName, float fontSize, 
                          unsigned color, const TextRenderSettings& settings) {
    glm::vec4 colorVec;
    colorVec.r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
    colorVec.g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
    colorVec.b = static_cast<float>(color & 0xFF) / 255.0f;
    colorVec.a = static_cast<float>((color >> 24) & 0xFF) / 255.0f;
    
    AddText(text, glm::vec2(x, y), fontName, fontSize, colorVec, settings);
}

void TextRenderer::NextBatch() {
    if (!m_IsRendering) {
        LOG_WARNING("Text rendering not in progress");
        return;
    }
    
    m_CurrentBatch++;
    LOG_DEBUG("Moved to batch {}", m_CurrentBatch);
}

void TextRenderer::ClearBatches() {
    m_Batches.clear();
    m_CurrentBatch = 0;
    LOG_DEBUG("Text batches cleared");
}

size_t TextRenderer::GetBatchCount() const {
    return m_Batches.size();
}

size_t TextRenderer::GetCommandCount() const {
    size_t total = 0;
    for (const auto& batch : m_Batches) {
        total += batch.size();
    }
    return total;
}

void TextRenderer::ProcessBatches() {
    LOG_INFO("Processing {} text batches with {} total commands", 
             m_Batches.size(), GetCommandCount());
    
    for (size_t i = 0; i < m_Batches.size(); ++i) {
        const auto& batch = m_Batches[i];
        LOG_DEBUG("Processing batch {} with {} commands", i, batch.size());
        
        for (const auto& command : batch) {
            ProcessTextCommand(command);
        }
    }
}

void TextRenderer::ProcessTextCommand(const TextRenderCommand& command) {
    // Aqui seria implementada a lógica real de renderização
    // Por enquanto, vamos apenas simular o processamento
    
    LOG_DEBUG("Processing text: '{}' at ({}, {}) with font '{}'", 
              command.text, command.position.x, command.position.y, 
              command.font->GetName());
    
    // Simular renderização de glyphs
    float currentX = command.position.x;
    float currentY = command.position.y;
    
    for (char c : command.text) {
        uint32_t character = static_cast<uint32_t>(c);
        const Glyph* glyph = command.font->GetGlyph(character);
        
        if (glyph) {
            // Simular posicionamento do glyph usando offsets
            float glyphX = currentX + glyph->offset.x;
            float glyphY = currentY - glyph->offset.y;
            
            LOG_DEBUG("  Glyph '{}' at ({}, {}) size {}x{}", 
                      c, glyphX, glyphY, glyph->size.x, glyph->size.y);
            
            currentX += glyph->advance;
            
            // Aplicar kerning se não for o último caractere
            if (c != command.text.back()) {
                currentX += command.font->GetKerning(character, 
                    static_cast<uint32_t>(command.text[&c - &command.text[0] + 1]));
            }
        }
    }
}

// UIBatcherTextRenderer implementation

UIBatcherTextRenderer::UIBatcherTextRenderer(Drift::RHI::IUIBatcher* batcher)
    : m_Batcher(batcher)
    , m_TextRenderer(std::make_unique<TextRenderer>()) {
    
    if (!m_Batcher) {
        LOG_ERROR("UIBatcher is null");
    }
}

UIBatcherTextRenderer::~UIBatcherTextRenderer() = default;

void UIBatcherTextRenderer::AddText(float x, float y, const char* text, unsigned color) {
    if (!m_TextRenderer || !text) {
        return;
    }
    
    // Usar configurações padrão
    TextRenderSettings settings;
    settings.quality = FontQuality::High;
    settings.smoothing = 0.5f;
    settings.gamma = 2.2f;
    settings.enableSubpixel = true;
    
    m_TextRenderer->AddText(text, x, y, "default", 16.0f, color, settings);
}

void UIBatcherTextRenderer::AddText(const std::string& text, const glm::vec2& position, 
                                   const std::string& fontName, float fontSize, 
                                   const glm::vec4& color, const TextRenderSettings& settings) {
    if (!m_TextRenderer) {
        return;
    }
    
    m_TextRenderer->AddText(text, position, fontName, fontSize, color, settings);
}

void UIBatcherTextRenderer::BeginTextRendering() {
    if (m_TextRenderer) {
        m_TextRenderer->BeginTextRendering();
    }
}

void UIBatcherTextRenderer::EndTextRendering() {
    if (m_TextRenderer) {
        m_TextRenderer->EndTextRendering();
    }
}

void UIBatcherTextRenderer::SetScreenSize(int width, int height) {
    m_ScreenWidth = width;
    m_ScreenHeight = height;
    
    LOG_DEBUG("Screen size set to {}x{}", width, height);
}

TextRenderer* UIBatcherTextRenderer::GetTextRenderer() const {
    return m_TextRenderer.get();
}

// Namespace functions

void TextRenderer::DrawText(const std::string& text, const glm::vec2& position, 
                           const std::string& fontName, float fontSize) {
    TextRenderSettings settings;
    settings.quality = FontQuality::High;
    
    DrawText(text, position, fontName, fontSize, glm::vec4(1.0f), settings);
}

void TextRenderer::DrawText(const std::string& text, const glm::vec2& position, 
                           const std::string& fontName, float fontSize, 
                           const glm::vec4& color) {
    TextRenderSettings settings;
    settings.quality = FontQuality::High;
    
    DrawText(text, position, fontName, fontSize, color, settings);
}

void TextRenderer::DrawText(const std::string& text, const glm::vec2& position, 
                           const std::string& fontName, float fontSize, 
                           const glm::vec4& color, const TextRenderSettings& settings) {
    // Esta função seria chamada pelo sistema de renderização
    // Por enquanto, vamos apenas logar a chamada
    
    LOG_DEBUG("DrawText called: '{}' at ({}, {}) with font '{}' size {}", 
              text, position.x, position.y, fontName, fontSize);
}

glm::vec2 TextRenderer::MeasureText(const std::string& text, const std::string& fontName, float fontSize) {
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont(fontName, fontSize);
    
    if (font) {
        return font->MeasureText(text);
    }
    
    return glm::vec2(0.0f);
}

} // namespace Drift::UI 