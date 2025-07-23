// Teste simples da UI - focado apenas nos botões
#include "Drift/Core/Log.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Button.h"
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
    Core::Log("[UI Test] Iniciando teste da UI...");

    // ================================
    // 1. INICIALIZAÇÃO DO GLFW
    // ================================
    if (!glfwInit()) {
        Core::Log("[UI Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }

    // ================================
    // 2. CRIAÇÃO DA JANELA
    // ================================
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "DriftEngine UI Test", nullptr, nullptr);
    if (!window) {
        Core::Log("[UI Test] ERRO: Falha ao criar janela!");
        glfwTerminate();
        return -1;
    }

    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        Core::Log("[UI Test] ERRO: Falha ao obter HWND!");
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
    // 7. CRIAÇÃO DOS WIDGETS
    // ================================
    Core::Log("[UI Test] Criando widgets...");

    // Botão vermelho
    auto redButton = std::make_shared<UI::Button>(uiContext.get());
    redButton->SetText("Botão Vermelho");
    redButton->SetPosition(glm::vec2(50, 50));
    redButton->SetSize(glm::vec2(200, 50));
    redButton->SetNormalColor(0xFFFF0000);
    redButton->SetHoverColor(0xFFCC0000);
    redButton->SetPressedColor(0xFF990000);
    redButton->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[UI Test] Botão vermelho clicado!");
    });
    uiContext->GetRoot()->AddChild(redButton);

    // Botão verde
    auto greenButton = std::make_shared<UI::Button>(uiContext.get());
    greenButton->SetText("Botão Verde");
    greenButton->SetPosition(glm::vec2(50, 120));
    greenButton->SetSize(glm::vec2(200, 50));
    greenButton->SetNormalColor(0xFF00FF00);
    greenButton->SetHoverColor(0xFF00CC00);
    greenButton->SetPressedColor(0xFF009900);
    greenButton->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[UI Test] Botão verde clicado!");
    });
    uiContext->GetRoot()->AddChild(greenButton);

    // Botão Sair
    auto quitButton = std::make_shared<UI::Button>(uiContext.get());
    quitButton->SetText("Sair");
    quitButton->SetPosition(glm::vec2(50, 190));
    quitButton->SetSize(glm::vec2(200, 50));
    quitButton->SetNormalColor(0xFF666666);
    quitButton->SetHoverColor(0xFF888888);
    quitButton->SetPressedColor(0xFF444444);
    quitButton->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[UI Test] Saindo...");
        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
    });
    uiContext->GetRoot()->AddChild(quitButton);

    Core::Log("[UI Test] Widgets criados. Teste movendo o mouse e clicando!");

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
    Core::Log("[UI Test] Finalizando...");
    uiContext->Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    Core::Log("[UI Test] Teste concluído com sucesso!");

    return 0;
} 