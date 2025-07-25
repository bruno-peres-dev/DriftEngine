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
    // Configura tamanho inicial da tela
    uiContext->SetScreenSize(800.0f, 600.0f);
    
    // ========================================
    // CONTAINER PRINCIPAL COM MARGEM
    // ========================================
    auto mainContainer = std::make_shared<UI::Panel>(uiContext);
    mainContainer->SetName("MainContainer");
    mainContainer->SetPosition({0.0f, 0.0f}); // Posição inicial
    mainContainer->SetSize({800.0f, 600.0f}); // Tamanho inicial
    mainContainer->SetColor(0xFF222222); // Fundo escuro
    
    // Layout responsivo que se adapta ao redimensionamento
    UI::LayoutProperties mainLayout;
    mainLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch; // Responsivo
    mainLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch; // Responsivo
    mainLayout.layoutType = UI::LayoutType::Stack;
    mainLayout.stackDirection = UI::StackDirection::Vertical;
    mainLayout.stackSpacing = 10.0f; // Espaçamento entre elementos
    mainLayout.margin = UI::LayoutMargins(20.0f).ToVec4(); // Margem externa
    mainLayout.padding = UI::LayoutMargins(15.0f).ToVec4(); // Padding interno
    mainContainer->SetLayoutProperties(mainLayout);
    
    uiContext->GetRoot()->AddChild(mainContainer);
    
    // ========================================
    // CINCO BOTÕES QUE OCUPAM TODA A LARGURA
    // ========================================
    
    std::vector<std::string> buttonTexts = {
        "Botão Principal",
        "Botão Secundário", 
        "Botão de Ação",
        "Botão de Configuração",
        "Botão de Sair"
    };
    
    std::vector<uint32_t> buttonColors = {
        0xFF4CAF50, // Verde
        0xFF2196F3, // Azul
        0xFFFF9800, // Laranja
        0xFF9C27B0, // Roxo
        0xFFF44336  // Vermelho
    };
    
    for (size_t i = 0; i < buttonTexts.size(); ++i) {
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("FullWidthButton" + std::to_string(i + 1));
        button->SetText(buttonTexts[i]);
        button->SetSize({200.0f, 40.0f}); // Tamanho base, será esticado
        button->SetNormalColor(buttonColors[i]);
        
        // Cores de hover e pressed calculadas de forma segura
        uint32_t baseColor = buttonColors[i];
        
        // Extrai componentes RGB
        uint8_t r = (baseColor >> 16) & 0xFF;
        uint8_t g = (baseColor >> 8) & 0xFF;
        uint8_t b = baseColor & 0xFF;
        
        // Calcula cores de hover (mais claras)
        uint8_t hoverR = (r + 40 > 255) ? 255 : r + 40;
        uint8_t hoverG = (g + 40 > 255) ? 255 : g + 40;
        uint8_t hoverB = (b + 40 > 255) ? 255 : b + 40;
        uint32_t hoverColor = 0xFF000000 | (hoverR << 16) | (hoverG << 8) | hoverB;
        
        // Calcula cores de pressed (mais escuras)
        uint8_t pressedR = (r < 40) ? 0 : r - 40;
        uint8_t pressedG = (g < 40) ? 0 : g - 40;
        uint8_t pressedB = (b < 40) ? 0 : b - 40;
        uint32_t pressedColor = 0xFF000000 | (pressedR << 16) | (pressedG << 8) | pressedB;
        
        button->SetHoverColor(hoverColor);
        button->SetPressedColor(pressedColor);
        
        // Layout que ocupa toda a largura horizontal
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
        buttonLayout.layoutType = UI::LayoutType::Absolute;
        buttonLayout.margin = UI::LayoutMargins(0.0f, 5.0f, 0.0f, 5.0f).ToVec4(); // Espaçamento vertical entre botões
        button->SetLayoutProperties(buttonLayout);
        
        button->SetOnClick([buttonTexts, i](const UI::ButtonClickEvent& event) {
            Core::Log("[UI] " + buttonTexts[i] + " clicado!");
        });
        
        mainContainer->AddChild(button);
    }
    
    // Processa alguns frames para aplicar o layout
    for (int i = 0; i < 3; ++i) {
        uiContext->Update(1.0f / 60.0f);
    }
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