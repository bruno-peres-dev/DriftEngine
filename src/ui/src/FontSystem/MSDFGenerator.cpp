#include "Drift/UI/FontSystem/MSDFGenerator.h"
#include "Drift/Core/Log.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <string>

namespace Drift::UI {

MSDFGenerator::MSDFGenerator(const MSDFConfig& config)
    : m_Config(config) {}

MSDFGenerator::~MSDFGenerator() = default;

bool MSDFGenerator::GenerateFromGlyph(const void* fontData, uint32_t codepoint, MSDFData& output) {
    if (!fontData) {
        return false;
    }

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, reinterpret_cast<const unsigned char*>(fontData), 0)) {
        LOG_ERROR("stbtt_InitFont failed in MSDFGenerator");
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&info, static_cast<float>(m_Config.height));
    int w, h, xoff, yoff;
    unsigned char* sdf = stbtt_GetCodepointSDF(&info, scale, static_cast<int>(codepoint), m_Config.range,
                                              128, 1.0f, &w, &h, &xoff, &yoff);
    if (!sdf) {
        LOG_ERROR(std::string("stbtt_GetCodepointSDF failed for codepoint ") + std::to_string(codepoint));
        return false;
    }

    output.width = w;
    output.height = h;
    output.range = static_cast<float>(m_Config.range);
    size_t count = static_cast<size_t>(w) * static_cast<size_t>(h);
    output.red.resize(count);
    output.green.resize(count);
    output.blue.resize(count);
    output.alpha.resize(count);

    for (size_t i = 0; i < count; ++i) {
        float v = sdf[i] / 255.0f;
        output.red[i] = v;
        output.green[i] = v;
        output.blue[i] = v;
        output.alpha[i] = v;
    }

    stbtt_FreeSDF(sdf, nullptr);
    return true;
}

bool MSDFGenerator::GenerateFromContours(const std::vector<Contour>&, MSDFData&) {
    // Implementação simplificada não suporta contornos customizados
    return false;
}

bool MSDFGenerator::ConvertToRGBA8(const MSDFData& msdf, std::vector<uint8_t>& output) {
    if (msdf.red.empty()) {
        return false;
    }
    output.resize(msdf.red.size() * 4);
    for (size_t i = 0; i < msdf.red.size(); ++i) {
        output[i * 4 + 0] = static_cast<uint8_t>(std::clamp(msdf.red[i], 0.0f, 1.0f) * 255.0f);
        output[i * 4 + 1] = static_cast<uint8_t>(std::clamp(msdf.green[i], 0.0f, 1.0f) * 255.0f);
        output[i * 4 + 2] = static_cast<uint8_t>(std::clamp(msdf.blue[i], 0.0f, 1.0f) * 255.0f);
        output[i * 4 + 3] = static_cast<uint8_t>(std::clamp(msdf.alpha[i], 0.0f, 1.0f) * 255.0f);
    }
    return true;
}

bool MSDFGenerator::ConvertToRGBA32F(const MSDFData& msdf, std::vector<float>& output) {
    if (msdf.red.empty()) {
        return false;
    }
    output.resize(msdf.red.size() * 4);
    for (size_t i = 0; i < msdf.red.size(); ++i) {
        output[i * 4 + 0] = msdf.red[i];
        output[i * 4 + 1] = msdf.green[i];
        output[i * 4 + 2] = msdf.blue[i];
        output[i * 4 + 3] = msdf.alpha[i];
    }
    return true;
}

void MSDFGenerator::ApplyAntiAliasing(MSDFData& msdf, float smoothing) {
    for (auto& v : msdf.alpha) {
        v = std::clamp(v / (1.0f + smoothing), 0.0f, 1.0f);
    }
}

void MSDFGenerator::ApplySubpixelRendering(MSDFData& msdf, float gamma) {
    float invGamma = 1.0f / gamma;
    for (auto& v : msdf.red) v = std::pow(v, invGamma);
    for (auto& v : msdf.green) v = std::pow(v, invGamma);
    for (auto& v : msdf.blue) v = std::pow(v, invGamma);
}

void MSDFGenerator::ApplyContrastEnhancement(MSDFData& msdf, float contrast) {
    for (auto& v : msdf.alpha) {
        v = std::clamp(v * (1.0f + contrast), 0.0f, 1.0f);
    }
}

FontProcessor::FontProcessor() : m_Size(32.0f), m_Hinting(true), m_Kerning(true) {}
FontProcessor::~FontProcessor() = default;

struct FontProcessor::FontData {
    stbtt_fontinfo info;
    std::vector<unsigned char> buffer;
    float ascender{};
    float descender{};
    float lineHeight{};
};

bool FontProcessor::LoadFont(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        LOG_ERROR(std::string("Failed to open font file ") + filePath);
        return false;
    }
    file.seekg(0, std::ios::end);
    size_t len = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    m_FontData = std::make_unique<FontData>();
    m_FontData->buffer.resize(len);
    file.read(reinterpret_cast<char*>(m_FontData->buffer.data()), len);
    file.close();
    return InitializeFont();
}

bool FontProcessor::LoadFontFromMemory(const void* data, size_t dataSize) {
    m_FontData = std::make_unique<FontData>();
    m_FontData->buffer.resize(dataSize);
    std::memcpy(m_FontData->buffer.data(), data, dataSize);
    return InitializeFont();
}

bool FontProcessor::InitializeFont() {
    if (!m_FontData) return false;
    if (!stbtt_InitFont(&m_FontData->info, m_FontData->buffer.data(), 0)) {
        LOG_ERROR("stbtt_InitFont failed in FontProcessor"); // já compatível
        return false;
    }
    float scale = stbtt_ScaleForPixelHeight(&m_FontData->info, m_Size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_FontData->info, &ascent, &descent, &lineGap);
    m_FontData->ascender = ascent * scale;
    m_FontData->descender = -descent * scale;
    m_FontData->lineHeight = (ascent - descent + lineGap) * scale;
    return true;
}

bool FontProcessor::ExtractGlyph(uint32_t codepoint, std::vector<Contour>&) {
    // Contour extraction not implemented in simplified version
    return false;
}

bool FontProcessor::ExtractGlyphMetrics(uint32_t codepoint, float& width, float& height,
                                        float& bearingX, float& bearingY, float& advance) {
    if (!m_FontData) return false;
    int glyph = stbtt_FindGlyphIndex(&m_FontData->info, static_cast<int>(codepoint));
    int ax, lsb;
    stbtt_GetGlyphHMetrics(&m_FontData->info, glyph, &ax, &lsb);
    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(&m_FontData->info, glyph, stbtt_ScaleForPixelHeight(&m_FontData->info, m_Size),
                            stbtt_ScaleForPixelHeight(&m_FontData->info, m_Size), &x0, &y0, &x1, &y1);
    width = static_cast<float>(x1 - x0);
    height = static_cast<float>(y1 - y0);
    bearingX = static_cast<float>(x0);
    bearingY = static_cast<float>(y0);
    advance = static_cast<float>(ax);
    return true;
}

float FontProcessor::GetAscender() const { return m_FontData ? m_FontData->ascender : 0.0f; }
float FontProcessor::GetDescender() const { return m_FontData ? m_FontData->descender : 0.0f; }
float FontProcessor::GetLineHeight() const { return m_FontData ? m_FontData->lineHeight : m_Size; }
float FontProcessor::GetBaseline() const { return GetAscender(); }

// Implementação do novo método accessor
const unsigned char* FontProcessor::GetFontBuffer() const {
    return m_FontData ? m_FontData->buffer.data() : nullptr;
}

FontProcessingPipeline::FontProcessingPipeline()
    : m_Processor(std::make_unique<FontProcessor>()),
      m_Generator(std::make_unique<MSDFGenerator>()),
      m_Quality(FontQuality::High), m_Smoothing(0.1f), m_Contrast(0.1f), m_Gamma(2.2f) {}

FontProcessingPipeline::~FontProcessingPipeline() = default;

bool FontProcessingPipeline::ProcessGlyph(uint32_t codepoint, MSDFData& output, const MSDFConfig& config) {
    m_Generator->SetConfig(config);
    if (!m_Generator || !m_Processor) return false;
    const unsigned char* buffer = m_Processor->GetFontBuffer();
    if (!buffer) return false;
    return m_Generator->GenerateFromGlyph(buffer, codepoint, output);
}

bool FontProcessingPipeline::ProcessFont(const std::string& fontPath, const std::string&, const MSDFConfig& config) {
    if (!m_Processor->LoadFont(fontPath)) {
        return false;
    }
    m_Generator->SetConfig(config);
    return true;
}

void FontProcessingPipeline::SetQuality(FontQuality quality) { m_Quality = quality; }
void FontProcessingPipeline::SetAntiAliasingSettings(float smoothing, float contrast, float gamma) {
    m_Smoothing = smoothing;
    m_Contrast = contrast;
    m_Gamma = gamma;
}

MSDFConfig FontProcessingPipeline::GetConfigForQuality(FontQuality quality) {
    MSDFConfig cfg;
    switch (quality) {
    case FontQuality::Low: cfg.range = 8; break;
    case FontQuality::Medium: cfg.range = 16; break;
    case FontQuality::High: cfg.range = 32; break;
    case FontQuality::Ultra: cfg.range = 64; break;
    }
    return cfg;
}

void FontProcessingPipeline::ApplyQualitySettings(MSDFData& msdf) {
    if (m_Smoothing > 0.0f) m_Generator->ApplyAntiAliasing(msdf, m_Smoothing);
    if (m_Contrast > 0.0f) m_Generator->ApplyContrastEnhancement(msdf, m_Contrast);
    if (m_Gamma > 0.0f) m_Generator->ApplySubpixelRendering(msdf, m_Gamma);
}

} // namespace Drift::UI

