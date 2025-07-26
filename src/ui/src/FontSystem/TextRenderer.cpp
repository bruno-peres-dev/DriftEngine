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
    
    LOG_DEBUG("Text rendering ended, processed " + std::to_string(m_Batches.size()) + " batches");
}

void TextRenderer::AddText(const std::string& text, const glm::vec2& position, 
                          const std::string& fontName, float fontSize, 
                          const glm::vec4& color, const TextRenderSettings& settings) {
    if (!m_IsRendering) {
        LOG_WARNING("Text rendering not in progress, call BeginTextRendering() first");
        return;
    }
    
    // Verificação de segurança para texto vazio
    if (text.empty()) {
        LOG_DEBUG("AddText: texto vazio, ignorando");
        return;
    }
    
    // Obter a fonte
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont(fontName, fontSize, settings.quality);
    if (!font) {
        LOG_ERROR("Font not found: " + fontName + " (size: " + std::to_string(fontSize) + ")");
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
    
    LOG_DEBUG("Added text command: '" + text + "' at (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
}

void TextRenderer::AddText(const std::string& text, float x, float y,
                          const std::string& fontName, float fontSize,
                          Drift::Color color, const TextRenderSettings& settings) {
    // Verificação de segurança para texto vazio
    if (text.empty()) {
        LOG_DEBUG("AddText: texto vazio, ignorando");
        return;
    }
    
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
    LOG_DEBUG("Moved to batch " + std::to_string(m_CurrentBatch));
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
    LOG_INFO("Processing " + std::to_string(m_Batches.size()) + " text batches with " + std::to_string(GetCommandCount()) + " total commands");
    
    for (size_t i = 0; i < m_Batches.size(); ++i) {
        const auto& batch = m_Batches[i];
        LOG_DEBUG("Processing batch " + std::to_string(i) + " with " + std::to_string(batch.size()) + " commands");
        
        for (const auto& command : batch) {
            ProcessTextCommand(command);
        }
    }
}

void TextRenderer::ProcessTextCommand(const TextRenderCommand& command) {
    // Verificações de segurança
    if (command.text.empty()) {
        LOG_DEBUG("ProcessTextCommand: texto vazio, ignorando");
        return;
    }
    
    if (!command.font) {
        LOG_ERROR("ProcessTextCommand: fonte é nullptr!");
        return;
    }
    
    LOG_DEBUG("Processing text: '" + command.text + "' at (" + std::to_string(command.position.x) + ", " + std::to_string(command.position.y) + ") with font '" + command.font->GetName() + "'");
    
    // Renderização real de glyphs
    float currentX = command.position.x;
    float currentY = command.position.y;
    
    // Obtém o atlas da fonte
    const auto& atlas = command.font->GetAtlas();
    if (!atlas) {
        LOG_ERROR("ProcessTextCommand: atlas da fonte é nullptr!");
        return;
    }
    
    // Obtém a textura do atlas
    auto* texture = atlas->GetTexture();
    if (!texture) {
        LOG_DEBUG("ProcessTextCommand: textura do atlas é nullptr, usando renderização de retângulos coloridos");
        // Continua mesmo sem textura - renderiza retângulos coloridos
    }
    
    for (size_t i = 0; i < command.text.length(); ++i) {
        char c = command.text[i];
        uint32_t character = static_cast<uint32_t>(c);
        const Glyph* glyph = command.font->GetGlyph(character);
        
        if (glyph && glyph->isValid) {
            // Calcula posição do glyph
            float glyphX = currentX + glyph->offset.x;
            float glyphY = currentY - glyph->offset.y;
            
            LOG_DEBUG("  Rendering glyph '" + std::string(1, c) + "' at (" + std::to_string(glyphX) + ", " + std::to_string(glyphY) + ") size " + std::to_string(glyph->size.x) + "x" + std::to_string(glyph->size.y));
            
            // Renderiza o quad do glyph
            if (m_UIBatcher) {
                // Por enquanto, renderiza um retângulo colorido representando o glyph
                // TODO: Implementar renderização real com textura quando o atlas estiver pronto
                // Converte glm::vec4 para Drift::Color
                Drift::Color color = static_cast<uint32_t>(command.color.a * 255) << 24 |
                                   static_cast<uint32_t>(command.color.r * 255) << 16 |
                                   static_cast<uint32_t>(command.color.g * 255) << 8 |
                                   static_cast<uint32_t>(command.color.b * 255);
                
                m_UIBatcher->AddRect(
                    glyphX, glyphY,                    // Posição
                    glyph->size.x, glyph->size.y,      // Tamanho
                    color                               // Cor
                );
            }
            
            currentX += glyph->advance;
            
            // Aplicar kerning se não for o último caractere
            if (i + 1 < command.text.length()) {
                uint32_t nextChar = static_cast<uint32_t>(command.text[i + 1]);
                currentX += command.font->GetKerning(character, nextChar);
            }
        } else {
            LOG_WARNING("ProcessTextCommand: glyph não encontrado para caractere '" + std::string(1, c) + "' (codepoint: " + std::to_string(character) + ")");
        }
    }
}

// UIBatcherTextRenderer implementation

UIBatcherTextRenderer::UIBatcherTextRenderer(Drift::RHI::IUIBatcher* batcher)
    : m_Batcher(batcher)
    , m_TextRenderer(std::make_unique<TextRenderer>()) {
    
    if (!m_Batcher) {
        LOG_ERROR("UIBatcher is null");
    } else {
        // Passa a referência do UIBatcher para o TextRenderer
        m_TextRenderer->m_UIBatcher = batcher;
    }
}

UIBatcherTextRenderer::~UIBatcherTextRenderer() = default;

void UIBatcherTextRenderer::AddText(float x, float y, const char* text, Drift::Color color) {
    if (!m_TextRenderer) {
        LOG_ERROR("UIBatcherTextRenderer::AddText: m_TextRenderer é nullptr");
        return;
    }
    
    if (!text) {
        LOG_ERROR("UIBatcherTextRenderer::AddText: text é nullptr");
        return;
    }
    
    // Verificar se a string não está vazia
    std::string textStr(text);
    if (textStr.empty()) {
        LOG_DEBUG("UIBatcherTextRenderer::AddText: texto vazio, ignorando");
        return;
    }
    
    // Usar configurações padrão
    TextRenderSettings settings;
    settings.quality = FontQuality::High;
    settings.smoothing = 0.5f;
    settings.gamma = 2.2f;
    settings.enableSubpixel = true;
    
    m_TextRenderer->AddText(textStr, x, y, "default", 16.0f, color, settings);
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
    
    LOG_DEBUG("Screen size set to " + std::to_string(width) + "x" + std::to_string(height));
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
    
    LOG_DEBUG("DrawText called: '" + text + "' at (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ") with font '" + fontName + "' size " + std::to_string(fontSize));
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