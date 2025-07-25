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

void TestClippingSystem(UI::UIContext* uiContext)
{
    // Configura tamanho inicial da tela
    uiContext->SetScreenSize(800.0f, 600.0f);
    
    // ========================================
    // CONTAINER PRINCIPAL COM CLIPPING
    // ========================================
    auto mainContainer = std::make_shared<UI::Panel>(uiContext);
    mainContainer->SetName("MainContainer");
    mainContainer->SetPosition({0.0f, 0.0f});
    mainContainer->SetSize({400.0f, 300.0f}); // Container menor para testar clipping
    mainContainer->SetColor(0xFF222222); // Fundo escuro
    mainContainer->SetBorderWidth(2.0f); // Borda visível para testar
    mainContainer->SetBorderColor(0xFFFF0000); // Borda vermelha
    
    // Layout com clipping habilitado
    UI::LayoutProperties mainLayout;
    mainLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    mainLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    mainLayout.layoutType = UI::LayoutType::Stack;
    mainLayout.stackDirection = UI::StackDirection::Vertical;
    mainLayout.stackSpacing = 5.0f;
    mainLayout.margin = UI::LayoutMargins(10.0f).ToVec4(); // Margem grande para testar
    mainLayout.padding = UI::LayoutMargins(15.0f).ToVec4(); // Padding grande para testar
    mainLayout.clipContent = true; // HABILITA CLIPPING
    mainContainer->SetLayoutProperties(mainLayout);
    
    uiContext->GetRoot()->AddChild(mainContainer);
    
    // ========================================
    // BOTÕES QUE ULTRAPASSAM OS LIMITES
    // ========================================
    
    // Botão 1: Tamanho normal
    auto button1 = std::make_shared<UI::Button>(uiContext);
    button1->SetName("Button1");
    button1->SetText("Botão Normal");
    button1->SetSize({200.0f, 40.0f});
    button1->SetNormalColor(0xFF4CAF50); // Verde
    
    UI::LayoutProperties button1Layout;
    button1Layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    button1Layout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    button1Layout.layoutType = UI::LayoutType::None;
    button1Layout.margin = UI::LayoutMargins(5.0f).ToVec4();
    button1->SetLayoutProperties(button1Layout);
    
    button1->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Clipping Test] Botão Normal clicado!");
    });
    
    mainContainer->AddChild(button1);
    
    // Botão 2: Muito largo (deve ser cortado)
    auto button2 = std::make_shared<UI::Button>(uiContext);
    button2->SetName("Button2");
    button2->SetText("Botão Muito Largo");
    button2->SetSize({600.0f, 40.0f}); // Maior que o container
    button2->SetNormalColor(0xFF2196F3); // Azul
    
    UI::LayoutProperties button2Layout;
    button2Layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    button2Layout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    button2Layout.layoutType = UI::LayoutType::None;
    button2Layout.margin = UI::LayoutMargins(5.0f).ToVec4();
    button2->SetLayoutProperties(button2Layout);
    
    button2->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Clipping Test] Botão Largo clicado!");
    });
    
    mainContainer->AddChild(button2);
    
    // Botão 3: Muito alto (deve ser cortado)
    auto button3 = std::make_shared<UI::Button>(uiContext);
    button3->SetName("Button3");
    button3->SetText("Botão Muito Alto");
    button3->SetSize({200.0f, 200.0f}); // Maior que o container
    button3->SetNormalColor(0xFFFF9800); // Laranja
    
    UI::LayoutProperties button3Layout;
    button3Layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    button3Layout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    button3Layout.layoutType = UI::LayoutType::None;
    button3Layout.margin = UI::LayoutMargins(5.0f).ToVec4();
    button3->SetLayoutProperties(button3Layout);
    
    button3->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Clipping Test] Botão Alto clicado!");
    });
    
    mainContainer->AddChild(button3);
    
    // Botão 4: Posicionado fora dos limites (deve ser cortado)
    auto button4 = std::make_shared<UI::Button>(uiContext);
    button4->SetName("Button4");
    button4->SetText("Botão Fora dos Limites");
    button4->SetSize({150.0f, 40.0f});
    button4->SetPosition({500.0f, 100.0f}); // Posição fora do container
    button4->SetNormalColor(0xFFF44336); // Vermelho
    
    UI::LayoutProperties button4Layout;
    button4Layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Left;
    button4Layout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    button4Layout.layoutType = UI::LayoutType::Absolute; // Posicionamento absoluto
    button4Layout.margin = UI::LayoutMargins(5.0f).ToVec4();
    button4->SetLayoutProperties(button4Layout);
    
    button4->SetOnClick([](const UI::ButtonClickEvent& event) {
        // Este log não deve aparecer se o clipping estiver funcionando
        Core::Log("[Clipping Test][ERRO] Botão Fora dos Limites clicado!");
    });
    
    mainContainer->AddChild(button4);
    
    // ========================================
    // CONTAINER SECUNDÁRIO PARA TESTE ADICIONAL
    // ========================================
    auto subContainer = std::make_shared<UI::Panel>(uiContext);
    subContainer->SetName("SubContainer");
    subContainer->SetSize({250.0f, 100.0f}); // Aumentado para melhor visualização
    subContainer->SetColor(0xFF444444); // Cinza escuro
    subContainer->SetBorderWidth(3.0f); // Borda mais grossa para visualizar melhor
    subContainer->SetBorderColor(0xFF00FF00); // Borda verde
    
    UI::LayoutProperties subLayout;
    subLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    subLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Top;
    subLayout.layoutType = UI::LayoutType::Stack;
    subLayout.stackDirection = UI::StackDirection::Horizontal;
    subLayout.stackSpacing = 8.0f; // Espaçamento entre botões
    subLayout.margin = UI::LayoutMargins(3.0f).ToVec4(); // Margem menor
    subLayout.padding = UI::LayoutMargins(8.0f).ToVec4(); // Padding para testar
    subLayout.clipContent = true; // Clipping no sub-container também
    subContainer->SetLayoutProperties(subLayout);
    
    mainContainer->AddChild(subContainer);
    
    // Botões dentro do sub-container - 3 botões para testar overflow
    for (int i = 0; i < 3; ++i) {
        auto subButton = std::make_shared<UI::Button>(uiContext);
        subButton->SetName("SubButton" + std::to_string(i));
        subButton->SetText("Sub " + std::to_string(i + 1));
        subButton->SetSize({70.0f, 30.0f}); // Botões um pouco maiores
        subButton->SetNormalColor(0xFF9C27B0); // Roxo
        
        UI::LayoutProperties subButtonLayout;
        subButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Center;
        subButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        subButtonLayout.layoutType = UI::LayoutType::None;
        subButtonLayout.margin = UI::LayoutMargins(2.0f).ToVec4(); // Margem pequena
        subButton->SetLayoutProperties(subButtonLayout);
        
        subButton->SetOnClick([i](const UI::ButtonClickEvent& event) {
            // Apenas os botões visíveis dentro do clipping devem responder
            Core::Log("[Clipping Test] Sub Botão " + std::to_string(i + 1) + " clicado!");
        });
        
        subContainer->AddChild(subButton);
    }
}

int main() {
    Core::Log("[Clipping Test] ==========================================");
    Core::Log("[Clipping Test] INICIANDO TESTE DO SISTEMA DE CLIPPING");
    Core::Log("[Clipping Test] ==========================================");

    // ================================
    // 1. INICIALIZAÇÃO DO GLFW
    // ================================
    Core::Log("[Clipping Test] 1. Inicializando GLFW...");
    if (!glfwInit()) {
        Core::Log("[Clipping Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }
    Core::Log("[Clipping Test] 1. GLFW inicializado com sucesso!");

    // ================================
    // 2. CRIAÇÃO DA JANELA
    // ================================
    Core::Log("[Clipping Test] 2. Criando janela...");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "DriftEngine Clipping Test", nullptr, nullptr);
    if (!window) {
        Core::Log("[Clipping Test] ERRO: Falha ao criar janela!");
        glfwTerminate();
        return -1;
    }
    Core::Log("[Clipping Test] 2. Janela criada com sucesso!");

    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        Core::Log("[Clipping Test] ERRO: Falha ao obter HWND!");
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
    // 7. CRIAÇÃO DOS TESTES DE CLIPPING
    // ================================
    Core::Log("[Clipping Test] Chamando TestClippingSystem...");
    TestClippingSystem(uiContext.get());
    Core::Log("[Clipping Test] TestClippingSystem concluído!");
    
    Core::Log("[Clipping Test] Iniciando loop principal...");

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
            Core::Log("[Clipping Test] Janela redimensionada: " + std::to_string(width) + "x" + std::to_string(height));
            
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
    Core::Log("[Clipping Test] Finalizando...");
    uiContext->Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    Core::Log("[Clipping Test] Teste concluído com sucesso!");

    return 0;
} 