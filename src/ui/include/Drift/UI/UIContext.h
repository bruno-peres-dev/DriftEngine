#pragma once

#include <memory>
#include <string>
#include <functional>
#include "Drift/UI/UIElement.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/UIInputHandler.h"
#include "Drift/Engine/EventBus.h"
#include "Drift/RHI/Device.h"
#include "Drift/Core/Log.h"
#include <vector>
#include <glm/vec2.hpp>

#include "Drift/RHI/UIBatcher.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/FontRendering.h"

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
    friend class UIInputHandler;
public:
    UIContext();
    ~UIContext();

    // Inicializa subsistemas de UI (por ex. tema, cache, atlas etc.)
    void Initialize();

    // Configurar device para o sistema de fontes
    void SetDevice(Drift::RHI::IDevice* device);
    
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

    Drift::UI::FontRendering* GetTextRenderer() const { return m_TextRenderer.get(); }

private:
    UIElement* FindElementAtPosition(UIElement* element, const glm::vec2& point) const;
    void InitializeFontSystem();
    void LoadFonts();
    std::string ResolveFontPath(const std::string& relativePath);
    
    std::shared_ptr<Drift::Engine::EventBus> m_EventBus;
    Drift::RHI::IDevice* m_Device{nullptr}; // Device para o sistema de fontes

    std::unique_ptr<LayoutEngine> m_LayoutEngine;
    std::unique_ptr<UIInputHandler> m_InputHandler;
    std::shared_ptr<UIElement> m_Root;
    std::unique_ptr<Drift::UI::FontRendering> m_TextRenderer;
};

} // namespace Drift::UI 