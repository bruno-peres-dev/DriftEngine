#include "Drift/RHI/Format.h"
#include <unordered_map>
#include <stdexcept>

namespace Drift::RHI {

    const char* VertexFormatToString(VertexFormat format) {
        switch (format) {
            case VertexFormat::R32_FLOAT: return "R32_FLOAT";
            case VertexFormat::R32G32_FLOAT: return "R32G32_FLOAT";
            case VertexFormat::R32G32B32_FLOAT: return "R32G32B32_FLOAT";
            case VertexFormat::R32G32B32A32_FLOAT: return "R32G32B32A32_FLOAT";
            case VertexFormat::R32_UINT: return "R32_UINT";
            case VertexFormat::R32G32_UINT: return "R32G32_UINT";
            case VertexFormat::R32G32B32_UINT: return "R32G32B32_UINT";
            case VertexFormat::R32G32B32A32_UINT: return "R32G32B32A32_UINT";
            case VertexFormat::R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
            case VertexFormat::R8G8B8A8_SNORM: return "R8G8B8A8_SNORM";
            case VertexFormat::R16G16_UNORM: return "R16G16_UNORM";
            case VertexFormat::R16G16B16A16_UNORM: return "R16G16B16A16_UNORM";
            case VertexFormat::R10G10B10A2_UNORM: return "R10G10B10A2_UNORM";
            case VertexFormat::R11G11B10_FLOAT: return "R11G11B10_FLOAT";
            default: return "UNKNOWN";
        }
    }

    VertexFormat StringToVertexFormat(const char* str) {
        static const std::unordered_map<std::string, VertexFormat> lut = {
            {"R32_FLOAT", VertexFormat::R32_FLOAT},
            {"R32G32_FLOAT", VertexFormat::R32G32_FLOAT},
            {"R32G32B32_FLOAT", VertexFormat::R32G32B32_FLOAT},
            {"R32G32B32A32_FLOAT", VertexFormat::R32G32B32A32_FLOAT},
            {"R32_UINT", VertexFormat::R32_UINT},
            {"R32G32_UINT", VertexFormat::R32G32_UINT},
            {"R32G32B32_UINT", VertexFormat::R32G32B32_UINT},
            {"R32G32B32A32_UINT", VertexFormat::R32G32B32A32_UINT},
            {"R8G8B8A8_UNORM", VertexFormat::R8G8B8A8_UNORM},
            {"R8G8B8A8_SNORM", VertexFormat::R8G8B8A8_SNORM},
            {"R16G16_UNORM", VertexFormat::R16G16_UNORM},
            {"R16G16B16A16_UNORM", VertexFormat::R16G16B16A16_UNORM},
            {"R10G10B10A2_UNORM", VertexFormat::R10G10B10A2_UNORM},
            {"R11G11B10_FLOAT", VertexFormat::R11G11B10_FLOAT}
        };
        
        auto it = lut.find(str ? str : "");
        if (it != lut.end()) {
            return it->second;
        }
        
        throw std::runtime_error("Invalid vertex format string: " + std::string(str ? str : "null"));
    }

} // namespace Drift::RHI 