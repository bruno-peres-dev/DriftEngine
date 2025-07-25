#include "Drift/Core/Log.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/Widgets/Panel.h"
#include "Drift/UI/LayoutEngine/FlexLayout.h"
#include "Drift/Engine/Input/InputManager.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/DX11/ContextDX11.h"
#include "Drift/RHI/DX11/SwapChainDX11.h"
#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/DX11/RingBufferDX11.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <memory>

using namespace Drift;

int main() {
    Core::Log("[Layout Test] Iniciando...");

    if (!glfwInit()) {
        Core::Log("[Layout Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "DriftEngine Layout Test", nullptr, nullptr);
    if (!window) {
        Core::Log("[Layout Test] ERRO: Falha ao criar janela!");
        glfwTerminate();
        return -1;
    }

    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        Core::Log("[Layout Test] ERRO: Falha ao obter HWND!");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    RHI::DeviceDesc desc{800, 600, false};
    auto device = RHI::DX11::CreateDeviceDX11(desc);
    auto swapChain = device->CreateSwapChain(hwnd);
    auto context = device->CreateContext();

    auto inputManager = Engine::Input::CreateGLFWInputManager(window);

    auto uiContext = std::make_unique<UI::UIContext>();
    uiContext->Initialize();
    uiContext->SetInputManager(inputManager.get());

    ID3D11Device* nativeDev = static_cast<ID3D11Device*>(device->GetNativeDevice());
    ID3D11DeviceContext* nativeCtx = static_cast<ID3D11DeviceContext*>(context->GetNativeContext());

    auto uiRingBuffer = RHI::DX11::CreateRingBufferDX11(nativeDev, nativeCtx, 1024 * 1024);
    auto uiBatcher = RHI::DX11::CreateUIBatcherDX11(uiRingBuffer, context.get());

    Core::Log("[Layout Test] Criando layout...");

    auto mainPanel = std::make_shared<UI::Panel>(uiContext.get());
    mainPanel->SetTitle("Painel Principal");
    mainPanel->SetPosition(glm::vec2(40, 40));
    mainPanel->SetSize(glm::vec2(720, 520));
    mainPanel->SetColor(0xFF2A2A2A);
    mainPanel->SetBorderColor(0xFF444444);
    uiContext->GetRoot()->AddChild(mainPanel);

    UI::FlexProperties mainFlex;
    mainFlex.direction = UI::FlexProperties::Direction::Row;
    mainFlex.justifyContent = UI::FlexProperties::JustifyContent::SpaceAround;
    mainFlex.alignItems = UI::FlexProperties::AlignItems::Center;
    mainFlex.gap = 15.0f;

    auto menuPanel = std::make_shared<UI::Panel>(uiContext.get());
    menuPanel->SetTitle("Menu");
    menuPanel->SetSize(glm::vec2(200, 400));
    menuPanel->SetColor(0xFF1A1A1A);
    menuPanel->SetBorderColor(0xFF666666);
    mainPanel->AddChild(menuPanel);

    auto contentPanel = std::make_shared<UI::Panel>(uiContext.get());
    contentPanel->SetTitle("Conteudo");
    contentPanel->SetSize(glm::vec2(460, 400));
    contentPanel->SetColor(0xFF222222);
    contentPanel->SetBorderColor(0xFF666666);
    mainPanel->AddChild(contentPanel);

    UI::FlexProperties menuFlex;
    menuFlex.direction = UI::FlexProperties::Direction::Column;
    menuFlex.justifyContent = UI::FlexProperties::JustifyContent::SpaceEvenly;
    menuFlex.alignItems = UI::FlexProperties::AlignItems::Center;
    menuFlex.gap = 10.0f;

    auto btnHome = std::make_shared<UI::Button>(uiContext.get());
    btnHome->SetText("Home");
    btnHome->SetSize(glm::vec2(160, 40));
    btnHome->SetNormalColor(0xFF0066CC);
    menuPanel->AddChild(btnHome);

    auto btnSettings = std::make_shared<UI::Button>(uiContext.get());
    btnSettings->SetText("Config");
    btnSettings->SetSize(glm::vec2(160, 40));
    btnSettings->SetNormalColor(0xFF00CC66);
    menuPanel->AddChild(btnSettings);

    auto btnQuit = std::make_shared<UI::Button>(uiContext.get());
    btnQuit->SetText("Sair");
    btnQuit->SetSize(glm::vec2(160, 40));
    btnQuit->SetNormalColor(0xFFCC3333);
    btnQuit->SetOnClick([](const UI::ButtonClickEvent&){
        Core::Log("[Layout Test] Encerrando...");
        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
    });
    menuPanel->AddChild(btnQuit);

    Core::Log("[Layout Test] Layout criado!");

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double now = glfwGetTime();
        float deltaTime = float(now - lastTime);
        lastTime = now;

        inputManager->Update();
        uiContext->Update(deltaTime);

        UI::FlexLayoutEngine::LayoutFlexContainer(mainPanel.get(), mainFlex);
        UI::FlexLayoutEngine::LayoutFlexContainer(menuPanel.get(), menuFlex);

        context->Clear(0.1f, 0.1f, 0.1f, 1.0f);

        uiBatcher->Begin();
        uiContext->Render(*uiBatcher);
        uiBatcher->End();

        context->Present();
    }

    Core::Log("[Layout Test] Finalizando...");
    uiContext->Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    Core::Log("[Layout Test] Concluído.");

    return 0;
}

