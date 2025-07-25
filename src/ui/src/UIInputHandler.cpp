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
                // Tenta fazer cast para Button para chamar OnMouseLeave
                if (auto* button = dynamic_cast<Button*>(m_HoveredElement)) {
                    button->OnMouseLeave();
                }
            }
            
            // Entra no novo elemento
            m_HoveredElement = elementAtPosition;
            if (m_HoveredElement) {
                // Tenta fazer cast para Button para chamar OnMouseEnter
                if (auto* button = dynamic_cast<Button*>(m_HoveredElement)) {
                    button->OnMouseEnter();
                }
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
            // Tenta fazer cast para Button para chamar OnMouseDown
            if (auto* button = dynamic_cast<Button*>(m_PressedElement)) {
                button->OnMouseDown(currentMousePos);
            }
        }
    }
    
    // Mouse up
    if (isMouseLeftReleased) {
        if (m_PressedElement) {
            // Tenta fazer cast para Button para chamar OnMouseUp
            if (auto* button = dynamic_cast<Button*>(m_PressedElement)) {
                button->OnMouseUp(currentMousePos);
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
    
    return FindElementAtPosition(root, position);
}

UIElement* UIInputHandler::FindElementAtPosition(UIElement* element, const glm::vec2& position)
{
    if (!element) return nullptr;
    
    // Verifica se o ponto está dentro deste elemento
    if (IsPointInElement(element, position)) {
        // Busca recursivamente nos filhos (em ordem reversa para pegar o topo primeiro)
        auto& children = element->GetChildren();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            auto* childResult = FindElementAtPosition(it->get(), position);
            if (childResult) {
                return childResult;
            }
        }
        
        // Se não encontrou nenhum filho, retorna este elemento
        return element;
    }
    
    return nullptr;
}

bool UIInputHandler::IsPointInElement(const UIElement* element, const glm::vec2& point) const
{
    if (!element) return false;
    
    // Usa posição absoluta para considerar a hierarquia de elementos
    glm::vec2 absPos = element->GetAbsolutePosition();
    glm::vec2 size = element->GetSize();
    
    return point.x >= absPos.x && point.x <= absPos.x + size.x &&
           point.y >= absPos.y && point.y <= absPos.y + size.y;
} 