#include "Drift/UI/FontSystem/Font.h"
#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/UI/FontSystem/FontMetrics.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Profiler.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Texture.h"
#include <stb_truetype.h>
#include <fstream>
#include <algorithm>
#include <cstring>

namespace Drift::UI {

// Constantes
const std::vector<std::string> FontLoader::s_SupportedExtensions = {
    ".ttf", ".otf", ".woff", ".woff2"
};

Font::Font(const std::string& name, const FontLoadConfig& config)
    : m_Name(name)
    , m_Config(config)
    , m_Status(Drift::Core::Assets::AssetStatus::NotLoaded)
    , m_LoadTime(std::chrono::steady_clock::now())
    , m_AccessCount(0)
    , m_IsValid(false) {
    
    DRIFT_PROFILE_FUNCTION();
    
    // Inicializar font info
    m_FontInfo = std::make_unique<stbtt_fontinfo>();
    
    // Criar atlas
    CreateAtlas();
}

Font::~Font() {
    DRIFT_PROFILE_FUNCTION();
    
    Unload();
}

size_t Font::GetMemoryUsage() const {
    size_t usage = 0;
    
    // Dados da fonte
    usage += m_FontData.size();
    
    // Glyphs
    usage += m_Glyphs.size() * sizeof(GlyphInfo);
    
    // Cache de kerning
    usage += m_KerningCache.size() * sizeof(std::pair<uint64_t, float>);
    
    // Atlas
    if (m_Atlas) {
        usage += m_Atlas->GetMemoryUsage();
    }
    
    return usage;
}

bool Font::Load() {
    DRIFT_PROFILE_FUNCTION();
    
    if (m_Status == Drift::Core::Assets::AssetStatus::Loaded) {
        return true;
    }
    
    m_Status = Drift::Core::Assets::AssetStatus::Loading;
    
    // Tentar carregar do arquivo
    if (!m_Path.empty() && LoadFromFile(m_Path)) {
        m_Status = Drift::Core::Assets::AssetStatus::Loaded;
        m_LoadTime = std::chrono::steady_clock::now();
        return true;
    }
    
    m_Status = Drift::Core::Assets::AssetStatus::Failed;
    DRIFT_LOG_ERROR("Falha ao carregar fonte: {}", m_Name);
    return false;
}

void Font::Unload() {
    DRIFT_PROFILE_FUNCTION();
    
    m_FontData.clear();
    m_FontInfo.reset();
    m_Glyphs.clear();
    m_KerningCache.clear();
    m_Atlas.reset();
    m_IsValid = false;
    m_Status = Drift::Core::Assets::AssetStatus::NotLoaded;
}

bool Font::LoadFromFile(const std::string& path) {
    DRIFT_PROFILE_FUNCTION();
    
    m_Path = path;
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        DRIFT_LOG_ERROR("Não foi possível abrir arquivo de fonte: {}", path);
        return false;
    }
    
    // Ler dados do arquivo
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    m_FontData.resize(fileSize);
    file.read(reinterpret_cast<char*>(m_FontData.data()), fileSize);
    
    return InitializeFontInfo();
}

bool Font::LoadFromMemory(const unsigned char* data, size_t size) {
    DRIFT_PROFILE_FUNCTION();
    
    m_FontData.assign(data, data + size);
    return InitializeFontInfo();
}

bool Font::LoadFromAsset(const std::string& assetPath) {
    DRIFT_PROFILE_FUNCTION();
    
    // Usar AssetsSystem para carregar
    auto asset = Drift::Core::Assets::AssetsSystem::GetInstance().LoadAsset(assetPath);
    if (!asset) {
        DRIFT_LOG_ERROR("Falha ao carregar asset de fonte: {}", assetPath);
        return false;
    }
    
    // Converter para dados de fonte
    auto data = asset->GetData();
    if (!data) {
        DRIFT_LOG_ERROR("Asset de fonte não contém dados válidos: {}", assetPath);
        return false;
    }
    
    return LoadFromMemory(data->data(), data->size());
}

bool Font::InitializeFontInfo() {
    DRIFT_PROFILE_FUNCTION();
    
    if (m_FontData.empty()) {
        return false;
    }
    
    // Inicializar STB TrueType
    if (!stbtt_InitFont(m_FontInfo.get(), m_FontData.data(), 0)) {
        DRIFT_LOG_ERROR("Falha ao inicializar fonte TTF: {}", m_Name);
        return false;
    }
    
    // Carregar informações da fonte
    if (!LoadFontInfo()) {
        return false;
    }
    
    // Carregar métricas
    if (!LoadFontMetrics()) {
        return false;
    }
    
    // Carregar caracteres pré-definidos
    for (uint32_t codepoint : m_Config.preloadChars) {
        LoadGlyph(codepoint);
    }
    
    m_IsValid = true;
    return true;
}

bool Font::LoadFontInfo() {
    DRIFT_PROFILE_FUNCTION();
    
    // Obter nome da família
    int nameLength;
    const char* familyName = stbtt_GetFontNameString(m_FontInfo.get(), &nameLength, 
                                                    STBTT_PLATFORM_ID_MICROSOFT, 
                                                    STBTT_MS_EID_UNICODE_BMP, 
                                                    STBTT_MS_LANG_ENGLISH, 1);
    if (familyName && nameLength > 0) {
        m_FamilyName = std::string(familyName, nameLength);
    } else {
        m_FamilyName = "Unknown";
    }
    
    // Obter nome do estilo
    const char* styleName = stbtt_GetFontNameString(m_FontInfo.get(), &nameLength, 
                                                   STBTT_PLATFORM_ID_MICROSOFT, 
                                                   STBTT_MS_EID_UNICODE_BMP, 
                                                   STBTT_MS_LANG_ENGLISH, 2);
    if (styleName && nameLength > 0) {
        m_StyleName = std::string(styleName, nameLength);
    } else {
        m_StyleName = "Regular";
    }
    
    // Detectar propriedades da fonte
    int isBold, isItalic, isMonospace;
    stbtt_GetFontVMetrics(m_FontInfo.get(), nullptr, nullptr, nullptr);
    
    // Verificar se é monospace (aproximação)
    m_IsMonospace = false; // TODO: Implementar detecção real
    
    return true;
}

bool Font::LoadFontMetrics() {
    DRIFT_PROFILE_FUNCTION();
    
    float scale = GetScale();
    
    // Métricas verticais
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(m_FontInfo.get(), &ascent, &descent, &lineGap);
    
    m_Metrics.ascent = ascent * scale;
    m_Metrics.descent = descent * scale;
    m_Metrics.lineGap = lineGap * scale;
    m_Metrics.lineHeight = (ascent - descent + lineGap) * scale;
    
    // Altura x e cap height
    int xHeight, capHeight;
    stbtt_GetFontVMetricsOS2(m_FontInfo.get(), &xHeight, &capHeight);
    
    m_Metrics.xHeight = xHeight * scale;
    m_Metrics.capHeight = capHeight * scale;
    
    // Métricas de sublinhado
    int underlinePosition, underlineThickness;
    stbtt_GetFontVMetricsOS2(m_FontInfo.get(), &underlinePosition, &underlineThickness);
    
    m_Metrics.underlinePosition = underlinePosition * scale;
    m_Metrics.underlineThickness = underlineThickness * scale;
    
    // Calcular larguras médias
    CalculateAverageWidths();
    
    return true;
}

void Font::CalculateAverageWidths() {
    DRIFT_PROFILE_FUNCTION();
    
    float scale = GetScale();
    float totalWidth = 0.0f;
    int charCount = 0;
    
    // Calcular largura média usando caracteres ASCII básicos
    for (int i = 32; i <= 126; ++i) { // Espaço até tilde
        int advance, leftBearing;
        stbtt_GetCodepointHMetrics(m_FontInfo.get(), i, &advance, &leftBearing);
        totalWidth += advance * scale;
        charCount++;
    }
    
    if (charCount > 0) {
        m_Metrics.avgCharWidth = totalWidth / charCount;
    }
    
    // Encontrar larguras mínima e máxima
    m_Metrics.minCharWidth = std::numeric_limits<float>::max();
    m_Metrics.maxCharWidth = 0.0f;
    
    for (int i = 32; i <= 126; ++i) {
        int advance, leftBearing;
        stbtt_GetCodepointHMetrics(m_FontInfo.get(), i, &advance, &leftBearing);
        float width = advance * scale;
        m_Metrics.minCharWidth = std::min(m_Metrics.minCharWidth, width);
        m_Metrics.maxCharWidth = std::max(m_Metrics.maxCharWidth, width);
    }
}

bool Font::CreateAtlas() {
    DRIFT_PROFILE_FUNCTION();
    
    FontAtlasConfig atlasConfig;
    atlasConfig.width = GetAtlasSize();
    atlasConfig.height = GetAtlasSize();
    atlasConfig.renderType = GlyphRenderType::MSDF;
    atlasConfig.padding = 2;
    atlasConfig.border = 1;
    
    // Obter dispositivo do sistema
    auto device = GetFontSystemDevice();
    if (!device) {
        DRIFT_LOG_ERROR("Dispositivo RHI não disponível para criar atlas");
        return false;
    }
    
    m_Atlas = std::make_shared<FontAtlas>(device, atlasConfig);
    return true;
}

const GlyphInfo* Font::GetGlyph(uint32_t codepoint) const {
    auto it = m_Glyphs.find(codepoint);
    if (it != m_Glyphs.end()) {
        return &it->second;
    }
    
    // Tentar carregar o glyph
    if (const_cast<Font*>(this)->LoadGlyph(codepoint)) {
        it = m_Glyphs.find(codepoint);
        return &it->second;
    }
    
    // Usar fallback se disponível
    if (m_FallbackFont) {
        return m_FallbackFont->GetGlyph(GetFallbackCodepoint(codepoint));
    }
    
    return nullptr;
}

bool Font::HasGlyph(uint32_t codepoint) const {
    return m_Glyphs.find(codepoint) != m_Glyphs.end();
}

bool Font::LoadGlyph(uint32_t codepoint) {
    DRIFT_PROFILE_FUNCTION();
    
    if (HasGlyph(codepoint)) {
        return true;
    }
    
    if (m_Glyphs.size() >= MAX_GLYPHS) {
        DRIFT_LOG_WARNING("Limite de glyphs atingido para fonte: {}", m_Name);
        return false;
    }
    
    return LoadGlyphInternal(codepoint);
}

bool Font::LoadGlyphInternal(uint32_t codepoint) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!m_IsValid || !m_FontInfo) {
        return false;
    }
    
    float scale = GetScale();
    
    // Obter métricas do glyph
    int advance, leftBearing;
    stbtt_GetCodepointHMetrics(m_FontInfo.get(), codepoint, &advance, &leftBearing);
    
    // Obter bounding box
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(m_FontInfo.get(), codepoint, scale, scale, &x0, &y0, &x1, &y1);
    
    if (x1 <= x0 || y1 <= y0) {
        // Glyph vazio, criar entrada mínima
        GlyphInfo info;
        info.advance = advance * scale;
        info.size = glm::vec2(0.0f);
        info.bearing = glm::vec2(0.0f);
        info.isLoaded = true;
        
        m_Glyphs[codepoint] = info;
        return true;
    }
    
    // Renderizar glyph
    int width = x1 - x0;
    int height = y1 - y0;
    
    std::vector<unsigned char> bitmap(width * height);
    stbtt_MakeCodepointBitmap(m_FontInfo.get(), bitmap.data(), width, height, width, scale, scale, codepoint);
    
    // Criar informações do glyph
    GlyphInfo info;
    info.size = glm::vec2(width, height);
    info.bearing = glm::vec2(leftBearing * scale, y0);
    info.advance = advance * scale;
    info.leftBearing = leftBearing * scale;
    info.rightBearing = (advance - leftBearing - width) * scale;
    info.topBearing = y0;
    info.bottomBearing = y1;
    info.renderType = GlyphRenderType::MSDF;
    info.isLoaded = true;
    
    // Adicionar ao atlas
    if (m_Atlas && m_Atlas->AddGlyph(codepoint, bitmap, width, height, info)) {
        // Obter coordenadas UV do atlas
        const GlyphInfo* atlasInfo = m_Atlas->GetGlyph(codepoint);
        if (atlasInfo) {
            info.uv0 = atlasInfo->uv0;
            info.uv1 = atlasInfo->uv1;
        }
    }
    
    m_Glyphs[codepoint] = info;
    return true;
}

float Font::GetKerning(uint32_t left, uint32_t right) const {
    if (!m_Config.enableKerning || !m_IsValid) {
        return 0.0f;
    }
    
    uint64_t key = MakeKerningKey(left, right);
    
    auto it = m_KerningCache.find(key);
    if (it != m_KerningCache.end()) {
        return it->second;
    }
    
    // Calcular kerning
    float kerning = stbtt_GetCodepointKernAdvance(m_FontInfo.get(), left, right) * GetScale();
    
    // Cache kerning
    if (m_KerningCache.size() < KERNING_CACHE_SIZE) {
        m_KerningCache[key] = kerning;
    }
    
    return kerning;
}

std::shared_ptr<Drift::RHI::ITexture> Font::GetAtlasTexture() const {
    if (m_Atlas) {
        return m_Atlas->GetTexture();
    }
    return nullptr;
}

uint32_t Font::GetFallbackCodepoint(uint32_t codepoint) const {
    // Mapeamento básico de fallback
    switch (codepoint) {
        case 0x00A0: return 0x0020; // Non-breaking space -> space
        case 0x2018: return 0x0027; // Left single quotation mark -> apostrophe
        case 0x2019: return 0x0027; // Right single quotation mark -> apostrophe
        case 0x201C: return 0x0022; // Left double quotation mark -> quote
        case 0x201D: return 0x0022; // Right double quotation mark -> quote
        case 0x2013: return 0x002D; // En dash -> hyphen
        case 0x2014: return 0x002D; // Em dash -> hyphen
        default: return 0x003F;     // Question mark como fallback geral
    }
}

int Font::GetAtlasSize() const {
    switch (m_Config.quality) {
        case FontQuality::Low: return 256;
        case FontQuality::Medium: return 512;
        case FontQuality::High: return 1024;
        case FontQuality::Ultra: return 2048;
        default: return 1024;
    }
}

float Font::GetScale() const {
    return stbtt_ScaleForPixelHeight(m_FontInfo.get(), m_Config.size);
}

uint64_t Font::MakeKerningKey(uint32_t left, uint32_t right) const {
    return (static_cast<uint64_t>(left) << 32) | right;
}

// FontLoader Implementation
FontLoader::FontLoader(Drift::RHI::IDevice* device)
    : m_Device(device) {
}

FontLoader::~FontLoader() = default;

std::shared_ptr<Font> FontLoader::Load(const std::string& path, const std::any& params) {
    DRIFT_PROFILE_FUNCTION();
    
    FontLoadConfig config = ParseLoadParams(params);
    auto font = std::make_shared<Font>(GetFontNameFromPath(path), config);
    
    if (font->LoadFromFile(path)) {
        return font;
    }
    
    return nullptr;
}

bool FontLoader::CanLoad(const std::string& path) const {
    return IsValidFontFile(path);
}

std::vector<std::string> FontLoader::GetSupportedExtensions() const {
    return s_SupportedExtensions;
}

std::string FontLoader::GetLoaderName() const {
    return "FontLoader";
}

size_t FontLoader::EstimateMemoryUsage(const std::string& path) const {
    // Estimativa baseada no tamanho do arquivo
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        size_t fileSize = file.tellg();
        // Estimativa: 3x o tamanho do arquivo para dados processados
        return fileSize * 3;
    }
    return 1024 * 1024; // 1MB como fallback
}

FontLoadConfig FontLoader::ParseLoadParams(const std::any& params) const {
    FontLoadConfig config;
    
    if (params.has_value()) {
        try {
            config = std::any_cast<FontLoadConfig>(params);
        } catch (const std::bad_any_cast&) {
            DRIFT_LOG_WARNING("Parâmetros inválidos para FontLoader");
        }
    }
    
    return config;
}

bool FontLoader::IsValidFontFile(const std::string& path) const {
    std::string extension = path.substr(path.find_last_of('.'));
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    return std::find(s_SupportedExtensions.begin(), s_SupportedExtensions.end(), extension) 
           != s_SupportedExtensions.end();
}

std::string FontLoader::GetFontNameFromPath(const std::string& path) const {
    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of('.');
    
    if (lastSlash == std::string::npos) {
        lastSlash = 0;
    } else {
        lastSlash++;
    }
    
    if (lastDot == std::string::npos) {
        lastDot = path.length();
    }
    
    return path.substr(lastSlash, lastDot - lastSlash);
}

} // namespace Drift::UI 