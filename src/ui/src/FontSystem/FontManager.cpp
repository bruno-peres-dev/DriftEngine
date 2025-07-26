#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <filesystem>

namespace Drift::UI {

FontManager& FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

FontManager::FontManager() {
    // Inicializar configurações padrão
    m_DefaultQuality = FontQuality::High;
    m_DefaultSize = 16.0f;
    m_DefaultFontName = "default";
}

FontManager::~FontManager() {
    UnloadAllFonts();
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& name, const std::string& filePath, float size, FontQuality quality) {
    FontKey key{name, size, quality};
    
    // Verificar se a fonte já está carregada
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        return it->second;
    }
    
    // Verificar se o arquivo existe
    if (!std::filesystem::exists(filePath)) {
        LOG_ERROR("Font file not found: " + filePath);
        return nullptr;
    }
    
    // Criar nova fonte
    auto font = std::make_shared<Font>(name, filePath, size, quality);
    if (!font->Load()) {
        LOG_ERROR("Failed to load font: " + filePath);
        return nullptr;
    }
    
    m_Fonts[key] = font;
    LOG_INFO("Font loaded successfully: " + name + " (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    
    return font;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    FontKey key{name, size, quality};
    
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        return it->second;
    }
    
    // Tentar carregar a fonte padrão se não encontrada
    if (name != m_DefaultFontName) {
        return GetFont(m_DefaultFontName, size, quality);
    }
    
    // Se não há fonte padrão carregada, criar uma fonte embutida simples
    if (m_Fonts.empty()) {
        LOG_INFO("No fonts loaded, creating embedded default font");
        return CreateEmbeddedDefaultFont(size, quality);
    }
    
    LOG_WARNING("Font not found: " + name + " (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    return nullptr;
}

void FontManager::UnloadFont(const std::string& name, float size, FontQuality quality) {
    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        m_Fonts.erase(it);
        LOG_INFO("Font unloaded: " + key.name + " (size: " + std::to_string(key.size) + ", quality: " + std::to_string(static_cast<int>(key.quality)) + ")");
    }
}

void FontManager::UnloadAllFonts() {
    m_Fonts.clear();
    LOG_INFO("All fonts unloaded");
}

void FontManager::SetDefaultQuality(FontQuality quality) {
    m_DefaultQuality = quality;
}

void FontManager::SetDefaultSize(float size) {
    m_DefaultSize = size;
}

void FontManager::SetDefaultFontName(const std::string& name) {
    m_DefaultFontName = name;
}

void FontManager::PreloadFont(const std::string& name, const std::string& filePath, const std::vector<float>& sizes, FontQuality quality) {
    for (float size : sizes) {
        LoadFont(name, filePath, size, quality);
    }
}

std::shared_ptr<Font> FontManager::CreateEmbeddedDefaultFont(float size, FontQuality quality) {
    // Criar uma fonte simples embutida com caracteres básicos
    auto font = std::make_shared<Font>("embedded_default", "", size, quality);
    
    // Configurar métricas básicas
    font->m_IsLoaded = true;
    font->m_LineHeight = size * 1.2f;
    font->m_Ascender = size * 0.8f;
    font->m_Descender = -size * 0.2f;
    font->m_Scale = 1.0f;
    
    // Criar glyphs básicos para caracteres ASCII (32-126)
    for (uint32_t cp = 32; cp <= 126; ++cp) {
        Glyph glyph{};
        glyph.codepoint = cp;
        glyph.isValid = true;
        glyph.size = glm::vec2(size * 0.6f, size);
        glyph.offset = glm::vec2(0.0f, -size * 0.8f);
        glyph.advance = size * 0.7f;
        glyph.position = glm::vec2(0.0f, 0.0f); // Será calculado pelo atlas
        glyph.uvMin = glm::vec2(0.0f, 0.0f);
        glyph.uvMax = glm::vec2(1.0f, 1.0f);
        
        font->m_Glyphs[cp] = glyph;
    }
    
    // Criar atlas simples
    AtlasConfig config;
    config.width = 512;
    config.height = 512;
    config.padding = 1;
    config.channels = 4;
    config.useMSDF = false; // Fonte simples não usa MSDF
    config.msdfSize = 32;
    
    font->m_Atlas = std::make_unique<FontAtlas>(config);
    
    // Alocar regiões no atlas para cada glyph
    for (auto& [cp, glyph] : font->m_Glyphs) {
        AtlasRegion* region = font->m_Atlas->AllocateRegion(
            static_cast<int>(glyph.size.x), 
            static_cast<int>(glyph.size.y), 
            cp
        );
        
        if (region) {
            glyph.position = glm::vec2(region->x, region->y);
            glyph.uvMin = glm::vec2(
                region->x / static_cast<float>(font->m_Atlas->GetWidth()),
                region->y / static_cast<float>(font->m_Atlas->GetHeight())
            );
            glyph.uvMax = glm::vec2(
                (region->x + region->width) / static_cast<float>(font->m_Atlas->GetWidth()),
                (region->y + region->height) / static_cast<float>(font->m_Atlas->GetHeight())
            );
        }
    }
    
    // Adicionar ao cache
    FontKey key{"embedded_default", size, quality};
    m_Fonts[key] = font;
    
    LOG_INFO("Created embedded default font (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    
    return font;
}

void FontManager::BeginTextRendering() {
    m_IsRendering = true;
}

void FontManager::EndTextRendering() {
    m_IsRendering = false;
}

size_t FontManager::GetLoadedFontCount() const {
    return m_Fonts.size();
}

std::vector<std::string> FontManager::GetLoadedFontNames() const {
    std::vector<std::string> names;
    names.reserve(m_Fonts.size());
    
    for (const auto& pair : m_Fonts) {
        names.push_back(pair.second->GetName());
    }
    
    // Remover duplicatas
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
    
    return names;
}


} // namespace Drift::UI 