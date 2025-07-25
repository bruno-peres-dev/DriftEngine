#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace Drift::UI {

struct Transform2D {
    glm::vec2 position{0.0f};
    glm::vec2 scale{1.0f, 1.0f};
    float rotation{0.0f};

};

} // namespace Drift::UI
