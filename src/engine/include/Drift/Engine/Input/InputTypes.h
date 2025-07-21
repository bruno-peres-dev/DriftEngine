#pragma once

#include <glm/glm.hpp>

namespace Drift::Engine::Input {

    // Códigos de tecla padronizados (independente de plataforma)
    enum class Key : int {
        Unknown = -1,
        
        // Letras
        A = 0, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        
        // Números
        Num0, Num1, Num2, Num3, Num4, 
        Num5, Num6, Num7, Num8, Num9,
        
        // Teclas de função
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        
        // Modificadores
        LeftShift, RightShift, LeftCtrl, RightCtrl,
        LeftAlt, RightAlt, LeftSuper, RightSuper,
        
        // Navegação
        Up, Down, Left, Right,
        PageUp, PageDown, Home, End, Insert, Delete,
        
        // Especiais  
        Space, Enter, Backspace, Tab, Escape,
        CapsLock, ScrollLock, NumLock, PrintScreen,
        Pause, Menu,
        
        // Keypad numérico
        KP0, KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9,
        KPDecimal, KPDivide, KPMultiply, KPSubtract, KPAdd, KPEnter, KPEqual,
        
        // Símbolos comuns
        Semicolon, Equal, Comma, Minus, Period, Slash, GraveAccent,
        LeftBracket, Backslash, RightBracket, Apostrophe,
        
        MAX_KEYS
    };
    
    // Botões do mouse
    enum class MouseButton : int {
        Left = 0,
        Right,
        Middle,
        Button4,
        Button5,
        Button6,
        Button7,
        Button8,
        MAX_BUTTONS
    };
    
    // Estado de uma tecla/botão
    enum class KeyState : uint8_t {
        Released = 0,  // Não pressionado
        Pressed = 1,   // Pressionado neste frame
        Held = 2,      // Mantido pressionado
        JustReleased = 3 // Liberado neste frame
    };
    
    // Estrutura para armazenar estado completo do input
    struct InputFrame {
        // Estados das teclas
        KeyState keys[static_cast<int>(Key::MAX_KEYS)];
        
        // Estados dos botões do mouse
        KeyState mouseButtons[static_cast<int>(MouseButton::MAX_BUTTONS)];
        
        // Posição do mouse
        glm::vec2 mousePosition = {0.0f, 0.0f};
        glm::vec2 mouseDelta = {0.0f, 0.0f};
        
        // Scroll do mouse
        glm::vec2 mouseScroll = {0.0f, 0.0f};
        
        // Gamepad (futuro)
        // GamepadState gamepads[MAX_GAMEPADS];
        
        // Métodos de conveniência
        bool IsKeyDown(Key key) const {
            int idx = static_cast<int>(key);
            if (idx < 0 || idx >= static_cast<int>(Key::MAX_KEYS)) return false;
            return keys[idx] == KeyState::Pressed || keys[idx] == KeyState::Held;
        }
        
        bool IsKeyPressed(Key key) const {
            int idx = static_cast<int>(key);
            if (idx < 0 || idx >= static_cast<int>(Key::MAX_KEYS)) return false;
            return keys[idx] == KeyState::Pressed;
        }
        
        bool IsKeyReleased(Key key) const {
            int idx = static_cast<int>(key);
            if (idx < 0 || idx >= static_cast<int>(Key::MAX_KEYS)) return false;
            return keys[idx] == KeyState::JustReleased;
        }
        
        bool IsMouseButtonDown(MouseButton button) const {
            int idx = static_cast<int>(button);
            if (idx < 0 || idx >= static_cast<int>(MouseButton::MAX_BUTTONS)) return false;
            return mouseButtons[idx] == KeyState::Pressed || mouseButtons[idx] == KeyState::Held;
        }
        
        bool IsMouseButtonPressed(MouseButton button) const {
            int idx = static_cast<int>(button);
            if (idx < 0 || idx >= static_cast<int>(MouseButton::MAX_BUTTONS)) return false;
            return mouseButtons[idx] == KeyState::Pressed;
        }
        
        bool IsMouseButtonReleased(MouseButton button) const {
            int idx = static_cast<int>(button);
            if (idx < 0 || idx >= static_cast<int>(MouseButton::MAX_BUTTONS)) return false;
            return mouseButtons[idx] == KeyState::JustReleased;
        }
        
        // Inicialização
        InputFrame() {
            for (int i = 0; i < static_cast<int>(Key::MAX_KEYS); ++i) {
                keys[i] = KeyState::Released;
            }
            for (int i = 0; i < static_cast<int>(MouseButton::MAX_BUTTONS); ++i) {
                mouseButtons[i] = KeyState::Released;
            }
        }
    };

} // namespace Drift::Engine::Input 