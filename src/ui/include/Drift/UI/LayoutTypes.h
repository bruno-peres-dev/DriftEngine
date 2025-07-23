#pragma once

#include <cstdint>

namespace Drift::UI {

    // Tipos de layout disponíveis
    enum class LayoutType {
        None,           // Posicionamento manual
        Horizontal,     // Elementos alinhados horizontalmente
        Vertical,       // Elementos alinhados verticalmente
        Grid,           // Layout em grade
        Flow            // Layout de fluxo (wrap)
    };

    // Alinhamento horizontal
    enum class HorizontalAlignment {
        Left,
        Center,
        Right,
        Stretch
    };

    // Alinhamento vertical
    enum class VerticalAlignment {
        Top,
        Center,
        Bottom,
        Stretch
    };

    // Margens e padding
    struct LayoutMargins {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
        
        LayoutMargins() = default;
        LayoutMargins(float uniform) : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
        LayoutMargins(float horizontal, float vertical) : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
        LayoutMargins(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
    };

    // Configuração de layout para um elemento
    struct LayoutConfig {
        LayoutType type = LayoutType::None;
        HorizontalAlignment horizontalAlign = HorizontalAlignment::Left;
        VerticalAlignment verticalAlign = VerticalAlignment::Top;
        LayoutMargins margin;
        LayoutMargins padding;
        float spacing = 0.0f;  // Espaçamento entre elementos filhos
        bool expandToFill = false;  // Se deve expandir para preencher o espaço disponível
    };

    // Dimensões de um elemento
    struct LayoutRect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        
        LayoutRect() = default;
        LayoutRect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
        
        float GetRight() const { return x + width; }
        float GetBottom() const { return y + height; }
        void SetRight(float right) { width = right - x; }
        void SetBottom(float bottom) { height = bottom - y; }
    };

} // namespace Drift::UI 