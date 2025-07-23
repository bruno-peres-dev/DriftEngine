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
    Core::Log("[Flexbox Test] Iniciando teste do Flexbox...");

    // ================================
    // 1. INICIALIZAÇÃO DO GLFW
    // ================================
    if (!glfwInit()) {
        Core::Log("[Flexbox Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }

    // ================================
    // 2. CRIAÇÃO DA JANELA
    // ================================
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "DriftEngine Flexbox Test", nullptr, nullptr);
    if (!window) {
        Core::Log("[Flexbox Test] ERRO: Falha ao criar janela!");
        glfwTerminate();
        return -1;
    }

    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        Core::Log("[Flexbox Test] ERRO: Falha ao obter HWND!");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // ================================
    // 3. INICIALIZAÇÃO DO DIRECTX 11
    // ================================
    RHI::DeviceDesc desc{ 800, 600, false };
    auto device = RHI::DX11::CreateDeviceDX11(desc);
    auto swapChain = device->CreateSwapChain(hwnd);
    auto context = device->CreateContext();

    // ================================
    // 4. SISTEMA DE INPUT
    // ================================
    auto inputManager = Engine::Input::CreateGLFWInputManager(window);

    // ================================
    // 5. SISTEMA DE UI
    // ================================
    auto uiContext = std::make_unique<UI::UIContext>();
    uiContext->Initialize();
    uiContext->SetInputManager(inputManager.get());

    // ================================
    // 6. UI BATCHER E RING BUFFER
    // ================================
    ID3D11Device* nativeDev = static_cast<ID3D11Device*>(device->GetNativeDevice());
    ID3D11DeviceContext* nativeCtx = static_cast<ID3D11DeviceContext*>(context->GetNativeContext());

    auto uiRingBuffer = RHI::DX11::CreateRingBufferDX11(nativeDev, nativeCtx, 1024 * 1024);
    auto uiBatcher = RHI::DX11::CreateUIBatcherDX11(uiRingBuffer, context.get());

    // ================================
    // 7. CRIAÇÃO DO LAYOUT FLEXBOX
    // ================================
    Core::Log("[Flexbox Test] Criando layout Flexbox...");

    // Container principal com Flexbox
    auto mainContainer = std::make_shared<UI::Panel>(uiContext.get());
    mainContainer->SetTitle("Container Flexbox");
    mainContainer->SetPosition(glm::vec2(50, 50));
    mainContainer->SetSize(glm::vec2(700, 500));
    mainContainer->SetColor(0xFF2A2A2A);
    mainContainer->SetBorderColor(0xFF666666);
    uiContext->GetRoot()->AddChild(mainContainer);

    // Configuração Flexbox para o container
    UI::FlexProperties flexProps;
    flexProps.direction = UI::FlexProperties::Direction::Row;
    flexProps.justifyContent = UI::FlexProperties::JustifyContent::SpaceEvenly;
    flexProps.alignItems = UI::FlexProperties::AlignItems::Center;
    flexProps.wrap = UI::FlexProperties::Wrap::Wrap;
    flexProps.gap = 10.0f;

    // Botões com diferentes flex-grow
    auto button1 = std::make_shared<UI::Button>(uiContext.get());
    button1->SetText("Flex 1");
    button1->SetSize(glm::vec2(100, 50));
    button1->SetNormalColor(0xFFFF0000);
    button1->SetHoverColor(0xFFCC0000);
    button1->SetPressedColor(0xFF990000);
    mainContainer->AddChild(button1);

    auto button2 = std::make_shared<UI::Button>(uiContext.get());
    button2->SetText("Flex 2");
    button2->SetSize(glm::vec2(100, 50));
    button2->SetNormalColor(0xFF00FF00);
    button2->SetHoverColor(0xFF00CC00);
    button2->SetPressedColor(0xFF009900);
    mainContainer->AddChild(button2);

    auto button3 = std::make_shared<UI::Button>(uiContext.get());
    button3->SetText("Flex 3");
    button3->SetSize(glm::vec2(100, 50));
    button3->SetNormalColor(0xFF0000FF);
    button3->SetHoverColor(0xFF0000CC);
    button3->SetPressedColor(0xFF000099);
    mainContainer->AddChild(button3);

    // Container vertical aninhado
    auto verticalContainer = std::make_shared<UI::Panel>(uiContext.get());
    verticalContainer->SetTitle("Vertical");
    verticalContainer->SetSize(glm::vec2(150, 200));
    verticalContainer->SetColor(0xFF1A1A1A);
    verticalContainer->SetBorderColor(0xFF888888);
    mainContainer->AddChild(verticalContainer);

    // Botões no container vertical
    auto vButton1 = std::make_shared<UI::Button>(uiContext.get());
    vButton1->SetText("V1");
    vButton1->SetSize(glm::vec2(120, 40));
    vButton1->SetNormalColor(0xFFFF8800);
    verticalContainer->AddChild(vButton1);

    auto vButton2 = std::make_shared<UI::Button>(uiContext.get());
    vButton2->SetText("V2");
    vButton2->SetSize(glm::vec2(120, 40));
    vButton2->SetNormalColor(0xFF8800FF);
    verticalContainer->AddChild(vButton2);

    // Botão Sair
    auto quitButton = std::make_shared<UI::Button>(uiContext.get());
    quitButton->SetText("Sair");
    quitButton->SetPosition(glm::vec2(650, 550));
    quitButton->SetSize(glm::vec2(100, 30));
    quitButton->SetNormalColor(0xFF666666);
    quitButton->SetHoverColor(0xFF888888);
    quitButton->SetPressedColor(0xFF444444);
    quitButton->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Flexbox Test] Saindo...");
        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
    });
    uiContext->GetRoot()->AddChild(quitButton);

    Core::Log("[Flexbox Test] Layout Flexbox criado!");

    // ================================
    // 8. LOOP PRINCIPAL
    // ================================
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Timing
        double now = glfwGetTime();
        float deltaTime = float(now - lastTime);
        lastTime = now;

        // Update
        inputManager->Update();
        uiContext->Update(deltaTime);

        // Aplica Flexbox layout
        UI::FlexLayoutEngine::LayoutFlexContainer(mainContainer.get(), flexProps);
        
        // Para o container vertical, aplica layout vertical
        UI::FlexProperties verticalFlexProps;
        verticalFlexProps.direction = UI::FlexProperties::Direction::Column;
        verticalFlexProps.justifyContent = UI::FlexProperties::JustifyContent::SpaceEvenly;
        verticalFlexProps.alignItems = UI::FlexProperties::AlignItems::Center;
        verticalFlexProps.gap = 5.0f;
        UI::FlexLayoutEngine::LayoutFlexContainer(verticalContainer.get(), verticalFlexProps);

        // Render
        context->Clear(0.1f, 0.1f, 0.1f, 1.0f);
        
        // UI Render
        uiBatcher->Begin();
        uiContext->Render(*uiBatcher);
        uiBatcher->End();

        // Present
        context->Present();
    }

    // ================================
    // 9. FINALIZAÇÃO
    // ================================
    Core::Log("[Flexbox Test] Finalizando...");
    uiContext->Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    Core::Log("[Flexbox Test] Teste concluído com sucesso!");

    return 0;
} 