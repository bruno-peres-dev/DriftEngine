#include "Drift/UI/Transform2D.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Drift::UI;

glm::mat4 Transform2D::ToMatrix() const {
    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(position, 0.0f));
    m = glm::rotate(m, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::scale(m, glm::vec3(scale, 1.0f));
    return m;
}
