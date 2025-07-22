#pragma once

#include <memory>
#include "Drift/Core/Log.h"
#include <vector>

namespace Drift::RHI { class IUIBatcher; }

namespace Drift::UI {

class LayoutEngine;
class UIElement;
class EventBus;

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
    std::shared_ptr<EventBus> GetEventBus() const { return m_EventBus; }

    // Raiz da árvore de elementos
    std::shared_ptr<UIElement> GetRoot() const { return m_Root; }

private:
    std::shared_ptr<EventBus> m_EventBus;

    std::unique_ptr<LayoutEngine> m_LayoutEngine;
    std::shared_ptr<UIElement> m_Root;
};

} // namespace Drift::UI 