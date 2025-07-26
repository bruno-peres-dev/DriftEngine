#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <fstream>
#include <filesystem>

using namespace Drift::UI;

FontManager& FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

std::shared_ptr<FontManager::TTFData> FontManager::LoadTTFData(const std::string& path) {
    // Verifica se já está no cache (sem lock, será feito pelo caller)
    auto it = m_TTFDataCache.find(path);
    if (it != m_TTFDataCache.end()) {
        it->second->refCount++;
        return it->second;
    }
    
    // Carrega o arquivo TTF
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        Drift::Core::LogError("[FontManager] Não foi possível abrir arquivo TTF: " + path);
        return nullptr;
    }
    
    // Lê o arquivo completo
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    auto ttfData = std::make_shared<TTFData>();
    ttfData->data.resize(fileSize);
    ttfData->path = path;
    ttfData->refCount = 1;
    
    file.read(reinterpret_cast<char*>(ttfData->data.data()), fileSize);
    file.close();
    
    // Armazena no cache
    m_TTFDataCache[path] = ttfData;
    
    Drift::Core::LogRHIDebug("[FontManager] TTF carregado no cache: " + path + " (" + std::to_string(fileSize) + " bytes)");
    return ttfData;
}

void FontManager::ReleaseTTFData(const std::string& path) {
    auto it = m_TTFDataCache.find(path);
    if (it != m_TTFDataCache.end()) {
        it->second->refCount--;
        if (it->second->refCount == 0) {
            m_TTFDataCache.erase(it);
            Drift::Core::LogRHIDebug("[FontManager] TTF removido do cache: " + path);
        }
    }
}

void FontManager::PreloadFontFile(const std::string& path) {
    if (std::filesystem::exists(path)) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        LoadTTFData(path);
        Drift::Core::LogRHI("[FontManager] Fonte pré-carregada: " + path);
    } else {
        Drift::Core::LogWarning("[FontManager] Arquivo de fonte não encontrado: " + path);
    }
}

void FontManager::ClearCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Fonts.clear();
    m_TTFDataCache.clear();
    Drift::Core::LogRHI("[FontManager] Cache limpo");
}

size_t FontManager::GetCacheSize() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Fonts.size();
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& name, const std::string& path, float size, FontQuality quality) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
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

    // Carrega dados TTF compartilhados (sem lock adicional, já temos o lock)
    auto ttfData = LoadTTFData(path);
    if (!ttfData) {
        Drift::Core::LogError("[FontManager] Falha ao carregar dados TTF: " + path);
        return nullptr;
    }

    Drift::Core::LogRHIDebug("[FontManager] Criando nova fonte...");
    auto font = std::make_shared<Font>(name, size, quality);
    
    try {
        // Passa os dados TTF já carregados para a fonte
        if (font->LoadFromMemory(ttfData->data.data(), ttfData->data.size(), m_Device)) {
            m_Fonts[key] = font;
            Drift::Core::LogRHI("[FontManager] Fonte carregada e armazenada no cache");
            return font;
        } else {
            Drift::Core::LogError("[FontManager] Falha ao carregar fonte: " + name);
            ReleaseTTFData(path);
            return nullptr;
        }
    } catch (const std::exception& e) {
        Drift::Core::LogException("[FontManager] Exceção ao carregar fonte", e);
        ReleaseTTFData(path);
        return nullptr;
    }
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) return it->second;
    return nullptr;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size) {
    return GetFont(name, size, m_DefaultQuality);
}

std::shared_ptr<Font> FontManager::GetOrLoadFont(const std::string& name, const std::string& path, float size, FontQuality quality) {
    // Primeiro tenta pegar do cache
    auto font = GetFont(name, size, quality);
    if (font) {
        return font;
    }
    
    // Se não encontrou, carrega sob demanda
    Drift::Core::LogRHIDebug("[FontManager] Lazy loading fonte: " + name + " (tamanho: " + std::to_string(size) + ")");
    return LoadFont(name, path, size, quality);
}

void FontManager::PreloadCommonSizes(const std::string& name, const std::string& path, const std::vector<float>& sizes) {
    Drift::Core::LogRHI("[FontManager] Pré-carregando " + std::to_string(sizes.size()) + " tamanhos para fonte: " + name);
    
    for (float size : sizes) {
        try {
            auto font = LoadFont(name, path, size, m_DefaultQuality);
            if (font) {
                Drift::Core::LogRHIDebug("[FontManager] Tamanho " + std::to_string(size) + " carregado com sucesso");
            }
        } catch (const std::exception& e) {
            Drift::Core::LogError("[FontManager] Erro ao carregar tamanho " + std::to_string(size) + ": " + e.what());
        }
    }
}

