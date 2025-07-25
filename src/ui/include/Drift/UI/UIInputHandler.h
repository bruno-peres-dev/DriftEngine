#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIContext.h"
#include "Drift/Engine/Input/InputManager.h"
#include <memory>

namespace Drift::UI {

// Handler de input específico para UI que usa o sistema de input da Engine
class UIInputHandler {
public:
    explicit UIInputHandler(UIContext* context);
    ~UIInputHandler() = default;

    // Conecta ao sistema de input da Engine
    void SetInputManager(Drift::Engine::Input::IInputManager* inputManager);
    
    // Atualiza o estado do input (chamado a cada frame)
    void Update(float deltaSeconds);

    // Obtém o elemento sob uma posição específica
    UIElement* GetElementAtPosition(const glm::vec2& position);
    
    // Obtém o elemento atualmente sob o mouse
    UIElement* GetHoveredElement() const { return m_HoveredElement; }
    
    // Obtém o elemento que está sendo pressionado
    UIElement* GetPressedElement() const { return m_PressedElement; }

private:
    // Processa eventos de mouse usando o sistema de input da Engine
    void ProcessMouseInput();

    UIContext* m_Context;
    Drift::Engine::Input::IInputManager* m_InputManager{nullptr};
    
    UIElement* m_HoveredElement{nullptr};
    UIElement* m_PressedElement{nullptr};
    glm::vec2 m_LastMousePosition{0.0f, 0.0f};
    
    // Estados do frame anterior para detectar mudanças
    bool m_WasMouseLeftDown{false};
    bool m_WasMouseRightDown{false};
};

} // namespace Drift::UI 