#include "Drift/Core/Log.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/Widgets/Panel.h"
#include "Drift/UI/Widgets/StackPanel.h"
#include "Drift/UI/Widgets/Grid.h"
#include "Drift/UI/LayoutTypes.h"
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
#include <iostream>

using namespace Drift;

void TestLayoutSystem(UI::UIContext* uiContext)
{
    std::cout << "=== Teste do Sistema de Layout ===" << std::endl;
    
    // Configura tamanho da tela
    uiContext->SetScreenSize(800.0f, 600.0f);
    
    // ================================
    // TESTE SIMPLES - VERIFICAR SE A UI ESTÁ FUNCIONANDO
    // ================================
    
    // Container principal simples
    auto mainContainer = std::make_shared<UI::Panel>(uiContext);
    mainContainer->SetName("MainContainer");
    mainContainer->SetPosition({50.0f, 50.0f});
    mainContainer->SetSize({700.0f, 500.0f});
    mainContainer->SetColor(0xFF2A2A2A); // Cinza escuro
    
    // Configura layout do container principal
    UI::LayoutProperties mainLayout;
    mainLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
    mainLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    mainLayout.layoutType = UI::LayoutType::Absolute;
    mainContainer->SetLayoutProperties(mainLayout);
    
    uiContext->GetRoot()->AddChild(mainContainer);
    
    Core::Log("[Layout Test] MainContainer criado: pos=(" + std::to_string(mainContainer->GetPosition().x) + 
              ", " + std::to_string(mainContainer->GetPosition().y) + "), size=(" + 
              std::to_string(mainContainer->GetSize().x) + ", " + std::to_string(mainContainer->GetSize().y) + ")");
    
    // ================================
    // BOTÕES SIMPLES PARA TESTE
    // ================================
    
    // Botão 1 - Vermelho
    auto button1 = std::make_shared<UI::Button>(uiContext);
    button1->SetName("Button1");
    button1->SetText("Botão 1");
    button1->SetPosition({100.0f, 100.0f});
    button1->SetSize({150.0f, 50.0f});
    button1->SetNormalColor(0xFFFF0000); // Vermelho
    button1->SetHoverColor(0xFFFF4444);
    button1->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Layout Test] Botão 1 clicado!");
    });
    
    UI::LayoutProperties btn1Layout;
    btn1Layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
    btn1Layout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    btn1Layout.layoutType = UI::LayoutType::Absolute;
    button1->SetLayoutProperties(btn1Layout);
    
    mainContainer->AddChild(button1);
    Core::Log("[Layout Test] Button1 criado: pos=(" + std::to_string(button1->GetPosition().x) + 
              ", " + std::to_string(button1->GetPosition().y) + "), size=(" + 
              std::to_string(button1->GetSize().x) + ", " + std::to_string(button1->GetSize().y) + ")");
    
    // Botão 2 - Verde
    auto button2 = std::make_shared<UI::Button>(uiContext);
    button2->SetName("Button2");
    button2->SetText("Botão 2");
    button2->SetPosition({300.0f, 100.0f});
    button2->SetSize({150.0f, 50.0f});
    button2->SetNormalColor(0xFF00FF00); // Verde
    button2->SetHoverColor(0xFF44FF44);
    button2->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Layout Test] Botão 2 clicado!");
    });
    
    UI::LayoutProperties btn2Layout;
    btn2Layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
    btn2Layout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    btn2Layout.layoutType = UI::LayoutType::Absolute;
    button2->SetLayoutProperties(btn2Layout);
    
    mainContainer->AddChild(button2);
    Core::Log("[Layout Test] Button2 criado: pos=(" + std::to_string(button2->GetPosition().x) + 
              ", " + std::to_string(button2->GetPosition().y) + "), size=(" + 
              std::to_string(button2->GetSize().x) + ", " + std::to_string(button2->GetSize().y) + ")");
    
    // Botão 3 - Azul
    auto button3 = std::make_shared<UI::Button>(uiContext);
    button3->SetName("Button3");
    button3->SetText("Botão 3");
    button3->SetPosition({500.0f, 100.0f});
    button3->SetSize({150.0f, 50.0f});
    button3->SetNormalColor(0xFF0000FF); // Azul
    button3->SetHoverColor(0xFF4444FF);
    button3->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Layout Test] Botão 3 clicado!");
    });
    
    UI::LayoutProperties btn3Layout;
    btn3Layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
    btn3Layout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    btn3Layout.layoutType = UI::LayoutType::Absolute;
    button3->SetLayoutProperties(btn3Layout);
    
    mainContainer->AddChild(button3);
    
    // Botão Sair - Preto
    auto quitButton = std::make_shared<UI::Button>(uiContext);
    quitButton->SetName("QuitButton");
    quitButton->SetText("Sair");
    quitButton->SetPosition({300.0f, 300.0f});
    quitButton->SetSize({100.0f, 40.0f});
    quitButton->SetNormalColor(0xFF666666);
    quitButton->SetHoverColor(0xFF888888);
    quitButton->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Layout Test] Saindo...");
        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
    });
    
    UI::LayoutProperties quitLayout;
    quitLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
    quitLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    quitLayout.layoutType = UI::LayoutType::Absolute;
    quitButton->SetLayoutProperties(quitLayout);
    
    mainContainer->AddChild(quitButton);
    
    std::cout << "Layout simples criado! Teste os botões:" << std::endl;
    std::cout << "- Botão 1 (Vermelho): posição (100, 100)" << std::endl;
    std::cout << "- Botão 2 (Verde): posição (300, 100)" << std::endl;
    std::cout << "- Botão 3 (Azul): posição (500, 100)" << std::endl;
    std::cout << "- Botão Sair (Cinza): posição (300, 300)" << std::endl;
    
    // Simula algumas atualizações
    for (int i = 0; i < 3; ++i) {
        uiContext->Update(1.0f / 60.0f);
        std::cout << "Frame " << (i + 1) << " processado" << std::endl;
    }
    
    std::cout << "\n=== Teste Concluído ===" << std::endl;
}

int main() {
    Core::Log("[Layout Test] ==========================================");
    Core::Log("[Layout Test] INICIANDO TESTE DO SISTEMA DE LAYOUT");
    Core::Log("[Layout Test] ==========================================");

    // ================================
    // 1. INICIALIZAÇÃO DO GLFW
    // ================================
    if (!glfwInit()) {
        Core::Log("[Layout Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }

    // ================================
    // 2. CRIAÇÃO DA JANELA
    // ================================
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
    
    // Configura tamanho da tela no UIBatcher
    uiBatcher->SetScreenSize(800.0f, 600.0f);

    // ================================
    // 7. CRIAÇÃO DOS LAYOUTS
    // ================================
    Core::Log("[Layout Test] Chamando TestLayoutSystem...");
    TestLayoutSystem(uiContext.get());
    Core::Log("[Layout Test] TestLayoutSystem concluído!");
    
    // Teste simples: adiciona um retângulo diretamente ao UIBatcher
    Core::Log("[Layout Test] Testando renderização direta...");
    uiBatcher->Begin();
    uiBatcher->AddRect(100.0f, 100.0f, 200.0f, 100.0f, 0xFFFF0000); // Retângulo vermelho
    uiBatcher->End();
    Core::Log("[Layout Test] Teste direto concluído!");
    
    // Teste de renderização no loop principal
    Core::Log("[Layout Test] Iniciando loop principal...");

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

        // Verifica redimensionamento da janela
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        // Atualiza tamanho da tela se mudou
        static int lastWidth = width;
        static int lastHeight = height;
        if (width != lastWidth || height != lastHeight) {
            Core::Log("[Layout Test] Janela redimensionada: " + std::to_string(width) + "x" + std::to_string(height));
            
            // Atualiza tamanho da tela no UIContext
            uiContext->SetScreenSize(static_cast<float>(width), static_cast<float>(height));
            
            // Atualiza tamanho da tela no UIBatcher
            uiBatcher->SetScreenSize(static_cast<float>(width), static_cast<float>(height));
            
            lastWidth = width;
            lastHeight = height;
        }

        // Update
        inputManager->Update();
        uiContext->Update(deltaTime);

        // Render
        context->Clear(0.1f, 0.1f, 0.1f, 1.0f);
        
        // UI Render
        uiBatcher->Begin();
        Core::Log("[Layout Test] Renderizando UI...");
        uiContext->Render(*uiBatcher);
        
        // Teste: adiciona um retângulo azul no loop principal
        uiBatcher->AddRect(400.0f, 100.0f, 150.0f, 100.0f, 0xFF0000FF); // Retângulo azul
        
        Core::Log("[Layout Test] UI renderizada!");
        uiBatcher->End();

        // Present
        context->Present();
    }

    // ================================
    // 9. FINALIZAÇÃO
    // ================================
    Core::Log("[Layout Test] Finalizando...");
    uiContext->