#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/Core/Log.h"
#include <functional>
#include <string>
#include <glm/vec2.hpp>

namespace Drift::UI {

// Forward declaration
class Button;

// Estados visuais do botão
enum class ButtonState {
    Normal,
    Hover,
    Pressed,
    Disabled
};

// Eventos do botão
struct ButtonClickEvent {
    Button* button;
    glm::vec2 clickPosition;
};

struct ButtonHoverEvent {
    Button* button;
    bool isHovering;
};

class Button : public UIElement {
public:
    explicit Button(UIContext* context);
    ~Button() override = default;

    // Propriedades do botão
    void SetText(const std::string& text) { m_Text = text; MarkDirty(); }
    const std::string& GetText() const { return m_Text; }

    void SetEnabled(bool enabled) { m_Enabled = enabled; UpdateState(); }
    bool IsEnabled() const { return m_Enabled; }

    // Callbacks
    void SetOnClick(std::function<void(const ButtonClickEvent&)> callback) { m_OnClick = callback; }
    void SetOnHover(std::function<void(const ButtonHoverEvent&)> callback) { m_OnHover = callback; }

    // Estados visuais
    void SetNormalColor(unsigned color) { 
        m_NormalColor = color;
        UpdateState(); 
    }
    void SetHoverColor(unsigned color) { 
        m_HoverColor = color;
        UpdateState(); 
    }
    void SetPressedColor(unsigned color) { 
        m_PressedColor = color;
        UpdateState(); 
    }
    void SetDisabledColor(unsigned color) { 
        m_DisabledColor = color;
        UpdateState(); 
    }

    // Overrides
    void Update(float deltaSeconds) override;
    void Render(Drift::RHI::IUIBatcher& batch) override;
    unsigned GetRenderColor() const override;

    // Input handling
    void OnMouseEnter();
    void OnMouseLeave();
    void OnMouseDown(const glm::vec2& position);
    void OnMouseUp(const glm::vec2& position);

    // Debug: Método público para verificar a cor atual
    unsigned GetCurrentColor() const;

private:
    void UpdateState();

    // Propriedades
    std::string m_Text;
    bool m_Enabled{true};
    ButtonState m_CurrentState{ButtonState::Normal};
    bool m_IsHovered{false};
    bool m_IsPressed{false};

    // Callbacks
    std::function<void(const ButtonClickEvent&)> m_OnClick;
    std::function<void(const ButtonHoverEvent&)> m_OnHover;

    // Cores por estado (32 bits ARGB)
    unsigned m_NormalColor{0xFF4A90E2};   // Azul
    unsigned m_HoverColor{0xFF357ABD};    // Azul escuro
    unsigned m_PressedColor{0xFF2E6DA4};  // Azul mais escuro
    unsigned m_DisabledColor{0xFFCCCCCC}; // Cinza

    // Constantes de cor comuns (ARGB format)
    static constexpr unsigned COLOR_RED = 0xFFFF0000;
    static constexpr unsigned COLOR_GREEN = 0xFF00FF00;
    static constexpr unsigned COLOR_BLUE = 0xFF0000FF;
    static constexpr unsigned COLOR_WHITE = 0xFFFFFFFF;
    static constexpr unsigned COLOR_BLACK = 0xFF000000;
    static constexpr unsigned COLOR_GRAY = 0xFF808080;
    static constexpr unsigned COLOR_TRANSPARENT = 0x00000000;
};

} // namespace Drift::UI 