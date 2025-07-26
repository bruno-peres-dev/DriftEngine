#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Device.h"

namespace Drift::UI {

enum class FontQuality { Low, Medium, High, Ultra };

struct GlyphInfo {
    glm::vec2 uv0{0.0f};
    glm::vec2 uv1{0.0f};
    glm::vec2 size{0.0f};
    glm::vec2 bearing{0.0f};
    float advance{0.0f};
};

class Font {
public:
    Font(std::string name, float size, FontQuality quality);
    bool LoadFromFile(const std::string& path, Drift::RHI::IDevice* device);

    const GlyphInfo* GetGlyph(uint32_t codepoint) const;
    std::shared_ptr<Drift::RHI::ITexture> GetAtlasTexture() const { return m_Texture; }

    float GetSize() const { return m_Size; }
    const std::string& GetName() const { return m_Name; }

private:
    std::string m_Name;
    float m_Size;
    FontQuality m_Quality;
    std::unordered_map<uint32_t, GlyphInfo> m_Glyphs;
    std::shared_ptr<Drift::RHI::ITexture> m_Texture;
};

} // namespace Drift::UI
