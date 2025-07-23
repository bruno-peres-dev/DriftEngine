#include "Drift/RHI/Format.h"
#include <unordered_map>
#include <stdexcept>

namespace Drift::RHI {

const char* FormatToString(Format format) {
    switch (format) {
        case Format::Unknown: return "Unknown";
        case Format::R8_UNORM: return "R8_UNORM";
        case Format::R8G8_UNORM: return "R8G8_UNORM";
        case Format::R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
        case Format::R8G8B8A8_SNORM: return "R8G8B8A8_SNORM";
        case Format::R16_UINT: return "R16_UINT";
        case Format::R16G16_UNORM: return "R16G16_UNORM";
        case Format::R16G16B16A16_UNORM: return "R16G16B16A16_UNORM";
        case Format::R32_UINT: return "R32_UINT";
        case Format::R32G32_UINT: return "R32G32_UINT";
        case Format::R32G32B32_UINT: return "R32G32B32_UINT";
        case Format::R32G32B32A32_UINT: return "R32G32B32A32_UINT";
        case Format::R32_FLOAT: return "R32_FLOAT";
        case Format::R32G32_FLOAT: return "R32G32_FLOAT";
        case Format::R32G32B32_FLOAT: return "R32G32B32_FLOAT";
        case Format::R32G32B32A32_FLOAT: return "R32G32B32A32_FLOAT";
        case Format::R10G10B10A2_UNORM: return "R10G10B10A2_UNORM";
        case Format::R11G11B10_FLOAT: return "R11G11B10_FLOAT";
        case Format::D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
        case Format::BC1_UNORM: return "BC1_UNORM";
        case Format::BC3_UNORM: return "BC3_UNORM";
        default: return "Unknown";
    }
}

Format StringToFormat(const char* str) {
    static const std::unordered_map<std::string, Format> lut = {
        {"Unknown", Format::Unknown},
        {"R8_UNORM", Format::R8_UNORM},
        {"R8G8_UNORM", Format::R8G8_UNORM},
        {"R8G8B8A8_UNORM", Format::R8G8B8A8_UNORM},
        {"R8G8B8A8_SNORM", Format::R8G8B8A8_SNORM},
        {"R16_UINT", Format::R16_UINT},
        {"R16G16_UNORM", Format::R16G16_UNORM},
        {"R16G16B16A16_UNORM", Format::R16G16B16A16_UNORM},
        {"R32_UINT", Format::R32_UINT},
        {"R32G32_UINT", Format::R32G32_UINT},
        {"R32G32B32_UINT", Format::R32G32B32_UINT},
        {"R32G32B32A32_UINT", Format::R32G32B32A32_UINT},
        {"R32_FLOAT", Format::R32_FLOAT},
        {"R32G32_FLOAT", Format::R32G32_FLOAT},
        {"R32G32B32_FLOAT", Format::R32G32B32_FLOAT},
        {"R32G32B32A32_FLOAT", Format::R32G32B32A32_FLOAT},
        {"R10G10B10A2_UNORM", Format::R10G10B10A2_UNORM},
        {"R11G11B10_FLOAT", Format::R11G11B10_FLOAT},
        {"D24_UNORM_S8_UINT", Format::D24_UNORM_S8_UINT},
        {"BC1_UNORM", Format::BC1_UNORM},
        {"BC3_UNORM", Format::BC3_UNORM}
    };
    
    auto it = lut.find(str);
    if (it != lut.end()) {
        return it->second;
    }
    
    throw std::invalid_argument("Invalid format string: " + std::string(str));
}

} // namespace Drift::RHI 