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
    std::string key = GenerateFontKey(name, size, quality);
    
    // Verificar se a fonte já está carregada
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        return it->second;
    }
    
    // Verificar se o arquivo existe
    if (!std::filesystem::exists(filePath)) {
        LOG_ERROR("Font file not found: {}", filePath);
        return nullptr;
    }
    
    // Criar nova fonte
    auto font = std::make_shared<Font>(name, filePath, size, quality);
    if (!font->Load()) {
        LOG_ERROR("Failed to load font: {}", filePath);
        return nullptr;
    }
    
    m_Fonts[key] = font;
    LOG_INFO("Font loaded successfully: {} (size: {}, quality: {})", name, size, static_cast<int>(quality));
    
    return font;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    std::string key = GenerateFontKey(name, size, quality);
    
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        return it->second;
    }
    
    // Tentar carregar a fonte padrão se não encontrada
    if (name != m_DefaultFontName) {
        return GetFont(m_DefaultFontName, size, quality);
    }
    
    LOG_WARNING("Font not found: {} (size: {}, quality: {})", name, size, static_cast<int>(quality));
    return nullptr;
}

void FontManager::UnloadFont(const std::string& name, float size, FontQuality quality) {
    std::string key = GenerateFontKey(name, size, quality);
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        m_Fonts.erase(it);
        LOG_INFO("Font unloaded: {}", key);
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

std::string FontManager::GenerateFontKey(const std::string& name, float size, FontQuality quality) {
    return name + "_" + std::to_string(static_cast<int>(size)) + "_" + std::to_string(static_cast<int>(quality));
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