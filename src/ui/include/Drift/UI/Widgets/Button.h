#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Color.h"
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
    void SetText(const std::string& text) { 
        // Verificação de segurança para evitar strings inválidas
        if (!text.empty()) {
            m_Text = text; 
        } else {
            m_Text.clear(); // Garante que a string está vazia mas válida
        }
        MarkDirty(); 
    }
    const std::string& GetText() const { return m_Text; }

    void SetEnabled(bool enabled) { m_Enabled = enabled; UpdateState(); }
    bool IsEnabled() const { return m_Enabled; }

    // Callbacks
    void SetOnClick(std::function<void(const ButtonClickEvent&)> callback) { m_OnClick = callback; }
    void SetOnHover(std::function<void(const ButtonHoverEvent&)> callback) { m_OnHover = callback; }

    // Estados visuais
    void SetNormalColor(Drift::Color color) {
        m_NormalColor = color;
        UpdateState(); 
    }
    void SetHoverColor(Drift::Color color) {
        m_HoverColor = color;
        UpdateState(); 
    }
    void SetPressedColor(Drift::Color color) {
        m_PressedColor = color;
        UpdateState(); 
    }
    void SetDisabledColor(Drift::Color color) {
        m_DisabledColor = color;
        UpdateState(); 
    }

    // Overrides
    void Update(float deltaSeconds) override;
    void Render(Drift::RHI::IUIBatcher& batch) override;
    Drift::Color GetRenderColor() const override;

    // Eventos de mouse (override dos métodos virtuais da base)
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseDown(const glm::vec2& position) override;
    void OnMouseUp(const glm::vec2& position) override;
    void OnMouseClick(const glm::vec2& position) override;

    // Debug: Método público para verificar a cor atual
    Drift::Color GetCurrentColor() const;

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
    Drift::Color m_NormalColor{0xFF4A90E2};   // Azul
    Drift::Color m_HoverColor{0xFF357ABD};    // Azul escuro
    Drift::Color m_PressedColor{0xFF2E6DA4};  // Azul mais escuro
    Drift::Color m_DisabledColor{0xFFCCCCCC}; // Cinza

    // Constantes de cor comuns (ARGB format)
    static constexpr Drift::Color COLOR_RED = 0xFFFF0000;
    static constexpr Drift::Color COLOR_GREEN = 0xFF00FF00;
    static constexpr Drift::Color COLOR_BLUE = 0xFF0000FF;
    static constexpr Drift::Color COLOR_WHITE = 0xFFFFFFFF;
    static constexpr Drift::Color COLOR_BLACK = 0xFF000000;
    static constexpr Drift::Color COLOR_GRAY = 0xFF808080;
    static constexpr Drift::Color COLOR_TRANSPARENT = 0x00000000;
};

} // namespace Drift::UI 