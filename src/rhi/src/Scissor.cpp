#include "Drift/RHI/Scissor.h"
#include <algorithm>

namespace Drift::RHI {

bool ScissorRect::Intersects(const ScissorRect& other) const {
    return !(GetRight() <= other.GetLeft() || other.GetRight() <= GetLeft() ||
             GetBottom() <= other.GetTop() || other.GetBottom() <= GetTop());
}

ScissorRect ScissorRect::Clip(const ScissorRect& other) const {
    if (!Intersects(other)) {
        return ScissorRect(); // Retorna retângulo inválido
    }
    
    float clipX = std::max(GetLeft(), other.GetLeft());
    float clipY = std::max(GetTop(), other.GetTop());
    float clipWidth = std::min(GetRight(), other.GetRight()) - clipX;
    float clipHeight = std::min(GetBottom(), other.GetBottom()) - clipY;
    
    return ScissorRect(clipX, clipY, clipWidth, clipHeight);
}

} // namespace Drift::RHI 