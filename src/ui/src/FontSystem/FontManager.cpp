#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

FontManager& FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& name, const std::string& path, float size, FontQuality quality) {
    Drift::Core::LogRHI("[FontManager] Carregando fonte: " + name + " (tamanho: " + std::to_string(size) + ")");
    
    if (!m_Device) {
        Drift::Core::LogError("[FontManager] Device não configurado");
        return nullptr;
    }

    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        Drift::Core::LogRHIDebug("[FontManager] Fonte já carregada, retornando cache");
        return it->second;
    }

    Drift::Core::LogRHIDebug("[FontManager] Criando nova fonte...");
    auto font = std::make_shared<Font>(name, size, quality);
    
    try {
        if (font->LoadFromFile(path, m_Device)) {
            m_Fonts[key] = font;
            Drift::Core::LogRHI("[FontManager] Fonte carregada e armazenada no cache");
            return font;
        } else {
            Drift::Core::LogError("[FontManager] Falha ao carregar fonte: " + name);
            return nullptr;
        }
    } catch (const std::exception& e) {
        Drift::Core::LogException("[FontManager] Exceção ao carregar fonte", e);
        return nullptr;
    }
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

