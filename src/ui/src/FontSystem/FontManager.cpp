#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <filesystem>

namespace Drift::UI {

FontManager& FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

FontManager::FontManager() 
    : m_DefaultQuality(FontQuality::High)
    , m_DefaultSize(16.0f)
    , m_DefaultFontName("default")
    , m_IsRendering(false)
    , m_FrameCounter(0) {
    
    // Configurar cache padrão otimizado para AAA
    m_CacheConfig.maxFonts = 64;
    m_CacheConfig.maxGlyphsPerFont = 4096;
    m_CacheConfig.maxAtlasSize = 4096;
    m_CacheConfig.enablePreloading = true;
    m_CacheConfig.enableLazyLoading = true;
    m_CacheConfig.memoryBudgetMB = 256.0f;
    
    // Pré-alocar espaço no mapa para evitar realocações
    m_Fonts.reserve(m_CacheConfig.maxFonts);
    
    LOG_INFO("FontManager initialized with AAA optimizations");
}

FontManager::~FontManager() {
    UnloadAllFonts();
    LOG_INFO("FontManager destroyed");
}

std::shared_ptr<Font> FontManager::LoadFont(const std::string& name, const std::string& filePath, 
                                           float size, FontQuality quality) {
    // Verificar se a fonte já está carregada (cache hit)
    FontKey key{name, size, quality};
    
    {
        std::lock_guard<std::mutex> lock(m_FontMutex);
        auto it = m_Fonts.find(key);
        if (it != m_Fonts.end()) {
            m_Stats.cacheHits++;
            UpdateFontUsage(key);
            return it->second;
        }
    }
    
    m_Stats.cacheMisses++;
    
    // Verificar se o arquivo existe
    if (!std::filesystem::exists(filePath)) {
        LOG_ERROR("Font file not found: " + filePath);
        return nullptr;
    }
    
    // Criar nova fonte
    auto font = std::make_shared<Font>(name, filePath, size, quality);
    if (!font->Load(m_Device)) {
        LOG_ERROR("Failed to load font: " + filePath);
        return nullptr;
    }
    
    // Adicionar ao cache com thread safety
    {
        std::lock_guard<std::mutex> lock(m_FontMutex);
        
        // Verificar novamente se não foi carregada por outra thread
        auto it = m_Fonts.find(key);
        if (it != m_Fonts.end()) {
            m_Stats.cacheHits++;
            UpdateFontUsage(key);
            return it->second;
        }
        
        // Verificar limites de cache
        if (m_Fonts.size() >= m_CacheConfig.maxFonts) {
            TrimCache();
        }
        
        m_Fonts[key] = font;
        UpdateFontUsage(key);
        m_Stats.totalFonts = m_Fonts.size();
    }
    
    return font;
}

std::shared_ptr<Font> FontManager::GetFont(const std::string& name, float size, FontQuality quality) {
    FontKey key{name, size, quality};
    
    {
        std::lock_guard<std::mutex> lock(m_FontMutex);
        auto it = m_Fonts.find(key);
        if (it != m_Fonts.end()) {
            m_Stats.cacheHits++;
            UpdateFontUsage(key);
            return it->second;
        }
    }
    
    m_Stats.cacheMisses++;
    
    // Tentar carregar a fonte padrão se não encontrada
    if (name != m_DefaultFontName) {
        return GetFont(m_DefaultFontName, size, quality);
    }
    
    // Se não há fonte padrão carregada, criar uma fonte embutida simples
    if (m_Fonts.empty()) {
        LOG_INFO("No fonts loaded, creating embedded default font");
        return CreateEmbeddedDefaultFont(size, quality);
    }
    
    LOG_WARNING("Font not found: " + name + " (size: " + std::to_string(size) + 
                ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    return nullptr;
}

void FontManager::UnloadFont(const std::string& name, float size, FontQuality quality) {
    FontKey key{name, size, quality};
    
    std::lock_guard<std::mutex> lock(m_FontMutex);
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        m_Fonts.erase(it);
        m_Stats.totalFonts = m_Fonts.size();
            // Font unloaded silently
    }
}

void FontManager::UnloadAllFonts() {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    size_t count = m_Fonts.size();
    m_Fonts.clear();
    m_Stats.totalFonts = 0;
    // All fonts unloaded silently
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

void FontManager::SetCacheConfig(const FontCacheConfig& config) {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    m_CacheConfig = config;
    
    // Ajustar tamanho do mapa se necessário
    if (m_Fonts.size() > config.maxFonts) {
        TrimCache();
    }
    
    // Font cache config updated silently
}

void FontManager::PreloadFont(const std::string& name, const std::string& filePath, 
                             const std::vector<float>& sizes, FontQuality quality) {
    for (float size : sizes) {
        LoadFont(name, filePath, size, quality);
    }
}

void FontManager::PreloadCharacters(const std::string& fontName, const std::vector<uint32_t>& characters,
                                   float size, FontQuality quality) {
    auto font = GetFont(fontName, size, quality);
    if (font) {
        font->PreloadGlyphs(characters);
        LOG_INFO("Preloaded " + std::to_string(characters.size()) + " characters for font: " + fontName);
    } else {
        LOG_WARNING("Failed to preload characters: font not found: " + fontName);
    }
}

std::shared_ptr<Font> FontManager::CreateEmbeddedDefaultFont(float size, FontQuality quality) {
    LOG_INFO("Creating embedded default font (size: " + std::to_string(size) + 
             ", quality: " + std::to_string(static_cast<int>(quality)) + ")");
    
    // Criar uma fonte simples embutida com caracteres básicos
    auto font = std::make_shared<Font>("embedded_default", "", size, quality);
    
    // Configurar métricas básicas
    font->m_IsLoaded = true;
    font->m_Metrics.lineHeight = size * 1.2f;
    font->m_Metrics.ascender = size * 0.8f;
    font->m_Metrics.descender = -size * 0.2f;
    font->m_Scale = 1.0f;
    
    LOG_INFO("Creating embedded default font: métricas configuradas");
    
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
    
    LOG_INFO("Creating embedded default font: " + std::to_string(font->m_Glyphs.size()) + " glyphs criados");
    
    // Criar atlas simples
    AtlasConfig config;
    config.width = 512;
    config.height = 512;
    config.padding = 1;
    config.channels = 4;
    config.useMSDF = false; // Fonte simples não usa MSDF
    config.msdfSize = 32;
    
    font->m_Atlas = std::make_unique<FontAtlas>(config, m_Device);
    
    LOG_INFO("Creating embedded default font: atlas criado");
    
    // Alocar regiões no atlas para cada glyph
    for (auto& [cp, glyph] : font->m_Glyphs) {
        AtlasRegion* region = font->m_Atlas->AllocateRegion(
            static_cast<int>(glyph.size.x), 
            static_cast<int>(glyph.size.y), 
            cp
        );
        
        if (region) {
            glyph.position = glm::vec2(region->x, region->y);
            
            float atlasWidth = static_cast<float>(font->m_Atlas->GetWidth());
            float atlasHeight = static_cast<float>(font->m_Atlas->GetHeight());
            
            glyph.uvMin = glm::vec2(
                region->x / atlasWidth,
                region->y / atlasHeight
            );
            
            glyph.uvMax = glm::vec2(
                (region->x + region->width) / atlasWidth,
                (region->y + region->height) / atlasHeight
            );
            
            LOG_DEBUG("Glyph " + std::to_string(cp) + " alocado no atlas: (" + 
                     std::to_string(region->x) + ", " + std::to_string(region->y) + ")");
        } else {
            LOG_ERROR("Falha ao alocar região no atlas para glyph " + std::to_string(cp));
        }
    }
    
    LOG_INFO("Creating embedded default font: regiões do atlas alocadas");
    
    // Adicionar ao cache
    FontKey key{"embedded_default", size, quality};
    {
        std::lock_guard<std::mutex> lock(m_FontMutex);
        m_Fonts[key] = font;
        m_Stats.totalFonts = m_Fonts.size();
    }
    
    LOG_INFO("Created embedded default font with " + std::to_string(font->m_Glyphs.size()) + " glyphs");
    
    return font;
}

void FontManager::BeginTextRendering() {
    m_IsRendering = true;
    m_FrameCounter++;
}

void FontManager::EndTextRendering() {
    m_IsRendering = false;
    
    // Atualizar cache periodicamente
    if (m_FrameCounter % 60 == 0) { // A cada 60 frames
        UpdateCache();
    }
}

size_t FontManager::GetLoadedFontCount() const {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    return m_Fonts.size();
}

std::vector<std::string> FontManager::GetLoadedFontNames() const {
    std::lock_guard<std::mutex> lock(m_FontMutex);
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

bool FontManager::IsFontLoaded(const std::string& name, float size, FontQuality quality) const {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    FontKey key{name, size, quality};
    return m_Fonts.find(key) != m_Fonts.end();
}

void FontManager::UpdateCache() {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    
    // Calcular taxa de acerto do cache
    size_t totalAccesses = m_Stats.cacheHits + m_Stats.cacheMisses;
    if (totalAccesses > 0) {
        m_Stats.cacheHitRate = static_cast<float>(m_Stats.cacheHits) / static_cast<float>(totalAccesses);
    }
    
    // Atualizar uso de memória
    m_Stats.memoryUsageBytes = 0;
    m_Stats.totalGlyphs = 0;
    m_Stats.totalAtlases = 0;
    
    for (const auto& pair : m_Fonts) {
        m_Stats.memoryUsageBytes += CalculateFontMemoryUsage(pair.second);
        m_Stats.totalGlyphs += pair.second->m_Glyphs.size();
        if (pair.second->m_Atlas) {
            m_Stats.totalAtlases++;
        }
    }
    
    // Verificar orçamento de memória
    float memoryUsageMB = static_cast<float>(m_Stats.memoryUsageBytes) / (1024.0f * 1024.0f);
    if (memoryUsageMB > m_CacheConfig.memoryBudgetMB) {
        TrimCache();
    }
}

void FontManager::ClearCache() {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    m_Fonts.clear();
    m_Stats.Reset();
    LOG_INFO("Font cache cleared");
}

void FontManager::TrimCache() {
    if (m_Fonts.size() <= m_CacheConfig.maxFonts / 2) {
        return; // Não precisa fazer trim se está bem abaixo do limite
    }
    
    // Ordenar fontes por último uso (LRU)
    std::vector<std::pair<FontKey, std::shared_ptr<Font>>> sortedFonts;
    sortedFonts.reserve(m_Fonts.size());
    
    for (const auto& pair : m_Fonts) {
        sortedFonts.emplace_back(pair.first, pair.second);
    }
    
    std::sort(sortedFonts.begin(), sortedFonts.end(),
              [](const auto& a, const auto& b) {
                  return a.second->GetLastUsed() < b.second->GetLastUsed();
              });
    
    // Remover 25% das fontes menos usadas
    size_t toRemove = m_Fonts.size() / 4;
    for (size_t i = 0; i < toRemove; ++i) {
        m_Fonts.erase(sortedFonts[i].first);
    }
    
    m_Stats.totalFonts = m_Fonts.size();
    LOG_INFO("Font cache trimmed: removed " + std::to_string(toRemove) + " fonts");
}

FontStats FontManager::GetStats() const {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    return m_Stats;
}

void FontManager::ResetStats() {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    m_Stats.Reset();
}

void FontManager::UpdateFontUsage(const FontKey& key) {
    // Atualizar timestamp da fonte
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) {
        it->second->Touch();
    }
}

void FontManager::UpdateStats() {
    std::lock_guard<std::mutex> lock(m_FontMutex);
    
    // Calcular taxa de acerto do cache
    size_t totalAccesses = m_Stats.cacheHits + m_Stats.cacheMisses;
    if (totalAccesses > 0) {
        m_Stats.cacheHitRate = static_cast<float>(m_Stats.cacheHits) / static_cast<float>(totalAccesses);
    }
    
    // Atualizar uso de memória
    m_Stats.memoryUsageBytes = 0;
    m_Stats.totalGlyphs = 0;
    m_Stats.totalAtlases = 0;
    
    for (const auto& pair : m_Fonts) {
        m_Stats.memoryUsageBytes += CalculateFontMemoryUsage(pair.second);
        m_Stats.totalGlyphs += pair.second->m_Glyphs.size();
        if (pair.second->m_Atlas) {
            m_Stats.totalAtlases++;
        }
    }
}

size_t FontManager::CalculateFontMemoryUsage(const std::shared_ptr<Font>& font) const {
    if (!font) return 0;
    return font->GetMemoryUsage();
}

void FontManager::SetMemoryBudget(float budgetMB) {
    m_CacheConfig.memoryBudgetMB = budgetMB;
}

void FontManager::EnableAsyncLoading(bool enabled) {
    m_AsyncLoadingEnabled = enabled;
}

void FontManager::SetWorkerThreadCount(size_t count) {
    m_WorkerThreadCount = count;
}

void FontManager::SetDevice(Drift::RHI::IDevice* device) {
    m_Device = device;
    LOG_INFO("FontManager: device configurado");
}

// === Implementação dos utilitários TextUtils ===

namespace TextUtils {

glm::vec2 MeasureText(const std::string& text, const std::string& fontName, 
                     float size, FontQuality quality) {
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont(fontName, size, quality);
    
    if (font) {
        return font->MeasureText(text);
    }
    
    return glm::vec2(0.0f);
}

std::vector<std::string> WordWrap(const std::string& text, float maxWidth, 
                                 const std::string& fontName, 
                                 float size, FontQuality quality) {
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont(fontName, size, quality);
    
    if (!font) {
        return {text};
    }
    
    std::vector<std::string> lines;
    std::string currentLine;
    std::string currentWord;
    
    for (char c : text) {
        if (c == ' ' || c == '\n') {
            if (!currentWord.empty()) {
                std::string testLine = currentLine + currentWord;
                glm::vec2 size = font->MeasureText(testLine);
                
                if (size.x > maxWidth && !currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = currentWord;
                } else {
                    currentLine = testLine;
                }
                currentWord.clear();
            }
            
            if (c == '\n') {
                lines.push_back(currentLine);
                currentLine.clear();
            } else {
                currentLine += ' ';
            }
        } else {
            currentWord += c;
        }
    }
    
    // Adicionar última palavra
    if (!currentWord.empty()) {
        std::string testLine = currentLine + currentWord;
        glm::vec2 size = font->MeasureText(testLine);
        
        if (size.x > maxWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = currentWord;
        } else {
            currentLine = testLine;
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

std::string TruncateText(const std::string& text, float maxWidth, 
                        const std::string& fontName, 
                        float size, FontQuality quality) {
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont(fontName, size, quality);
    
    if (!font) {
        return text;
    }
    
    glm::vec2 textSize = font->MeasureText(text);
    if (textSize.x <= maxWidth) {
        return text;
    }
    
    // Busca binária para encontrar o ponto de truncamento
    size_t left = 0;
    size_t right = text.length();
    size_t best = 0;
    
    while (left <= right) {
        size_t mid = (left + right) / 2;
        std::string testText = text.substr(0, mid) + "...";
        glm::vec2 size = font->MeasureText(testText);
        
        if (size.x <= maxWidth) {
            best = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return text.substr(0, best) + "...";
}

std::vector<uint32_t> StringToCodepoints(const std::string& text) {
    std::vector<uint32_t> codepoints;
    codepoints.reserve(text.length());
    
    for (char c : text) {
        codepoints.push_back(static_cast<uint32_t>(c));
    }
    
    return codepoints;
}

std::string CodepointsToString(const std::vector<uint32_t>& codepoints) {
    std::string result;
    result.reserve(codepoints.size());
    
    for (uint32_t cp : codepoints) {
        if (cp <= 0x7F) { // ASCII
            result += static_cast<char>(cp);
        }
    }
    
    return result;
}

} // namespace TextUtils

} // namespace Drift::UI 