#include "Drift/Core/Assets/FontAsset.h"
#include "Drift/Core/Log.h"
#include <filesystem>
#include <algorithm>

namespace Drift::Core::Assets {

// =============================================================================
// FontAsset Implementation
// =============================================================================

FontAsset::FontAsset(const std::string& path, std::shared_ptr<UI::Font> font)
    : m_Path(path), m_Font(font) {
    if (m_Font) {
        // Estima uso de memória baseado na fonte existente
        FontLoader loader;
        m_EstimatedMemoryUsage = loader.EstimateFontMemoryUsage(m_Size, m_Quality);
    }
}

size_t FontAsset::GetMemoryUsage() const {
    if (m_Font) {
        // Em uma implementação real, a Font teria um método GetMemoryUsage()
        // Por enquanto, usa a estimativa
        return m_EstimatedMemoryUsage;
    }
    return m_EstimatedMemoryUsage;
}

bool FontAsset::Load() {
    if (IsLoaded()) {
        return true;
    }
    
    // Recarrega a fonte através do FontManager
    auto& fontManager = UI::FontManager::GetInstance();
    m_Font = fontManager.LoadFont(m_FontName, m_Path, m_Size, m_Quality);
    
    if (m_Font) {
        Log("[FontAsset] Fonte recarregada: " + m_Path + " (tamanho: " + std::to_string(m_Size) + ")");
        return true;
    } else {
        Log("[FontAsset] ERRO: Falha ao recarregar fonte: " + m_Path);
        return false;
    }
}

void FontAsset::Unload() {
    if (m_Font) {
        Log("[FontAsset] Descarregando fonte: " + m_Path + " (tamanho: " + std::to_string(m_Size) + ")");
        m_Font.reset();
    }
}

void FontAsset::SetLoadParams(const FontLoadParams& params) {
    m_Size = params.size;
    m_Quality = params.quality;
    if (!params.name.empty()) {
        m_FontName = params.name;
    }
    m_EstimatedMemoryUsage = EstimateFontMemoryUsage(m_Size, m_Quality);
}

// =============================================================================
// FontLoader Implementation
// =============================================================================

std::shared_ptr<FontAsset> FontLoader::Load(const std::string& path, const std::any& params) {
    if (!CanLoad(path)) {
        Log("[FontLoader] ERRO: Não é possível carregar o arquivo: " + path);
        return nullptr;
    }
    
    FontLoadParams loadParams = ExtractParams(params);
    std::string fontName = ExtractFontName(path, loadParams.name);
    
    // Carrega a fonte através do FontManager
    auto& fontManager = UI::FontManager::GetInstance();
    auto font = fontManager.LoadFont(fontName, path, loadParams.size, loadParams.quality);
    
    if (!font) {
        Log("[FontLoader] ERRO: Falha ao carregar fonte: " + path);
        return nullptr;
    }
    
    // Cria o asset wrapper
    auto asset = std::make_shared<FontAsset>(path, font);
    asset->SetLoadParams(loadParams);
    
    Log("[FontLoader] Fonte carregada com sucesso: " + path + 
        " (nome: " + fontName + ", tamanho: " + std::to_string(loadParams.size) + ")");
    
    return asset;
}

bool FontLoader::CanLoad(const std::string& path) const {
    std::filesystem::path fsPath(path);
    std::string extension = fsPath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    auto supportedExts = GetSupportedExtensions();
    return std::find(supportedExts.begin(), supportedExts.end(), extension) != supportedExts.end();
}

std::vector<std::string> FontLoader::GetSupportedExtensions() const {
    return {
        ".ttf", ".otf", ".woff", ".woff2", ".eot"
    };
}

FontLoadParams FontLoader::ExtractParams(const std::any& params) const {
    FontLoadParams defaultParams;
    
    if (params.has_value()) {
        try {
            return std::any_cast<FontLoadParams>(params);
        } catch (const std::bad_any_cast& e) {
            Log("[FontLoader] AVISO: Parâmetros inválidos, usando padrões");
        }
    }
    
    return defaultParams;
}

std::string FontLoader::ExtractFontName(const std::string& path, const std::string& requestedName) const {
    if (!requestedName.empty()) {
        return requestedName;
    }
    
    // Extrai o nome do arquivo sem extensão
    std::filesystem::path fsPath(path);
    return fsPath.stem().string();
}

size_t FontLoader::EstimateFontMemoryUsage(float size, UI::FontQuality quality) const {
    // Estimativa baseada no tamanho da fonte e qualidade
    size_t baseMemory = static_cast<size_t>(size * size * 4); // Aproximação para atlas de glifos
    
    switch (quality) {
        case UI::FontQuality::Low:
            return baseMemory;
        case UI::FontQuality::Medium:
            return static_cast<size_t>(baseMemory * 1.5f);
        case UI::FontQuality::High:
            return static_cast<size_t>(baseMemory * 2.0f);
        case UI::FontQuality::Ultra:
            return static_cast<size_t>(baseMemory * 3.0f);
        default:
            return baseMemory;
    }
}

} // namespace Drift::Core::Assets