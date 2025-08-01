#include "Drift/UI/UIInputHandler.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/Engine/Input/InputTypes.h"
#include "Drift/Engine/Input/InputManager.h"
#include "Drift/Core/Log.h"
#include <algorithm>

using namespace Drift::UI;
using namespace Drift::Engine::Input;

UIInputHandler::UIInputHandler(UIContext* context)
    : m_Context(context)
{
}

void UIInputHandler::SetInputManager(Drift::Engine::Input::IInputManager* inputManager)
{
    m_InputManager = inputManager;
}

void UIInputHandler::Update(float deltaSeconds)
{
    if (!m_InputManager) return;
    
    // Processa input do mouse
    ProcessMouseInput();
}

void UIInputHandler::ProcessMouseInput()
{
    if (!m_InputManager) return;
    
    // Obtém posição atual do mouse
    glm::vec2 currentMousePos = m_InputManager->GetMousePosition();
    
    // Detecta mudanças na posição do mouse
    if (currentMousePos != m_LastMousePosition) {
        UIElement* elementAtPosition = GetElementAtPosition(currentMousePos);
        
        // Atualiza elemento sob o mouse
        if (elementAtPosition != m_HoveredElement) {
            // Sai do elemento anterior
            if (m_HoveredElement) {
                m_HoveredElement->OnMouseLeave();
            }
            
            // Entra no novo elemento
            m_HoveredElement = elementAtPosition;
            if (m_HoveredElement) {
                m_HoveredElement->OnMouseEnter();
            }
        }
        
        m_LastMousePosition = currentMousePos;
    }
    
    // Detecta cliques do mouse
    bool isMouseLeftDown = m_InputManager->IsMouseButtonDown(MouseButton::Left);
    bool isMouseLeftPressed = m_InputManager->IsMouseButtonPressed(MouseButton::Left);
    bool isMouseLeftReleased = m_InputManager->IsMouseButtonReleased(MouseButton::Left);
    
    // Mouse down
    if (isMouseLeftPressed) {
        m_PressedElement = m_HoveredElement;
        if (m_PressedElement) {
            m_PressedElement->OnMouseDown(currentMousePos);
        }
    }
    
    // Mouse up
    if (isMouseLeftReleased) {
        if (m_PressedElement) {
            m_PressedElement->OnMouseUp(currentMousePos);
            
            // Se o mouse foi solto sobre o mesmo elemento que foi pressionado, é um clique
            if (m_PressedElement == m_HoveredElement) {
                m_PressedElement->OnMouseClick(currentMousePos);
            }
            
            m_PressedElement = nullptr;
        }
    }
    
    // Atualiza estados do frame anterior
    m_WasMouseLeftDown = isMouseLeftDown;
}

UIElement* UIInputHandler::GetElementAtPosition(const glm::vec2& position)
{
    auto* root = m_Context->GetRoot().get();
    if (!root) return nullptr;

    return m_Context->FindElementAtPosition(root, position);
}
