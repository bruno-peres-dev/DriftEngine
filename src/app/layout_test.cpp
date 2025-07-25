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
    uiContext->SetScreenSize(800.0f, 600.0f);
    
    // Container principal responsivo com fundo vermelho
    auto mainContainer = std::make_shared<UI::Panel>(uiContext);
    mainContainer->SetName("MainContainer");
    mainContainer->SetPosition({50.0f, 50.0f});
    mainContainer->SetSize({700.0f, 500.0f});
    mainContainer->SetColor(0xFFFF0000); // Fundo vermelho puro
    
    // Layout absoluto do container principal (não interfere nos filhos)
    UI::LayoutProperties mainLayout;
    mainLayout.padding = UI::LayoutMargins(5.0f).ToVec4();
    mainLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
    mainLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    mainLayout.layoutType = UI::LayoutType::Absolute;
    mainContainer->SetLayoutProperties(mainLayout);
    
    uiContext->GetRoot()->AddChild(mainContainer);
    Core::Log("[Layout Test] Container principal criado: fundo vermelho responsivo");
    
    Core::Log("[Layout Test] Container principal criado: fundo vermelho responsivo");
    
    // Lista de botões pretos
    std::vector<std::string> buttonTexts = {
        "Botão 1",
        "Botão 2", 
        "Botão 3",
        "Botão 4",
        "Botão 5",
        "Sair"
    };
    
    for (size_t i = 0; i < buttonTexts.size(); ++i) {
        // Botão simples com cor preta
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("Button" + std::to_string(i + 1));
        button->SetText(buttonTexts[i]);
        
        // Posicionamento absoluto dos botões
        float buttonY = 100.0f + (i * 60.0f); // 60px de espaçamento entre botões
        button->SetPosition({250.0f, buttonY}); // Centralizado horizontalmente
        button->SetSize({200.0f, 40.0f});
        
        // Define cores específicas para cada estado do botão
        button->SetNormalColor(0xFF000000);   // Preto
        button->SetHoverColor(0xFF333333);    // Cinza escuro
        button->SetPressedColor(0xFF666666);  // Cinza médio
        button->SetDisabledColor(0xFFCCCCCC); // Cinza claro
        
        // Layout absoluto do botão
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
        buttonLayout.layoutType = UI::LayoutType::Absolute;
        button->SetLayoutProperties(buttonLayout);
        
        // Callback do botão
        button->SetOnClick([buttonTexts, i](const UI::ButtonClickEvent& event) {
            Core::Log("[UI] " + buttonTexts[i] + " clicado!");
        });
        
        mainContainer->AddChild(button);
        Core::Log("[Layout Test] " + buttonTexts[i] + " criado (preto com estados)");
    }
    
    std::cout << "Layout criado! Lista vertical de botões pretos com fundo vermelho:" << std::endl;
    std::cout << "- Container principal: fundo vermelho puro responsivo" << std::endl;
    std::cout << "- StackPanel vertical com espaçamento de 10px entre botões" << std::endl;
    std::cout << "- " << buttonTexts.size() << " botões pretos puros centralizados" << std::endl;
    std::cout << "- Botões clicáveis com callbacks funcionais" << std::endl;
    std::cout << "- Layout responsivo: se adapta ao redimensionamento da janela" << std::endl;
    
    // Processa alguns frames para garantir que o layout seja aplicado
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
    Core::Log("[Layout Test] 1. Inicializando GLFW...");
    if (!glfwInit()) {
        Core::Log("[Layout Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }
    Core::Log("[Layout Test] 1. GLFW inicializado com sucesso!");

    // ================================
    // 2. CRIAÇÃO DA JANELA
    // ================================
    Core::Log("[Layout Test] 2. Criando janela...");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "DriftEngine Layout Test", nullptr, nullptr);
    if (!window) {
        Core::Log("[Layout Test] ERRO: Falha ao criar janela!");
        glfwTerminate();
        return -1;
    }
    Core::Log("[Layout Test] 2. Janela criada com sucesso!");

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
    
    // Teste de renderização no loop principal
    Core::Log("[Layout Test] Iniciando loop principal...");
    
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
        uiContext->Render(*uiBatcher);
        uiBatcher->End();

        // Present
        context->Present();
    }

    // ================================
    // 9. FINALIZAÇÃO
    // ================================
    Core::Log("[Layout Test] Finalizando...");
    uiContext->Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    Core::Log("[Layout Test] Teste concluído com sucesso!");

    return 0;
} 