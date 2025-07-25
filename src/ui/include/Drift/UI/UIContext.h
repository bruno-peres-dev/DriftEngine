#pragma once

#include <memory>
#include "Drift/Core/Log.h"
#include <vector>
#include <glm/vec2.hpp>

namespace Drift::RHI { class IUIBatcher; }

// Forward declaration para evitar include circular
namespace Drift::Engine::Input {
    class IInputManager;
}

namespace Drift::Engine {
    class EventBus;
}

namespace Drift::UI {

class LayoutEngine;
class UIElement;
class UIInputHandler;

class UIContext {
public:
    UIContext();
    ~UIContext();

    // Inicializa subsistemas de UI (por ex. tema, cache, atlas etc.)
    void Initialize();

    // Loop de atualização
    void Update(float deltaSeconds);

    // Renderiza a árvore de UI usando um IUIBatcher
    void Render(Drift::RHI::IUIBatcher& batch);

    // Limpa recursos gráficos
    void Shutdown();

    // Acesso global ao EventBus
    std::shared_ptr<Drift::Engine::EventBus> GetEventBus() const;

    // Raiz da árvore de elementos
    std::shared_ptr<UIElement> GetRoot() const;

    // Acesso ao sistema de input
    UIInputHandler* GetInputHandler() const { return m_InputHandler.get(); }

    // Conecta ao sistema de input da Engine
    void SetInputManager(Drift::Engine::Input::IInputManager* inputManager);
    
    // Ajusta o tamanho da tela
    void SetScreenSize(float width, float height);
    
    // Hit testing
    UIElement* HitTest(const glm::vec2& point);

private:
    std::shared_ptr<Drift::Engine::EventBus> m_EventBus;

    std::unique_ptr<LayoutEngine> m_LayoutEngine;
    std::unique_ptr<UIInputHandler> m_InputHandler;
    std::shared_ptr<UIElement> m_Root;
};

} // namespace Drift::UI 