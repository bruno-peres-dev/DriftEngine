#include "Drift/UI/UIContext.h"
#include "Drift/Engine/EventBus.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIInputHandler.h"
#include "Drift/UI/DataDriven/UIComponentRegistry.h"
#include "Drift/Engine/Input/InputManager.h"
#include <glm/mat4x4.hpp>
#include <mutex>
#include <fstream>

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
    // Registra widgets padrão apenas uma vez de forma thread-safe
    static std::once_flag widgetsRegisteredFlag;
    std::call_once(widgetsRegisteredFlag, []() {
        UIComponentRegistry::GetInstance().RegisterDefaultWidgets();
    });
    
    // Inicializar sistema de fontes
    InitializeFontSystem();
    
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
    
    // Processa layout usando engine somente se necessário
    if (m_LayoutEngine && m_Root && m_Root->IsLayoutDirty()) {
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

void UIContext::InitializeFontSystem()
{
    Core::Log("[UIContext] Inicializando sistema de fontes...");
    
    auto& fontManager = UI::FontManager::GetInstance();
    
    // Configurar fonte padrão
    fontManager.SetDefaultFontName("default");
    fontManager.SetDefaultSize(16.0f);
    fontManager.SetDefaultQuality(UI::FontQuality::High);
    
    // Tentar carregar fonte do arquivo
    std::string fontPath = "fonts/Arial-Regular.ttf";
    
    // Verificar se o arquivo existe
    std::ifstream testFile(fontPath);
    if (!testFile.good()) {
        Core::Log("[UIContext] ERRO: Arquivo de fonte não encontrado: " + fontPath);
        Core::Log("[UIContext] Tentando caminho absoluto...");
        fontPath = "C:/Users/Bruno/Desktop/DriftEngine/fonts/Arial-Regular.ttf";
    } else {
        testFile.close();
        Core::Log("[UIContext] Arquivo de fonte encontrado: " + fontPath);
    }
    
    auto font = fontManager.LoadFont("default", fontPath, 16.0f, UI::FontQuality::High);
    
    if (font) {
        Core::Log("[UIContext] Fonte padrão carregada com sucesso: " + fontPath);
        Core::Log("[UIContext] Sistema de fontes inicializado com sucesso!");
    } else {
        Core::Log("[UIContext] AVISO: Não foi possível carregar a fonte padrão: " + fontPath);
        Core::Log("[UIContext] O sistema usará a fonte embutida padrão.");
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

UIElement* UIContext::FindElementAtPosition(UIElement* element, const glm::vec2& point) const
{
    if (!element) return nullptr;

    const bool pointInside = element->HitTest(point);

    if (element->GetLayoutProperties().clipContent && !pointInside)
        return nullptr;

    auto& children = element->GetChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if (UIElement* childHit = FindElementAtPosition(it->get(), point)) {
            return childHit;
        }
    }

    return pointInside ? element : nullptr;
}

UIElement* UIContext::HitTest(const glm::vec2& point)
{
    if (m_Root) {
        return FindElementAtPosition(m_Root.get(), point);
    }
    return nullptr;
}
