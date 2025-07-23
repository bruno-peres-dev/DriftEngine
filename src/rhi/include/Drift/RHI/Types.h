#pragma once
#include "Drift/RHI/Format.h"
#include <cstdint>

namespace Drift::RHI {

using UINT = unsigned int;
using INT = int;

// Topologias de primitivos para renderização
enum class PrimitiveTopology {
    Undefined,
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip
};

} // namespace Drift::RHI
