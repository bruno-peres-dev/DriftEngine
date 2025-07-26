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
    Core::Log("[FontManager] Inicializando FontManager...");
    
    // Inicializar configurações padrão
    m_DefaultQuality = FontQuality::High;
    m_DefaultSize = 16.0f;
    m_DefaultFontName = "default";
    
    Core::Log("[FontManager] Configuracoes padrao:");
    Core::Log("[FontManager]   - Qualidade: " + std::to_string(static_cast<int>(m_DefaultQuality)));
    Core::Log("[FontManager]   - Tamanho: " + std::to_string(m_DefaultSize));
    Core::Log("[FontManager]   - Nome padrão: " + m_DefaultFontName);
    Core::Log("[FontManager] FontManager inicializado com sucesso!");
}

FontManager::~FontManager() {
    Core::Log("[FontManager] Destruindo FontManager...");
    UnloadAllFonts();
    Core::Log("[FontManager] FontManager destruído!");
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& name, const std::string& filePath, float size, FontQuality quality) {
    Core::Log("[FontManager] Carregando fonte: " + name + " (" + filePath + ")");
    Core::Log("[FontManager]   - Tamanho: " + std::to_string(size));
    Core::Log("[FontManager]   - Qualidade: " + std::to_string(static_cast<int>(quality)));
    
    FontKey key{name, size, quality};
    
    // Verificar se a fonte já está carregada
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        Core::Log("[FontManager] Fonte já carregada, retornando cache: " + name);
        return it->second;
    }
    
    // Verificar se o arquivo existe
    if (!std::filesystem::exists(filePath)) {
        Core::Log("[FontManager] ERRO: Arquivo de fonte não encontrado: " + filePath);
        LOG_ERROR("Font file not found: " + filePath);
        return nullptr;
    }
    
    Core::Log("[FontManager] Criando nova fonte...");
    // Criar nova fonte
    auto font = std::make_shared<Font>(name, filePath, size, quality);
    if (!font->Load()) {
        Core::Log("[FontManager] ERRO: Falha ao carregar fonte: " + filePath);
        LOG_ERROR("Failed to load font: " + filePath);
        return nullptr;
    }
    
    m_Fonts[key] = font;
    Core::Log("[FontManager] Fonte carregada com sucesso: " + name);
    Core::Log("[FontManager] Total de fontes carregadas: " + std::to_string(m_Fonts.size()));
    LOG_INFO("Font loaded successfully: " + name + " (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    
    return font;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    Core::Log("[FontManager] Buscando fonte: " + name + " (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    
    FontKey key{name, size, quality};
    
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        Core::Log("[FontManager] Fonte encontrada no cache: " + name);
        return it->second;
    }
    
    // Tentar carregar a fonte padrão se não encontrada
    if (name != m_DefaultFontName) {
        Core::Log("[FontManager] Fonte nao encontrada, tentando fonte padrao: " + m_DefaultFontName);
        return GetFont(m_DefaultFontName, size, quality);
    }
    
    // Se não há fonte padrão carregada, criar uma fonte embutida simples
    if (m_Fonts.empty()) {
        Core::Log("[FontManager] Nenhuma fonte carregada, criando fonte embutida padrao");
        LOG_INFO("No fonts loaded, creating embedded default font");
        return CreateEmbeddedDefaultFont(size, quality);
    }
    
            Core::Log("[FontManager] AVISO: Fonte nao encontrada: " + name);
    LOG_WARNING("Font not found: " + name + " (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    return nullptr;
}

void FontManager::UnloadFont(const std::string& name, float size, FontQuality quality) {
    Core::Log("[FontManager] Descarregando fonte: " + name + " (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    
    FontKey key{name, size, quality};
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        m_Fonts.erase(it);
        Core::Log("[FontManager] Fonte descarregada com sucesso: " + name);
        Core::Log("[FontManager] Total de fontes restantes: " + std::to_string(m_Fonts.size()));
        LOG_INFO("Font unloaded: " + key.name + " (size: " + std::to_string(key.size) + ", quality: " + std::to_string(static_cast<int>(key.quality)) + ")");
    } else {
        Core::Log("[FontManager] AVISO: Fonte nao encontrada para descarregar: " + name);
    }
}

void FontManager::UnloadAllFonts() {
    Core::Log("[FontManager] Descarregando todas as fontes...");
    size_t count = m_Fonts.size();
    m_Fonts.clear();
    Core::Log("[FontManager] " + std::to_string(count) + " fontes descarregadas");
    LOG_INFO("All fonts unloaded");
}

void FontManager::SetDefaultQuality(FontQuality quality) {
    Core::Log("[FontManager] Configurando qualidade padrao: " + std::to_string(static_cast<int>(quality)));
    m_DefaultQuality = quality;
}

void FontManager::SetDefaultSize(float size) {
    Core::Log("[FontManager] Configurando tamanho padrao: " + std::to_string(size));
    m_DefaultSize = size;
}

void FontManager::SetDefaultFontName(const std::string& name) {
    Core::Log("[FontManager] Configurando fonte padrao: " + name);
    m_DefaultFontName = name;
}

void FontManager::PreloadFont(const std::string& name, const std::string& filePath, const std::vector<float>& sizes, FontQuality quality) {
    Core::Log("[FontManager] Pré-carregando fonte: " + name + " (" + filePath + ")");
    Core::Log("[FontManager]   - Tamanhos: " + std::to_string(sizes.size()) + " variações");
    Core::Log("[FontManager]   - Qualidade: " + std::to_string(static_cast<int>(quality)));
    
    for (float size : sizes) {
        Core::Log("[FontManager]   - Carregando tamanho: " + std::to_string(size));
        LoadFont(name, filePath, size, quality);
    }
    
    Core::Log("[FontManager] Pré-carregamento concluído para: " + name);
}

std::shared_ptr<Font> FontManager::CreateEmbeddedDefaultFont(float size, FontQuality quality) {
    Core::Log("[FontManager] Criando fonte embutida padrao...");
    Core::Log("[FontManager]   - Tamanho: " + std::to_string(size));
    Core::Log("[FontManager]   - Qualidade: " + std::to_string(static_cast<int>(quality)));
    
    // Criar uma fonte simples embutida com caracteres básicos
    auto font = std::make_shared<Font>("embedded_default", "", size, quality);
    
    // Configurar métricas básicas
    font->m_IsLoaded = true;
    font->m_Metrics.lineHeight = size * 1.2f;
    font->m_Metrics.ascender = size * 0.8f;
    font->m_Metrics.descender = -size * 0.2f;
    font->m_Scale = 1.0f;
    
    Core::Log("[FontManager] Criando glyphs para caracteres ASCII (32-126)...");
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
    
    Core::Log("[FontManager] Criando atlas de textura...");
    // Criar atlas simples
    AtlasConfig config;
    config.width = 512;
    config.height = 512;
    config.padding = 1;
    config.channels = 4;
    config.useMSDF = false; // Fonte simples não usa MSDF
    config.msdfSize = 32;
    
    font->m_Atlas = std::make_unique<FontAtlas>(config);
    
    Core::Log("[FontManager] Alocando regioes no atlas...");
    Core::Log("[FontManager]   - Total de glyphs para alocar: " + std::to_string(font->m_Glyphs.size()));
    // Alocar regiões no atlas para cada glyph
    int allocatedCount = 0;
    int totalGlyphs = static_cast<int>(font->m_Glyphs.size());
    try {
        for (auto& [cp, glyph] : font->m_Glyphs) {
            Core::Log("[FontManager]   - Progresso: " + std::to_string(allocatedCount + 1) + "/" + std::to_string(totalGlyphs));
            std::string charStr;
            if (cp >= 32 && cp <= 126) {
                charStr = "'" + std::string(1, static_cast<char>(cp)) + "'";
            } else {
                charStr = "[nao-printavel]";
            }
            Core::Log("[FontManager]   - Alocando glyph para codepoint: " + std::to_string(cp) + " (char: " + charStr + ")");
            Core::Log("[FontManager]   - Tamanho do glyph: " + std::to_string(glyph.size.x) + "x" + std::to_string(glyph.size.y));
            
            AtlasRegion* region = font->m_Atlas->AllocateRegion(
                static_cast<int>(glyph.size.x), 
                static_cast<int>(glyph.size.y), 
                cp
            );
            
            if (region) {
                Core::Log("[FontManager]     - Configurando UVs do glyph...");
                Core::Log("[FontManager]     - Configurando posicao...");
                Core::Log("[FontManager]     - Valores da regiao: x=" + std::to_string(region->x) + ", y=" + std::to_string(region->y));
                Core::Log("[FontManager]     - Criando glm::vec2...");
                Core::Log("[FontManager]     - Antes de glyph.position = glm::vec2...");
                glyph.position = glm::vec2(region->x, region->y);
                Core::Log("[FontManager]     - Depois de glyph.position = glm::vec2...");
                Core::Log("[FontManager]     - Posicao configurada: (" + std::to_string(glyph.position.x) + ", " + std::to_string(glyph.position.y) + ")");
                
                Core::Log("[FontManager]     - Calculando UVs...");
                float atlasWidth = static_cast<float>(font->m_Atlas->GetWidth());
                float atlasHeight = static_cast<float>(font->m_Atlas->GetHeight());
                Core::Log("[FontManager]     - Tamanho do atlas: " + std::to_string(atlasWidth) + "x" + std::to_string(atlasHeight));
                
                glyph.uvMin = glm::vec2(
                    region->x / atlasWidth,
                    region->y / atlasHeight
                );
                Core::Log("[FontManager]     - UV min calculado: (" + std::to_string(glyph.uvMin.x) + ", " + std::to_string(glyph.uvMin.y) + ")");
                
                glyph.uvMax = glm::vec2(
                    (region->x + region->width) / atlasWidth,
                    (region->y + region->height) / atlasHeight
                );
                Core::Log("[FontManager]     - UV max calculado: (" + std::to_string(glyph.uvMax.x) + ", " + std::to_string(glyph.uvMax.y) + ")");
                
                allocatedCount++;
                Core::Log("[FontManager]     - Glyph alocado com sucesso na posicao: (" + std::to_string(region->x) + ", " + std::to_string(region->y) + ")");
                Core::Log("[FontManager]     - UVs configuradas: min(" + std::to_string(glyph.uvMin.x) + ", " + std::to_string(glyph.uvMin.y) + ") max(" + std::to_string(glyph.uvMax.x) + ", " + std::to_string(glyph.uvMax.y) + ")");
            } else {
                Core::Log("[FontManager]     - ERRO: Falha ao alocar glyph para codepoint: " + std::to_string(cp));
            }
            Core::Log("[FontManager]     - Glyph " + std::to_string(cp) + " processado. Proximo...");
        }
        Core::Log("[FontManager] Total de glyphs alocados: " + std::to_string(allocatedCount) + "/" + std::to_string(font->m_Glyphs.size()));
    } catch (const std::exception& e) {
        Core::Log("[FontManager] ERRO CRITICO durante alocacao de glyphs: " + std::string(e.what()));
        throw;
    } catch (...) {
        Core::Log("[FontManager] ERRO CRITICO desconhecido durante alocacao de glyphs!");
        throw;
    }
    
    // Adicionar ao cache
    FontKey key{"embedded_default", size, quality};
    m_Fonts[key] = font;
    
    Core::Log("[FontManager] Fonte embutida padrao criada com sucesso!");
    Core::Log("[FontManager]   - Glyphs criados: " + std::to_string(font->m_Glyphs.size()));
    Core::Log("[FontManager]   - Atlas: " + std::to_string(config.width) + "x" + std::to_string(config.height));
    LOG_INFO("Created embedded default font (size: " + std::to_string(size) + ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    
    return font;
}

void FontManager::BeginTextRendering() {
    Core::Log("[FontManager] Iniciando renderização de texto...");
    m_IsRendering = true;
}

void FontManager::EndTextRendering() {
    Core::Log("[FontManager] Finalizando renderização de texto...");
    m_IsRendering = false;
}

size_t FontManager::GetLoadedFontCount() const {
    size_t count = m_Fonts.size();
    Core::Log("[FontManager] Total de fontes carregadas: " + std::to_string(count));
    return count;
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
    
    Core::Log("[FontManager] Nomes das fontes carregadas:");
    for (const auto& name : names) {
        Core::Log("[FontManager]   - " + name);
    }
    
    return names;
}


} // namespace Drift::UI 