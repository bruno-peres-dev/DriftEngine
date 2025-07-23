// Teste simples da UI - focado apenas nos botões
#include "Drift/Core/Log.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/Engine/Input/Input.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/RHI/DX11/RingBufferDX11.h"
#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include <d3d11.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <Windows.h>
#include <memory>
#include <stdexcept>

using namespace Drift;

int main() {
    try {
        Core::Log("[UI Test] Iniciando teste da UI...");
        
        // ================================
        // 1. INICIALIZAÇÃO BÁSICA
        // ================================
        
        if (!glfwInit())
            throw std::runtime_error("Falha ao inicializar GLFW");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(800, 600, "UI Test - DriftEngine", nullptr, nullptr);
        if (!window) throw std::runtime_error("Falha ao criar janela GLFW");

        HWND hwnd = glfwGetWin32Window(window);
        if (!hwnd) throw std::runtime_error("Falha ao obter HWND");

        // ================================
        // 2. RHI SIMPLES
        // ================================
        
        RHI::DeviceDesc desc{ 800, 600, false };
        auto device = RHI::DX11::CreateDeviceDX11(desc);
        auto swapChain = device->CreateSwapChain(hwnd);
        auto context = device->CreateContext();
        
        // ================================
        // 3. SISTEMA DE INPUT
        // ================================
        
        auto inputManager = Engine::Input::CreateGLFWInputManager(window);
        
        // ================================
        // 4. UI CONTEXT
        // ================================
        
        auto uiContext = std::make_unique<UI::UIContext>();
        uiContext->Initialize();
        uiContext->SetInputManager(inputManager.get());
        uiContext->SetScreenSize(800.0f, 600.0f);

        // ================================
        // 5. UI BATCHER
        // ================================
        
        ID3D11Device* nativeDev = static_cast<ID3D11Device*>(device->GetNativeDevice());
        ID3D11DeviceContext* nativeCtx = static_cast<ID3D11DeviceContext*>(context->GetNativeContext());
        
        auto uiRingBuffer = RHI::DX11::CreateRingBufferDX11(nativeDev, nativeCtx, 1024 * 1024);
        auto uiBatcher = RHI::DX11::CreateUIBatcherDX11(uiRingBuffer, context.get());
        uiBatcher->SetScreenSize(800.0f, 600.0f);
        
        // ================================
        // 6. CRIAÇÃO DOS BOTÕES
        // ================================
        
        Core::Log("[UI Test] Criando botões...");
        
        auto root = uiContext->GetRoot();
        
        // Botão vermelho
        auto redButton = std::make_shared<UI::Button>(uiContext.get());
        redButton->SetText("Botão Vermelho");
        redButton->SetPosition({50.0f, 50.0f});
        redButton->SetSize({200.0f, 50.0f});
        redButton->SetNormalColor(0xFFFF0000);  // Vermelho
        redButton->SetHoverColor(0xFFCC0000);   // Vermelho escuro
        redButton->SetPressedColor(0xFF990000); // Vermelho mais escuro
        redButton->SetOnClick([](const UI::ButtonClickEvent& event) {
            Core::Log("[UI Test] Botão vermelho clicado!");
        });
        root->AddChild(redButton);
        
        // Botão verde
        auto greenButton = std::make_shared<UI::Button>(uiContext.get());
        greenButton->SetText("Botão Verde");
        greenButton->SetPosition({50.0f, 120.0f});
        greenButton->SetSize({200.0f, 50.0f});
        greenButton->SetNormalColor(0xFF00FF00);  // Verde
        greenButton->SetHoverColor(0xFF00CC00);   // Verde escuro
        greenButton->SetPressedColor(0xFF009900); // Verde mais escuro
        greenButton->SetOnClick([](const UI::ButtonClickEvent& event) {
            Core::Log("[UI Test] Botão verde clicado!");
        });
        root->AddChild(greenButton);
        
        // Botão azul
        auto blueButton = std::make_shared<UI::Button>(uiContext.get());
        blueButton->SetText("Botão Azul");
        blueButton->SetPosition({50.0f, 190.0f});
        blueButton->SetSize({200.0f, 50.0f});
        blueButton->SetNormalColor(0xFF0000FF);  // Azul
        blueButton->SetHoverColor(0xFF0000CC);   // Azul escuro
        blueButton->SetPressedColor(0xFF000099); // Azul mais escuro
        blueButton->SetOnClick([](const UI::ButtonClickEvent& event) {
            Core::Log("[UI Test] Botão azul clicado!");
        });
        root->AddChild(blueButton);
        
        // Botão Quit
        auto quitButton = std::make_shared<UI::Button>(uiContext.get());
        quitButton->SetText("Sair");
        quitButton->SetPosition({50.0f, 260.0f});
        quitButton->SetSize({200.0f, 50.0f});
        quitButton->SetNormalColor(0xFF666666);  // Cinza
        quitButton->SetHoverColor(0xFF444444);   // Cinza escuro
        quitButton->SetPressedColor(0xFF222222); // Cinza mais escuro
        quitButton->SetOnClick([](const UI::ButtonClickEvent& event) {
            Core::Log("[UI Test] Botão Sair clicado!");
            glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
        });
        root->AddChild(quitButton);
        
        Core::Log("[UI Test] Botões criados. Teste movendo o mouse e clicando!");
        
        // ================================
        // 7. LOOP PRINCIPAL
        // ================================
        
        double lastTime = glfwGetTime();
        
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            // Timing
            double now = glfwGetTime();
            float deltaTime = float(now - lastTime);
            lastTime = now;
            
            // Input
            inputManager->Update();
            
            // UI Update
            uiContext->Update(deltaTime);
            
            // Render
            context->Clear(0.1f, 0.1f, 0.1f, 1.0f); // Fundo escuro
            
            // UI Render
            uiBatcher->Begin();
            uiContext->Render(*uiBatcher);
            uiBatcher->End();
            
            // Present
            context->Present();
        }
        
        // ================================
        // 8. CLEANUP
        // ================================
        
        Core::Log("[UI Test] Finalizando...");
        uiContext->Shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        
        Core::Log("[UI Test] Teste concluído com sucesso!");
        return 0;
        
    } catch (const std::exception& e) {
        MessageBoxA(nullptr,
            ("Erro no teste da UI: " + std::string(e.what())).c_str(),
            "UI Test Error",
            MB_ICONERROR);
        return -1;
    }
} 