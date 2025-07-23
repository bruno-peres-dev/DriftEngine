#include "Drift/Engine/Input/InputManager.h"
#include "Drift/Core/Log.h"

#include <GLFW/glfw3.h>
#include <unordered_map>

namespace Drift::Engine::Input {

    // Mapeamento de teclas GLFW para nosso enum
    static const std::unordered_map<int, Key> glfwToKey = {
        // Letras
        {GLFW_KEY_A, Key::A}, {GLFW_KEY_B, Key::B}, {GLFW_KEY_C, Key::C},
        {GLFW_KEY_D, Key::D}, {GLFW_KEY_E, Key::E}, {GLFW_KEY_F, Key::F},
        {GLFW_KEY_G, Key::G}, {GLFW_KEY_H, Key::H}, {GLFW_KEY_I, Key::I},
        {GLFW_KEY_J, Key::J}, {GLFW_KEY_K, Key::K}, {GLFW_KEY_L, Key::L},
        {GLFW_KEY_M, Key::M}, {GLFW_KEY_N, Key::N}, {GLFW_KEY_O, Key::O},
        {GLFW_KEY_P, Key::P}, {GLFW_KEY_Q, Key::Q}, {GLFW_KEY_R, Key::R},
        {GLFW_KEY_S, Key::S}, {GLFW_KEY_T, Key::T}, {GLFW_KEY_U, Key::U},
        {GLFW_KEY_V, Key::V}, {GLFW_KEY_W, Key::W}, {GLFW_KEY_X, Key::X},
        {GLFW_KEY_Y, Key::Y}, {GLFW_KEY_Z, Key::Z},
        
        // Números
        {GLFW_KEY_0, Key::Num0}, {GLFW_KEY_1, Key::Num1}, {GLFW_KEY_2, Key::Num2},
        {GLFW_KEY_3, Key::Num3}, {GLFW_KEY_4, Key::Num4}, {GLFW_KEY_5, Key::Num5},
        {GLFW_KEY_6, Key::Num6}, {GLFW_KEY_7, Key::Num7}, {GLFW_KEY_8, Key::Num8},
        {GLFW_KEY_9, Key::Num9},
        
        // Teclas de função
        {GLFW_KEY_F1, Key::F1}, {GLFW_KEY_F2, Key::F2}, {GLFW_KEY_F3, Key::F3},
        {GLFW_KEY_F4, Key::F4}, {GLFW_KEY_F5, Key::F5}, {GLFW_KEY_F6, Key::F6},
        {GLFW_KEY_F7, Key::F7}, {GLFW_KEY_F8, Key::F8}, {GLFW_KEY_F9, Key::F9},
        {GLFW_KEY_F10, Key::F10}, {GLFW_KEY_F11, Key::F11}, {GLFW_KEY_F12, Key::F12},
        
        // Modificadores
        {GLFW_KEY_LEFT_SHIFT, Key::LeftShift}, {GLFW_KEY_RIGHT_SHIFT, Key::RightShift},
        {GLFW_KEY_LEFT_CONTROL, Key::LeftCtrl}, {GLFW_KEY_RIGHT_CONTROL, Key::RightCtrl},
        {GLFW_KEY_LEFT_ALT, Key::LeftAlt}, {GLFW_KEY_RIGHT_ALT, Key::RightAlt},
        {GLFW_KEY_LEFT_SUPER, Key::LeftSuper}, {GLFW_KEY_RIGHT_SUPER, Key::RightSuper},
        
        // Navegação
        {GLFW_KEY_UP, Key::Up}, {GLFW_KEY_DOWN, Key::Down},
        {GLFW_KEY_LEFT, Key::Left}, {GLFW_KEY_RIGHT, Key::Right},
        {GLFW_KEY_PAGE_UP, Key::PageUp}, {GLFW_KEY_PAGE_DOWN, Key::PageDown},
        {GLFW_KEY_HOME, Key::Home}, {GLFW_KEY_END, Key::End},
        {GLFW_KEY_INSERT, Key::Insert}, {GLFW_KEY_DELETE, Key::Delete},
        
        // Especiais
        {GLFW_KEY_SPACE, Key::Space}, {GLFW_KEY_ENTER, Key::Enter},
        {GLFW_KEY_BACKSPACE, Key::Backspace}, {GLFW_KEY_TAB, Key::Tab},
        {GLFW_KEY_ESCAPE, Key::Escape}, {GLFW_KEY_CAPS_LOCK, Key::CapsLock},
        {GLFW_KEY_SCROLL_LOCK, Key::ScrollLock}, {GLFW_KEY_NUM_LOCK, Key::NumLock},
        {GLFW_KEY_PRINT_SCREEN, Key::PrintScreen}, {GLFW_KEY_PAUSE, Key::Pause},
        {GLFW_KEY_MENU, Key::Menu},
        
        // Keypad numérico
        {GLFW_KEY_KP_0, Key::KP0}, {GLFW_KEY_KP_1, Key::KP1}, {GLFW_KEY_KP_2, Key::KP2},
        {GLFW_KEY_KP_3, Key::KP3}, {GLFW_KEY_KP_4, Key::KP4}, {GLFW_KEY_KP_5, Key::KP5},
        {GLFW_KEY_KP_6, Key::KP6}, {GLFW_KEY_KP_7, Key::KP7}, {GLFW_KEY_KP_8, Key::KP8},
        {GLFW_KEY_KP_9, Key::KP9}, {GLFW_KEY_KP_DECIMAL, Key::KPDecimal},
        {GLFW_KEY_KP_DIVIDE, Key::KPDivide}, {GLFW_KEY_KP_MULTIPLY, Key::KPMultiply},
        {GLFW_KEY_KP_SUBTRACT, Key::KPSubtract}, {GLFW_KEY_KP_ADD, Key::KPAdd},
        {GLFW_KEY_KP_ENTER, Key::KPEnter}, {GLFW_KEY_KP_EQUAL, Key::KPEqual},
        
        // Símbolos
        {GLFW_KEY_SEMICOLON, Key::Semicolon}, {GLFW_KEY_EQUAL, Key::Equal},
        {GLFW_KEY_COMMA, Key::Comma}, {GLFW_KEY_MINUS, Key::Minus},
        {GLFW_KEY_PERIOD, Key::Period}, {GLFW_KEY_SLASH, Key::Slash},
        {GLFW_KEY_GRAVE_ACCENT, Key::GraveAccent},
        {GLFW_KEY_LEFT_BRACKET, Key::LeftBracket}, {GLFW_KEY_BACKSLASH, Key::Backslash},
        {GLFW_KEY_RIGHT_BRACKET, Key::RightBracket}, {GLFW_KEY_APOSTROPHE, Key::Apostrophe}
    };
    
    // Mapeamento de botões do mouse
    static const std::unordered_map<int, MouseButton> glfwToMouseButton = {
        {GLFW_MOUSE_BUTTON_LEFT, MouseButton::Left},
        {GLFW_MOUSE_BUTTON_RIGHT, MouseButton::Right},
        {GLFW_MOUSE_BUTTON_MIDDLE, MouseButton::Middle},
        {GLFW_MOUSE_BUTTON_4, MouseButton::Button4},
        {GLFW_MOUSE_BUTTON_5, MouseButton::Button5},
        {GLFW_MOUSE_BUTTON_6, MouseButton::Button6},
        {GLFW_MOUSE_BUTTON_7, MouseButton::Button7},
        {GLFW_MOUSE_BUTTON_8, MouseButton::Button8}
    };

    class GLFWInputManager : public IInputManager {
    public:
        explicit GLFWInputManager(GLFWwindow* window)
            : _window(window)
            , _mouseLocked(false)
            , _mouseVisible(true)
            , _firstMouse(true)
            , _lastMouseX(0.0)
            , _lastMouseY(0.0)
        {
            if (!_window) {
                Drift::Core::Log("[InputManager] ERRO: Janela GLFW é nullptr!");
                return;
            }
            
            // Configura callbacks do GLFW
            glfwSetWindowUserPointer(_window, this);
            glfwSetScrollCallback(_window, ScrollCallback);
        }
        
        ~GLFWInputManager() override {
            if (_window) {
                glfwSetScrollCallback(_window, nullptr);
                glfwSetWindowUserPointer(_window, nullptr);
            }
        }

        void Update() override {
            if (!_window) return;
            
            // Swap frames (previous <- current)
            _previousFrame = _currentFrame;
            
            // Reset scroll for next frame
            ResetScrollForNextFrame();
            
            // Update mouse position and delta
            double mouseX, mouseY;
            glfwGetCursorPos(_window, &mouseX, &mouseY);
            
            if (_firstMouse) {
                _lastMouseX = mouseX;
                _lastMouseY = mouseY;
                _firstMouse = false;
            }
            
            _currentFrame.mousePosition = {static_cast<float>(mouseX), static_cast<float>(mouseY)};
            _currentFrame.mouseDelta = {
                static_cast<float>(mouseX - _lastMouseX),
                static_cast<float>(mouseY - _lastMouseY)
            };
            
            _lastMouseX = mouseX;
            _lastMouseY = mouseY;
            
            // Update keyboard state
            for (const auto& [glfwKey, ourKey] : glfwToKey) {
                int state = glfwGetKey(_window, glfwKey);
                bool isPressed = (state == GLFW_PRESS || state == GLFW_REPEAT);
                
                int keyIdx = static_cast<int>(ourKey);
                _currentFrame.keys[keyIdx] = UpdateKeyState(_currentFrame.keys[keyIdx], isPressed);
            }
            
            // Update mouse button state
            for (const auto& [glfwButton, ourButton] : glfwToMouseButton) {
                int state = glfwGetMouseButton(_window, glfwButton);
                bool isPressed = (state == GLFW_PRESS);
                
                int buttonIdx = static_cast<int>(ourButton);
                _currentFrame.mouseButtons[buttonIdx] = UpdateKeyState(
                    _currentFrame.mouseButtons[buttonIdx], isPressed);
            }
            
        }

        const InputFrame& GetCurrentFrame() const override {
            return _currentFrame;
        }

        const InputFrame& GetPreviousFrame() const override {
            return _previousFrame;
        }

        void SetMouseLocked(bool locked) override {
            if (!_window || _mouseLocked == locked) return;
            
            _mouseLocked = locked;
            if (locked) {
                glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        bool IsMouseLocked() const override {
            return _mouseLocked;
        }

        void SetMouseVisible(bool visible) override {
            if (!_window || _mouseVisible == visible) return;
            
            _mouseVisible = visible;
            if (!_mouseLocked) {  // Só aplica se não estiver locked
                glfwSetInputMode(_window, GLFW_CURSOR, 
                                visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
            }
        }

        bool IsMouseVisible() const override {
            return _mouseVisible;
        }

    private:
        GLFWwindow* _window;
        InputFrame _currentFrame;
        InputFrame _previousFrame;
        
        bool _mouseLocked;
        bool _mouseVisible;
        bool _firstMouse;
        double _lastMouseX, _lastMouseY;
        
        // Callback para scroll do mouse
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
            auto* manager = static_cast<GLFWInputManager*>(glfwGetWindowUserPointer(window));
            if (manager) {
                manager->_currentFrame.mouseScroll.x += static_cast<float>(xoffset);
                manager->_currentFrame.mouseScroll.y += static_cast<float>(yoffset);
            }
        }
        
        // Reset scroll para próximo frame
        void ResetScrollForNextFrame() {
            _currentFrame.mouseScroll = {0.0f, 0.0f};
        }
    };

    // Factory function
    std::unique_ptr<IInputManager> CreateGLFWInputManager(void* glfwWindow) {
        return std::make_unique<GLFWInputManager>(static_cast<GLFWwindow*>(glfwWindow));
    }

} // namespace Drift::Engine::Input 