#pragma once

#include "Font.h"
#include <unordered_map>
#include <memory>

namespace Drift::UI {

class FontManager {
public:
    static FontManager& GetInstance();

    void SetDevice(Drift::RHI::IDevice* device) { m_Device = device; }
    void SetDefaultFontName(const std::string& name) { m_DefaultFontName = name; }
    void SetDefaultSize(float size) { m_DefaultSize = size; }
    void SetDefaultQuality(FontQuality q) { m_DefaultQuality = q; }

    std::shared_ptr<Font> LoadFont(const std::string& name, const std::string& path, float size, FontQuality quality);
    std::shared_ptr<Font> GetFont(const std::string& name, float size, FontQuality quality);
    std::shared_ptr<Font> GetFont(const std::string& name, float size);

private:
    FontManager() = default;
    struct FontKey {
        std::string name;
        float size;
        FontQuality quality;
    };
    struct KeyHash {
        size_t operator()(const FontKey& k) const noexcept {
            return std::hash<std::string>{}(k.name) ^ (std::hash<int>{}(int(k.size*10)) << 1) ^ (std::hash<int>{}(int(k.quality)) << 2);
        }
    };
    struct KeyEq {
        bool operator()(const FontKey& a, const FontKey& b) const noexcept {
            return a.name == b.name && a.size == b.size && a.quality == b.quality;
        }
    };

    std::unordered_map<FontKey, std::shared_ptr<Font>, KeyHash, KeyEq> m_Fonts;
    Drift::RHI::IDevice* m_Device{nullptr};
    std::string m_DefaultFontName{"default"};
    float m_DefaultSize{16.0f};
    FontQuality m_DefaultQuality{FontQuality::High};
};

} // namespace Drift::UI
