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

void TestLayoutFixes(UI::UIContext* uiContext)
{
    // Configura tamanho inicial da tela
    uiContext->SetScreenSize(1200.0f, 800.0f);
    
    // ========================================
    // CONTAINER PRINCIPAL
    // ========================================
    auto mainContainer = std::make_shared<UI::Panel>(uiContext);
    mainContainer->SetName("MainContainer");
    mainContainer->SetPosition({0.0f, 0.0f});
    mainContainer->SetSize({1200.0f, 800.0f});
    mainContainer->SetColor(0xFF1E1E1E); // Fundo escuro
    mainContainer->SetBorderWidth(3.0f);
    mainContainer->SetBorderColor(0xFF444444);
    
    // Layout principal
    UI::LayoutProperties mainLayout;
    mainLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    mainLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    mainLayout.layoutType = UI::LayoutType::Stack;
    mainLayout.stackDirection = UI::StackDirection::Vertical;
    mainLayout.stackSpacing = 15.0f;
    mainLayout.margin = UI::LayoutMargins(20.0f).ToVec4();
    mainLayout.padding = UI::LayoutMargins(25.0f).ToVec4(); // Padding grande para testar
    mainContainer->SetLayoutProperties(mainLayout);
    
    uiContext->GetRoot()->AddChild(mainContainer);
    
    // ========================================
    // TÍTULO PRINCIPAL
    // ========================================
    auto titlePanel = std::make_shared<UI::Panel>(uiContext);
    titlePanel->SetName("TitlePanel");
    titlePanel->SetSize({1150.0f, 80.0f});
    titlePanel->SetColor(0xFF2D2D2D);
    titlePanel->SetBorderWidth(2.0f);
    titlePanel->SetBorderColor(0xFF666666);
    
    UI::LayoutProperties titleLayout;
    titleLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Center;
    titleLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
    titleLayout.layoutType = UI::LayoutType::None;
    titlePanel->SetLayoutProperties(titleLayout);
    
    mainContainer->AddChild(titlePanel);
    
    // Título
    auto titleButton = std::make_shared<UI::Button>(uiContext);
    titleButton->SetName("TitleButton");
    titleButton->SetText("Correções do Sistema de Layout - DriftEngine");
    titleButton->SetSize({800.0f, 60.0f});
    titleButton->SetNormalColor(0xFF4CAF50);
    titleButton->SetHoverColor(0xFF66BB6A);
    titleButton->SetPressedColor(0xFF388E3C);
    
    UI::LayoutProperties titleButtonLayout;
    titleButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Center;
    titleButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
    titleButtonLayout.layoutType = UI::LayoutType::None;
    titleButton->SetLayoutProperties(titleButtonLayout);
    
    titleButton->SetOnClick([](const UI::ButtonClickEvent& event) {
        Core::Log("[Layout Fixes Test] Título clicado!");
    });
    
    titlePanel->AddChild(titleButton);
    
    // ========================================
    // CONTAINER DE TESTES
    // ========================================
    auto testContainer = std::make_shared<UI::Panel>(uiContext);
    testContainer->SetName("TestContainer");
    testContainer->SetSize({1150.0f, 600.0f});
    testContainer->SetColor(0xFF252525);
    testContainer->SetBorderWidth(2.0f);
    testContainer->SetBorderColor(0xFF555555);
    
    UI::LayoutProperties testLayout;
    testLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    testLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    testLayout.layoutType = UI::LayoutType::Grid;
    testLayout.gridColumns = 2;
    testLayout.gridRows = 3;
    testLayout.gridSpacing = 15.0f;
    testLayout.margin = UI::LayoutMargins(15.0f).ToVec4();
    testLayout.padding = UI::LayoutMargins(20.0f).ToVec4(); // Padding para testar
    testContainer->SetLayoutProperties(testLayout);
    
    mainContainer->AddChild(testContainer);
    
    // ========================================
    // TESTE 1: PADDING RESPITADO
    // ========================================
    auto paddingTestPanel = std::make_shared<UI::Panel>(uiContext);
    paddingTestPanel->SetName("PaddingTestPanel");
    paddingTestPanel->SetColor(0xFF333333);
    paddingTestPanel->SetBorderWidth(3.0f);
    paddingTestPanel->SetBorderColor(0xFF00FF00); // Borda verde para destacar
    paddingTestPanel->SetProportionalBorders(true); // Bordas proporcionais
    paddingTestPanel->SetBorderProportion(0.02f); // 2% do menor lado
    
    UI::LayoutProperties paddingTestLayout;
    paddingTestLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    paddingTestLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    paddingTestLayout.layoutType = UI::LayoutType::Stack;
    paddingTestLayout.stackDirection = UI::StackDirection::Vertical;
    paddingTestLayout.stackSpacing = 8.0f;
    paddingTestLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    paddingTestLayout.padding = UI::LayoutMargins(15.0f).ToVec4(); // Padding interno
    paddingTestPanel->SetLayoutProperties(paddingTestLayout);
    
    testContainer->AddChild(paddingTestPanel);
    
    // Botões que devem respeitar o padding
    std::vector<std::string> paddingTestButtons = {
        "Botão 1 - Deve estar dentro do padding",
        "Botão 2 - Não deve tocar a borda",
        "Botão 3 - Padding respeitado",
        "Botão 4 - Espaçamento correto"
    };
    
    for (const auto& buttonText : paddingTestButtons) {
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("PaddingButton_" + buttonText);
        button->SetText(buttonText);
        button->SetSize({250.0f, 35.0f});
        button->SetNormalColor(0xFF2196F3);
        button->SetHoverColor(0xFF42A5F5);
        button->SetPressedColor(0xFF1976D2);
        
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        buttonLayout.layoutType = UI::LayoutType::None;
        buttonLayout.margin = UI::LayoutMargins(3.0f).ToVec4();
        button->SetLayoutProperties(buttonLayout);
        
        button->SetOnClick([buttonText](const UI::ButtonClickEvent& event) {
            Core::Log("[Layout Fixes Test] Botão de padding clicado: " + buttonText);
        });
        
        paddingTestPanel->AddChild(button);
    }
    
    // ========================================
    // TESTE 2: BORDAS PROPORCIONAIS
    // ========================================
    auto borderTestPanel = std::make_shared<UI::Panel>(uiContext);
    borderTestPanel->SetName("BorderTestPanel");
    borderTestPanel->SetColor(0xFF333333);
    borderTestPanel->SetBorderWidth(5.0f);
    borderTestPanel->SetBorderColor(0xFFFF0000); // Borda vermelha
    borderTestPanel->SetProportionalBorders(true); // Bordas proporcionais
    borderTestPanel->SetBorderProportion(0.015f); // 1.5% do menor lado
    
    UI::LayoutProperties borderTestLayout;
    borderTestLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    borderTestLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    borderTestLayout.layoutType = UI::LayoutType::Stack;
    borderTestLayout.stackDirection = UI::StackDirection::Vertical;
    borderTestLayout.stackSpacing = 8.0f;
    borderTestLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    borderTestLayout.padding = UI::LayoutMargins(12.0f).ToVec4();
    borderTestPanel->SetLayoutProperties(borderTestLayout);
    
    testContainer->AddChild(borderTestPanel);
    
    // Botões para testar bordas proporcionais
    std::vector<std::string> borderTestButtons = {
        "Bordas Proporcionais",
        "Redimensione a janela",
        "Bordas se ajustam",
        "Teste de responsividade"
    };
    
    for (const auto& buttonText : borderTestButtons) {
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("BorderButton_" + buttonText);
        button->SetText(buttonText);
        button->SetSize({250.0f, 35.0f});
        button->SetNormalColor(0xFF9C27B0);
        button->SetHoverColor(0xFFBA68C8);
        button->SetPressedColor(0xFF7B1FA2);
        
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        buttonLayout.layoutType = UI::LayoutType::None;
        buttonLayout.margin = UI::LayoutMargins(3.0f).ToVec4();
        button->SetLayoutProperties(buttonLayout);
        
        button->SetOnClick([buttonText](const UI::ButtonClickEvent& event) {
            Core::Log("[Layout Fixes Test] Botão de borda clicado: " + buttonText);
        });
        
        borderTestPanel->AddChild(button);
    }
    
    // ========================================
    // TESTE 3: CLIPPING COM PADDING
    // ========================================
    auto clippingTestPanel = std::make_shared<UI::Panel>(uiContext);
    clippingTestPanel->SetName("ClippingTestPanel");
    clippingTestPanel->SetColor(0xFF333333);
    clippingTestPanel->SetBorderWidth(4.0f);
    clippingTestPanel->SetBorderColor(0xFF00FFFF); // Borda ciano
    clippingTestPanel->SetProportionalBorders(false); // Bordas fixas
    
    UI::LayoutProperties clippingTestLayout;
    clippingTestLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    clippingTestLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    clippingTestLayout.layoutType = UI::LayoutType::Stack;
    clippingTestLayout.stackDirection = UI::StackDirection::Horizontal;
    clippingTestLayout.stackSpacing = 10.0f;
    clippingTestLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    clippingTestLayout.padding = UI::LayoutMargins(18.0f).ToVec4();
    clippingTestLayout.clipContent = true; // Habilita clipping
    clippingTestPanel->SetLayoutProperties(clippingTestLayout);
    
    testContainer->AddChild(clippingTestPanel);
    
    // Botões que devem ser cortados pelo clipping
    std::vector<std::string> clippingTestButtons = {
        "Clipping",
        "Funciona",
        "Com",
        "Padding"
    };
    
    for (const auto& buttonText : clippingTestButtons) {
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("ClippingButton_" + buttonText);
        button->SetText(buttonText);
        button->SetSize({120.0f, 35.0f});
        button->SetNormalColor(0xFFFF9800);
        button->SetHoverColor(0xFFFFB74D);
        button->SetPressedColor(0xFFF57C00);
        
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Center;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        buttonLayout.layoutType = UI::LayoutType::None;
        buttonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        button->SetLayoutProperties(buttonLayout);
        
        button->SetOnClick([buttonText](const UI::ButtonClickEvent& event) {
            Core::Log("[Layout Fixes Test] Botão de clipping clicado: " + buttonText);
        });
        
        clippingTestPanel->AddChild(button);
    }
    
    // ========================================
    // TESTE 4: MARGENS E PADDING COMBINADOS
    // ========================================
    auto marginPaddingTestPanel = std::make_shared<UI::Panel>(uiContext);
    marginPaddingTestPanel->SetName("MarginPaddingTestPanel");
    marginPaddingTestPanel->SetColor(0xFF333333);
    marginPaddingTestPanel->SetBorderWidth(3.0f);
    marginPaddingTestPanel->SetBorderColor(0xFFFFFF00); // Borda amarela
    marginPaddingTestPanel->SetProportionalBorders(true);
    marginPaddingTestPanel->SetBorderProportion(0.01f);
    
    UI::LayoutProperties marginPaddingTestLayout;
    marginPaddingTestLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    marginPaddingTestLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    marginPaddingTestLayout.layoutType = UI::LayoutType::Stack;
    marginPaddingTestLayout.stackDirection = UI::StackDirection::Vertical;
    marginPaddingTestLayout.stackSpacing = 6.0f;
    marginPaddingTestLayout.margin = UI::LayoutMargins(8.0f).ToVec4(); // Margens externas
    marginPaddingTestLayout.padding = UI::LayoutMargins(20.0f).ToVec4(); // Padding interno
    marginPaddingTestPanel->SetLayoutProperties(marginPaddingTestLayout);
    
    testContainer->AddChild(marginPaddingTestPanel);
    
    // Botões com margens e padding
    std::vector<std::string> marginPaddingButtons = {
        "Margens + Padding",
        "Espaçamento Duplo",
        "Layout Correto",
        "Sem Sobreposição"
    };
    
    for (const auto& buttonText : marginPaddingButtons) {
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("MarginPaddingButton_" + buttonText);
        button->SetText(buttonText);
        button->SetSize({250.0f, 30.0f});
        button->SetNormalColor(0xFF4CAF50);
        button->SetHoverColor(0xFF66BB6A);
        button->SetPressedColor(0xFF388E3C);
        
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        buttonLayout.layoutType = UI::LayoutType::None;
        buttonLayout.margin = UI::LayoutMargins(4.0f).ToVec4(); // Margens do botão
        button->SetLayoutProperties(buttonLayout);
        
        button->SetOnClick([buttonText](const UI::ButtonClickEvent& event) {
            Core::Log("[Layout Fixes Test] Botão de margem/padding clicado: " + buttonText);
        });
        
        marginPaddingTestPanel->AddChild(button);
    }
    
    // ========================================
    // TESTE 5: LAYOUT RESPONSIVO
    // ========================================
    auto responsiveTestPanel = std::make_shared<UI::Panel>(uiContext);
    responsiveTestPanel->SetName("ResponsiveTestPanel");
    responsiveTestPanel->SetColor(0xFF333333);
    responsiveTestPanel->SetBorderWidth(2.0f);
    responsiveTestPanel->SetBorderColor(0xFFFF00FF); // Borda magenta
    responsiveTestPanel->SetProportionalBorders(true);
    responsiveTestPanel->SetBorderProportion(0.008f);
    
    UI::LayoutProperties responsiveTestLayout;
    responsiveTestLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    responsiveTestLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    responsiveTestLayout.layoutType = UI::LayoutType::Grid;
    responsiveTestLayout.gridColumns = 2;
    responsiveTestLayout.gridRows = 2;
    responsiveTestLayout.gridSpacing = 8.0f;
    responsiveTestLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    responsiveTestLayout.padding = UI::LayoutMargins(15.0f).ToVec4();
    responsiveTestPanel->SetLayoutProperties(responsiveTestLayout);
    
    testContainer->AddChild(responsiveTestPanel);
    
    // Botões responsivos
    std::vector<std::string> responsiveButtons = {
        "Grid 1x1",
        "Grid 1x2",
        "Grid 2x1",
        "Grid 2x2"
    };
    
    for (const auto& buttonText : responsiveButtons) {
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("ResponsiveButton_" + buttonText);
        button->SetText(buttonText);
        button->SetSize({120.0f, 40.0f});
        button->SetNormalColor(0xFF607D8B);
        button->SetHoverColor(0xFF78909C);
        button->SetPressedColor(0xFF455A64);
        
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
        buttonLayout.layoutType = UI::LayoutType::None;
        buttonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        button->SetLayoutProperties(buttonLayout);
        
        button->SetOnClick([buttonText](const UI::ButtonClickEvent& event) {
            Core::Log("[Layout Fixes Test] Botão responsivo clicado: " + buttonText);
        });
        
        responsiveTestPanel->AddChild(button);
    }
    
    // ========================================
    // TESTE 6: CONTROLES
    // ========================================
    auto controlsTestPanel = std::make_shared<UI::Panel>(uiContext);
    controlsTestPanel->SetName("ControlsTestPanel");
    controlsTestPanel->SetColor(0xFF333333);
    controlsTestPanel->SetBorderWidth(3.0f);
    controlsTestPanel->SetBorderColor(0xFF00FF00);
    controlsTestPanel->SetProportionalBorders(false);
    
    UI::LayoutProperties controlsTestLayout;
    controlsTestLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    controlsTestLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    controlsTestLayout.layoutType = UI::LayoutType::Stack;
    controlsTestLayout.stackDirection = UI::StackDirection::Vertical;
    controlsTestLayout.stackSpacing = 6.0f;
    controlsTestLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    controlsTestLayout.padding = UI::LayoutMargins(12.0f).ToVec4();
    controlsTestPanel->SetLayoutProperties(controlsTestLayout);
    
    testContainer->AddChild(controlsTestPanel);
    
    // Botões de controle
    std::vector<std::string> controlButtons = {
        "Alternar Bordas Proporcionais",
        "Aumentar Padding",
        "Diminuir Padding",
        "Reset Layout"
    };
    
    for (const auto& buttonText : controlButtons) {
        auto button = std::make_shared<UI::Button>(uiContext);
        button->SetName("ControlButton_" + buttonText);
        button->SetText(buttonText);
        button->SetSize({250.0f, 30.0f});
        button->SetNormalColor(0xFFF44336);
        button->SetHoverColor(0xFFEF5350);
        button->SetPressedColor(0xFFD32F2F);
        
        UI::LayoutProperties buttonLayout;
        buttonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        buttonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        buttonLayout.layoutType = UI::LayoutType::None;
        buttonLayout.margin = UI::LayoutMargins(3.0f).ToVec4();
        button->SetLayoutProperties(buttonLayout);
        
        button->SetOnClick([buttonText](const UI::ButtonClickEvent& event) {
            Core::Log("[Layout Fixes Test] Controle: " + buttonText);
        });
        
        controlsTestPanel->AddChild(button);
    }
}

int main() {
    Core::Log("[Layout Fixes Test] ==========================================");
    Core::Log("[Layout Fixes Test] INICIANDO TESTE DAS CORREÇÕES DE LAYOUT");
    Core::Log("[Layout Fixes Test] ==========================================");

    // ================================
    // 1. INICIALIZAÇÃO DO GLFW
    // ================================
    Core::Log("[Layout Fixes Test] 1. Inicializando GLFW...");
    if (!glfwInit()) {
        Core::Log("[Layout Fixes Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }
    Core::Log("[Layout Fixes Test] 1. GLFW inicializado com sucesso!");

    // ================================
    // 2. CRIAÇÃO DA JANELA
    // ================================
    Core::Log("[Layout Fixes Test] 2. Criando janela...");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1200, 800, "DriftEngine Layout Fixes Test", nullptr, nullptr);
    if (!window) {
        Core::Log("[Layout Fixes Test] ERRO: Falha ao criar janela!");
        glfwTerminate();
        return -1;
    }
    Core::Log("[Layout Fixes Test] 2. Janela criada com sucesso!");

    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        Core::Log("[Layout Fixes Test] ERRO: Falha ao obter HWND!");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // ================================
    // 3. INICIALIZAÇÃO DO DIRECTX 11
    // ================================
    RHI::DeviceDesc desc{ 1200, 800, false };
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
    uiBatcher->SetScreenSize(1200.0f, 800.0f);

    // ================================
    // 7. CRIAÇÃO DOS TESTES DE LAYOUT
    // ================================
    Core::Log("[Layout Fixes Test] Chamando TestLayoutFixes...");
    TestLayoutFixes(uiContext.get());
    Core::Log("[Layout Fixes Test] TestLayoutFixes concluído!");
    
    Core::Log("[Layout Fixes Test] Iniciando loop principal...");

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
            Core::Log("[Layout Fixes Test] Janela redimensionada: " + std::to_string(width) + "x" + std::to_string(height));
            
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
    Core::Log("[Layout Fixes Test] Finalizando...");
    uiContext->Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    Core::Log("[Layout Fixes Test] Teste concluído com sucesso!");

    return 0;
} 