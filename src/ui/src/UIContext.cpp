#include "Drift/UI/UIContext.h"
#include "Drift/UI/EventBus.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/UIElement.h"

using namespace Drift::UI;

UIContext::UIContext()
    : m_EventBus(std::make_shared<EventBus>())
    , m_LayoutEngine(std::make_unique<LayoutEngine>())
{
    Drift::Core::Log("UIContext criado");
    // Cria elemento raiz que cobre a tela inteira por padrão
    m_Root = std::make_shared<UIElement>(this);
}

UIContext::~UIContext()
{
    Shutdown();
}

void UIContext::Initialize()
{
    // Carregar temas, preparar atlases, etc.
    Drift::Core::Log("UIContext::Initialize");
}

void UIContext::Update(float deltaSeconds)
{
    // Processa layout (placeholder)
    if (m_LayoutEngine && m_Root)
        m_LayoutEngine->Layout(*m_Root);

    // Futuramente: animações e render queue
    (void)deltaSeconds;
}

void UIContext::Render(Drift::RHI::IUIBatcher& batch)
{
    if (m_Root)
        m_Root->Render(batch);
}

void UIContext::Shutdown()
{
    // Liberar recursos
    if (m_EventBus)
    {
        Drift::Core::Log("UIContext::Shutdown");
        m_EventBus.reset();
    }
} 