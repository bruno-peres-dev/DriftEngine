#pragma once

#include "Font.h"
#include <unordered_map>
#include <memory>
#include <mutex>
#include "Drift/RHI/Device.h"

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
    
    // Novos métodos otimizados
    void PreloadFontFile(const std::string& path);
    void ClearCache();
    size_t GetCacheSize() const;
    
    // Lazy loading de fontes
    std::shared_ptr<Font> GetOrLoadFont(const std::string& name, const std::string& path, float size, FontQuality quality = FontQuality::High);
    void PreloadCommonSizes(const std::string& name, const std::string& path, const std::vector<float>& sizes);

private:
    FontManager() = default;
    
    struct FontKey {
        std::string name;
        float size;
        FontQuality quality;
        
        bool operator==(const FontKey& other) const {
            return name == other.name && size == other.size && quality == other.quality;
        }
    };
    
    struct FontKeyHash {
        size_t operator()(const FontKey& k) const noexcept {
            return std::hash<std::string>{}(k.name) ^ 
                   (std::hash<int>{}(static_cast<int>(k.size * 10)) << 1) ^ 
                   (std::hash<int>{}(static_cast<int>(k.quality)) << 2);
        }
    };

    // Cache de dados TTF compartilhados
    struct TTFData {
        std::vector<unsigned char> data;
        std::string path;
        size_t refCount = 0;
    };
    
    std::unordered_map<std::string, std::shared_ptr<TTFData>> m_TTFDataCache;
    std::unordered_map<FontKey, std::shared_ptr<Font>, FontKeyHash> m_Fonts;
    
    mutable std::mutex m_Mutex;
    Drift::RHI::IDevice* m_Device{nullptr};
    std::string m_DefaultFontName{"default"};
    float m_DefaultSize{16.0f};
    FontQuality m_DefaultQuality{FontQuality::High};
    
    // Métodos auxiliares
    std::shared_ptr<TTFData> LoadTTFData(const std::string& path);
    void ReleaseTTFData(const std::string& path);
};

} // namespace Drift::UI
