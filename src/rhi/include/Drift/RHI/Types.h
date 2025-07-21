#pragma once
#include <cstdint>

namespace Drift::RHI {

using UINT = unsigned int;
using INT = int;

// Formatos de dados suportados
enum class Format {
    Unknown,
    R8_UNORM,
    R8G8_UNORM,
    R8G8B8A8_UNORM,
    R16_UINT,
    R32_UINT,
    D24_UNORM_S8_UINT,
    BC1_UNORM,
    BC3_UNORM
};

// Topologias de primitivos
enum class PrimitiveTopology {
    Undefined,
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip
};

} // namespace Drift::RHI
