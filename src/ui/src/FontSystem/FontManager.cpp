#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

FontManager& FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& name, const std::string& path, float size, FontQuality quality) {
    if (!m_Device) {
        Drift::Core::LogError("[FontManager] Device não configurado");
        return nullptr;
    }

    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        return it->second;
    }

    auto font = std::make_shared<Font>(name, size, quality);
    if (font->LoadFromFile(path, m_Device)) {
        m_Fonts[key] = font;
        return font;
    }
    return nullptr;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) return it->second;
    return nullptr;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size) {
    return GetFont(name, size, m_DefaultQuality);
}

