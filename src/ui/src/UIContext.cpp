#include "Drift/UI/UIContext.h"
#include "Drift/Engine/EventBus.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIInputHandler.h"
#include "Drift/UI/DataDriven/UIComponentRegistry.h"
#include "Drift/Engine/Input/InputManager.h"
#include <glm/mat4x4.hpp>

using namespace Drift::UI;

namespace {
    void ProcessLayoutRecursive(UIElement* element) {
        if (!element) return;
        
        // Processa este elemento se estiver dirty
        if (element->IsLayoutDirty()) {
            element->RecalculateLayout();
        }
        
        // Processa filhos recursivamente
        for (auto& child : element->GetChildren()) {
            ProcessLayoutRecursive(child.get());
        }
    }
}

UIContext::UIContext()
    : m_EventBus(std::make_shared<Drift::Engine::EventBus>())
    , m_LayoutEngine(std::make_unique<LayoutEngine>())
    , m_InputHandler(std::make_unique<UIInputHandler>(this))
{
    // Cria elemento raiz que cobre a tela inteira por padrão
    m_Root = std::make_shared<UIElement>(this);
    m_Root->SetName("Root");
    m_Root->SetPosition({0.0f, 0.0f});
    m_Root->SetSize({1920.0f, 1080.0f}); // Tamanho padrão, será ajustado
    m_Root->SetColor(0x00000000); // Transparente
    
    // Configura layout do root para preencher toda a tela
    LayoutProperties rootLayout;
    rootLayout.horizontalAlign = LayoutProperties::HorizontalAlign::Stretch;
    rootLayout.verticalAlign = LayoutProperties::VerticalAlign::Stretch;
    m_Root->SetLayoutProperties(rootLayout);
}

UIContext::~UIContext()
{
    Shutdown();
}

void UIContext::Initialize()
{
    // Registra widgets padrão apenas uma vez
    static bool widgetsRegistered = false;
    if (!widgetsRegistered) {
        UIComponentRegistry::GetInstance().RegisterDefaultWidgets();
        widgetsRegistered = true;
    }
    
    // Carregar temas, preparar atlases, etc.
}

void UIContext::Update(float deltaSeconds)
{
    // Atualiza o sistema de input
    if (m_InputHandler) {
        m_InputHandler->Update(deltaSeconds);
    }
    
    // Atualiza elementos da UI
    if (m_Root) {
        m_Root->Update(deltaSeconds);
    }
    
    // Processa layout usando engine
    if (m_LayoutEngine && m_Root) {
        m_LayoutEngine->Layout(*m_Root);
    }
    
    // Chama RecalculateLayout recursivamente em todos os elementos que precisam
    if (m_Root) {
        ProcessLayoutRecursive(m_Root.get());
    }

    // Prepara transformações para renderização
    if (m_Root) {
        m_Root->PreRender(glm::mat4(1.0f));
    }
}

void UIContext::Render(Drift::RHI::IUIBatcher& batch)
{
    if (m_Root) {
        m_Root->Render(batch);
        m_Root->PostRender();
    }
}

void UIContext::Shutdown()
{
    // Liberar recursos
    if (m_EventBus) {
        Drift::Core::Log("UIContext::Shutdown");
        m_EventBus.reset();
    }
    
    m_InputHandler.reset();
    m_Root.reset();
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
        // Marca como dirty para recalcular layout
        m_Root->MarkLayoutDirty();
        
        // Força recálculo imediato do layout para todos os elementos
        if (m_LayoutEngine) {
            m_LayoutEngine->Layout(*m_Root);
        }
        
        // Processa layout recursivamente em todos os elementos
        ProcessLayoutRecursive(m_Root.get());
        
        // Prepara transformações para renderização
        m_Root->PreRender(glm::mat4(1.0f));
    }
}

std::shared_ptr<UIElement> UIContext::GetRoot() const
{
    return m_Root;
}

std::shared_ptr<Drift::Engine::EventBus> UIContext::GetEventBus() const
{
    return m_EventBus;
}

UIElement* UIContext::HitTest(const glm::vec2& point)
{
    if (m_Root) {
        return m_Root->HitTestChildren(point);
    }
    return nullptr;
} 