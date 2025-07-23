#include "Drift/UI/UIContext.h"
#include "Drift/Engine/EventBus.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIInputHandler.h"
#include "Drift/Engine/Input/InputManager.h"

using namespace Drift::UI;

UIContext::UIContext()
    : m_EventBus(std::make_shared<Drift::Engine::EventBus>())
    , m_LayoutEngine(std::make_unique<LayoutEngine>())
    , m_InputHandler(std::make_unique<UIInputHandler>(this))
{
    // Cria elemento raiz que cobre a tela inteira por padrão
    m_Root = std::make_shared<UIElement>(this);
    m_Root->SetPosition({0.0f, 0.0f});
    m_Root->SetSize({1920.0f, 1080.0f}); // Tamanho padrão, será ajustado
    m_Root->SetColor(0x00000000); // Transparente
}

UIContext::~UIContext()
{
    Shutdown();
}

void UIContext::Initialize()
{
    // Carregar temas, preparar atlases, etc.
}

void UIContext::Update(float deltaSeconds)
{
    // Atualiza o sistema de input
    if (m_InputHandler) {
        m_InputHandler->Update(deltaSeconds);
    }
    
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
    
    m_InputHandler.reset();
}

void UIContext::SetInputManager(Drift::Engine::Input::IInputManager* inputManager)
{
    if (m_InputHandler) {
        m_InputHandler->SetInputManager(inputManager);
    }
}

void UIContext::SetScreenSize(float width, float height)
{
    if (m_Root) {
        m_Root->SetSize({width, height});
    }
} 