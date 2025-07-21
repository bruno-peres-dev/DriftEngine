#pragma once

#include "Drift/Engine/Input/InputTypes.h"
#include <memory>

namespace Drift::Engine::Input {

    // Interface abstrata para gerenciar input independente de plataforma
    class IInputManager {
    public:
        virtual ~IInputManager() = default;
        
        // Atualiza o estado do input (chamado uma vez por frame)
        virtual void Update() = 0;
        
        // Acesso aos frames de input
        virtual const InputFrame& GetCurrentFrame() const = 0;
        virtual const InputFrame& GetPreviousFrame() const = 0;
        
        // Configurações de mouse
        virtual void SetMouseLocked(bool locked) = 0;
        virtual bool IsMouseLocked() const = 0;
        virtual void SetMouseVisible(bool visible) = 0;
        virtual bool IsMouseVisible() const = 0;
        
        // Métodos de conveniência que usam current frame
        bool IsKeyDown(Key key) const { return GetCurrentFrame().IsKeyDown(key); }
        bool IsKeyPressed(Key key) const { return GetCurrentFrame().IsKeyPressed(key); }
        bool IsKeyReleased(Key key) const { return GetCurrentFrame().IsKeyReleased(key); }
        
        bool IsMouseButtonDown(MouseButton button) const { 
            return GetCurrentFrame().IsMouseButtonDown(button); 
        }
        bool IsMouseButtonPressed(MouseButton button) const { 
            return GetCurrentFrame().IsMouseButtonPressed(button); 
        }
        bool IsMouseButtonReleased(MouseButton button) const { 
            return GetCurrentFrame().IsMouseButtonReleased(button); 
        }
        
        glm::vec2 GetMousePosition() const { return GetCurrentFrame().mousePosition; }
        glm::vec2 GetMouseDelta() const { return GetCurrentFrame().mouseDelta; }
        glm::vec2 GetMouseScroll() const { return GetCurrentFrame().mouseScroll; }
        
    protected:
        // Helper para transições de estado
        static KeyState UpdateKeyState(KeyState current, bool isPressed) {
            if (isPressed) {
                return (current == KeyState::Released || current == KeyState::JustReleased) 
                       ? KeyState::Pressed : KeyState::Held;
            } else {
                return (current == KeyState::Pressed || current == KeyState::Held)
                       ? KeyState::JustReleased : KeyState::Released;
            }
        }
    };
    
    // Factory functions para diferentes plataformas
    std::unique_ptr<IInputManager> CreateGLFWInputManager(void* glfwWindow);
    // std::unique_ptr<IInputManager> CreateWin32InputManager(HWND hwnd);
    // std::unique_ptr<IInputManager> CreateSDLInputManager();

} // namespace Drift::Engine::Input 