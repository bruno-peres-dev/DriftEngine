// src/rhi/include/Drift/RHI/Scissor.h
#pragma once
#include <algorithm>

namespace Drift::RHI {

// Estrutura para representar um retângulo de clipping
struct ScissorRect {
    float x, y, width, height;
    
    ScissorRect() : x(0), y(0), width(0), height(0) {}
    ScissorRect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    bool IsValid() const { return width > 0 && height > 0; }
    bool Intersects(const ScissorRect& other) const;
    ScissorRect Clip(const ScissorRect& other) const;
    
    // Utilitários
    float GetRight() const { return x + width; }
    float GetBottom() const { return y + height; }
    float GetLeft() const { return x; }
    float GetTop() const { return y; }
    
    // Verificar se um ponto está dentro do retângulo
    bool Contains(float px, float py) const {
        return px >= x && px < GetRight() && py >= y && py < GetBottom();
    }
    
    // Verificar se um retângulo está completamente dentro
    bool Contains(const ScissorRect& other) const {
        return other.x >= x && other.GetRight() <= GetRight() &&
               other.y >= y && other.GetBottom() <= GetBottom();
    }
};

} // namespace Drift::RHI 