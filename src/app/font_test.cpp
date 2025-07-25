#include "Drift/Core/Log.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/Widgets/Panel.h"
#include "Drift/UI/Widgets/StackPanel.h"
#include "Drift/UI/Widgets/Grid.h"
#include "Drift/UI/LayoutTypes.h"
#include "Drift/UI/FontSystem/FontManager.h"
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

void TestFontSystem(UI::UIContext* uiContext)
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
    mainContainer->SetBorderWidth(2.0f);
    mainContainer->SetBorderColor(0xFF444444);
    
    // Layout principal
    UI::LayoutProperties mainLayout;
    mainLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    mainLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    mainLayout.layoutType = UI::LayoutType::Stack;
    mainLayout.stackDirection = UI::StackDirection::Vertical;
    mainLayout.stackSpacing = 10.0f;
    mainLayout.margin = UI::LayoutMargins(20.0f).ToVec4();
    mainLayout.padding = UI::LayoutMargins(15.0f).ToVec4();
    mainContainer->SetLayoutProperties(mainLayout);
    
    uiContext->GetRoot()->AddChild(mainContainer);
    
    // ========================================
    // TÍTULO PRINCIPAL
    // ========================================
    auto titlePanel = std::make_shared<UI::Panel>(uiContext);
    titlePanel->SetName("TitlePanel");
    titlePanel->SetSize({1160.0f, 80.0f});
    titlePanel->SetColor(0xFF2D2D2D);
    titlePanel->SetBorderWidth(1.0f);
    titlePanel->SetBorderColor(0xFF666666);
    
    UI::LayoutProperties titleLayout;
    titleLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Center;
    titleLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
    titleLayout.layoutType = UI::LayoutType::None;
    titlePanel->SetLayoutProperties(titleLayout);
    
    mainContainer->AddChild(titlePanel);
    
    // Título com texto de demonstração
    auto titleButton = std::make_shared<UI::Button>(uiContext);
    titleButton->SetName("TitleButton");
    titleButton->SetText("Sistema de Fontes Profissional - DriftEngine");
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
        Core::Log("[Font Test] Título clicado!");
    });
    
    titlePanel->AddChild(titleButton);
    
    // ========================================
    // CONTAINER DE DEMONSTRAÇÕES
    // ========================================
    auto demoContainer = std::make_shared<UI::Panel>(uiContext);
    demoContainer->SetName("DemoContainer");
    demoContainer->SetSize({1160.0f, 600.0f});
    demoContainer->SetColor(0xFF252525);
    demoContainer->SetBorderWidth(1.0f);
    demoContainer->SetBorderColor(0xFF555555);
    
    UI::LayoutProperties demoLayout;
    demoLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    demoLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    demoLayout.layoutType = UI::LayoutType::Grid;
    demoLayout.gridColumns = 2;
    demoLayout.gridRows = 3;
    demoLayout.gridSpacing = 10.0f;
    demoLayout.margin = UI::LayoutMargins(10.0f).ToVec4();
    demoLayout.padding = UI::LayoutMargins(10.0f).ToVec4();
    demoContainer->SetLayoutProperties(demoLayout);
    
    mainContainer->AddChild(demoContainer);
    
    // ========================================
    // DEMONSTRAÇÃO 1: DIFERENTES TAMANHOS
    // ========================================
    auto sizeDemoPanel = std::make_shared<UI::Panel>(uiContext);
    sizeDemoPanel->SetName("SizeDemoPanel");
    sizeDemoPanel->SetColor(0xFF333333);
    sizeDemoPanel->SetBorderWidth(1.0f);
    sizeDemoPanel->SetBorderColor(0xFF777777);
    
    UI::LayoutProperties sizeDemoLayout;
    sizeDemoLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    sizeDemoLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    sizeDemoLayout.layoutType = UI::LayoutType::Stack;
    sizeDemoLayout.stackDirection = UI::StackDirection::Vertical;
    sizeDemoLayout.stackSpacing = 5.0f;
    sizeDemoLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    sizeDemoLayout.padding = UI::LayoutMargins(8.0f).ToVec4();
    sizeDemoPanel->SetLayoutProperties(sizeDemoLayout);
    
    demoContainer->AddChild(sizeDemoPanel);
    
    // Botões com diferentes tamanhos de fonte
    std::vector<std::pair<std::string, float>> sizeExamples = {
        {"Pequeno (12px)", 12.0f},
        {"Normal (16px)", 16.0f},
        {"Médio (20px)", 20.0f},
        {"Grande (24px)", 24.0f},
        {"Extra Grande (32px)", 32.0f}
    };
    
    for (const auto& example : sizeExamples) {
        auto sizeButton = std::make_shared<UI::Button>(uiContext);
        sizeButton->SetName("SizeButton_" + std::to_string(static_cast<int>(example.second)));
        sizeButton->SetText(example.first);
        sizeButton->SetSize({280.0f, 35.0f});
        sizeButton->SetNormalColor(0xFF2196F3);
        sizeButton->SetHoverColor(0xFF42A5F5);
        sizeButton->SetPressedColor(0xFF1976D2);
        
        UI::LayoutProperties sizeButtonLayout;
        sizeButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        sizeButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        sizeButtonLayout.layoutType = UI::LayoutType::None;
        sizeButtonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        sizeButton->SetLayoutProperties(sizeButtonLayout);
        
        sizeButton->SetOnClick([example](const UI::ButtonClickEvent& event) {
            Core::Log("[Font Test] Botão de tamanho " + std::to_string(static_cast<int>(example.second)) + "px clicado!");
        });
        
        sizeDemoPanel->AddChild(sizeButton);
    }
    
    // ========================================
    // DEMONSTRAÇÃO 2: DIFERENTES CORES
    // ========================================
    auto colorDemoPanel = std::make_shared<UI::Panel>(uiContext);
    colorDemoPanel->SetName("ColorDemoPanel");
    colorDemoPanel->SetColor(0xFF333333);
    colorDemoPanel->SetBorderWidth(1.0f);
    colorDemoPanel->SetBorderColor(0xFF777777);
    
    UI::LayoutProperties colorDemoLayout;
    colorDemoLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    colorDemoLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    colorDemoLayout.layoutType = UI::LayoutType::Stack;
    colorDemoLayout.stackDirection = UI::StackDirection::Vertical;
    colorDemoLayout.stackSpacing = 5.0f;
    colorDemoLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    colorDemoLayout.padding = UI::LayoutMargins(8.0f).ToVec4();
    colorDemoPanel->SetLayoutProperties(colorDemoLayout);
    
    demoContainer->AddChild(colorDemoPanel);
    
    // Botões com diferentes cores
    std::vector<std::pair<std::string, unsigned>> colorExamples = {
        {"Texto Branco", 0xFFFFFFFF},
        {"Texto Vermelho", 0xFFFF4444},
        {"Texto Verde", 0xFF44FF44},
        {"Texto Azul", 0xFF4444FF},
        {"Texto Amarelo", 0xFFFFFF44},
        {"Texto Ciano", 0xFF44FFFF},
        {"Texto Magenta", 0xFFFF44FF}
    };
    
    for (const auto& example : colorExamples) {
        auto colorButton = std::make_shared<UI::Button>(uiContext);
        colorButton->SetName("ColorButton_" + example.first);
        colorButton->SetText(example.first);
        colorButton->SetSize({280.0f, 30.0f});
        colorButton->SetNormalColor(0xFF424242);
        colorButton->SetHoverColor(0xFF555555);
        colorButton->SetPressedColor(0xFF333333);
        
        UI::LayoutProperties colorButtonLayout;
        colorButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        colorButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        colorButtonLayout.layoutType = UI::LayoutType::None;
        colorButtonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        colorButton->SetLayoutProperties(colorButtonLayout);
        
        colorButton->SetOnClick([example](const UI::ButtonClickEvent& event) {
            Core::Log("[Font Test] Botão de cor " + example.first + " clicado!");
        });
        
        colorDemoPanel->AddChild(colorButton);
    }
    
    // ========================================
    // DEMONSTRAÇÃO 3: QUALIDADE DE FONTE
    // ========================================
    auto qualityDemoPanel = std::make_shared<UI::Panel>(uiContext);
    qualityDemoPanel->SetName("QualityDemoPanel");
    qualityDemoPanel->SetColor(0xFF333333);
    qualityDemoPanel->SetBorderWidth(1.0f);
    qualityDemoPanel->SetBorderColor(0xFF777777);
    
    UI::LayoutProperties qualityDemoLayout;
    qualityDemoLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    qualityDemoLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    qualityDemoLayout.layoutType = UI::LayoutType::Stack;
    qualityDemoLayout.stackDirection = UI::StackDirection::Vertical;
    qualityDemoLayout.stackSpacing = 5.0f;
    qualityDemoLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    qualityDemoLayout.padding = UI::LayoutMargins(8.0f).ToVec4();
    qualityDemoPanel->SetLayoutProperties(qualityDemoLayout);
    
    demoContainer->AddChild(qualityDemoPanel);
    
    // Botões demonstrando diferentes qualidades
    std::vector<std::string> qualityExamples = {
        "Qualidade Baixa (8x MSDF)",
        "Qualidade Média (16x MSDF)",
        "Qualidade Alta (32x MSDF)",
        "Qualidade Ultra (64x MSDF)",
        "Anti-aliasing Subpixel"
    };
    
    for (const auto& example : qualityExamples) {
        auto qualityButton = std::make_shared<UI::Button>(uiContext);
        qualityButton->SetName("QualityButton_" + example);
        qualityButton->SetText(example);
        qualityButton->SetSize({280.0f, 30.0f});
        qualityButton->SetNormalColor(0xFF9C27B0);
        qualityButton->SetHoverColor(0xFFBA68C8);
        qualityButton->SetPressedColor(0xFF7B1FA2);
        
        UI::LayoutProperties qualityButtonLayout;
        qualityButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        qualityButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        qualityButtonLayout.layoutType = UI::LayoutType::None;
        qualityButtonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        qualityButton->SetLayoutProperties(qualityButtonLayout);
        
        qualityButton->SetOnClick([example](const UI::ButtonClickEvent& event) {
            Core::Log("[Font Test] Botão de qualidade " + example + " clicado!");
        });
        
        qualityDemoPanel->AddChild(qualityButton);
    }
    
    // ========================================
    // DEMONSTRAÇÃO 4: TEXTO LONGO
    // ========================================
    auto longTextDemoPanel = std::make_shared<UI::Panel>(uiContext);
    longTextDemoPanel->SetName("LongTextDemoPanel");
    longTextDemoPanel->SetColor(0xFF333333);
    longTextDemoPanel->SetBorderWidth(1.0f);
    longTextDemoPanel->SetBorderColor(0xFF777777);
    
    UI::LayoutProperties longTextDemoLayout;
    longTextDemoLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    longTextDemoLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    longTextDemoLayout.layoutType = UI::LayoutType::Stack;
    longTextDemoLayout.stackDirection = UI::StackDirection::Vertical;
    longTextDemoLayout.stackSpacing = 5.0f;
    longTextDemoLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    longTextDemoLayout.padding = UI::LayoutMargins(8.0f).ToVec4();
    longTextDemoPanel->SetLayoutProperties(longTextDemoLayout);
    
    demoContainer->AddChild(longTextDemoPanel);
    
    // Botões com texto longo
    std::vector<std::string> longTextExamples = {
        "Texto muito longo que deve ser cortado adequadamente",
        "PalavraSuperLongaSemEspaçosQueDeveSerTratadaCorretamente",
        "Texto com caracteres especiais: áéíóú çãõ ñ",
        "Texto com números: 1234567890",
        "Texto com símbolos: !@#$%^&*()_+-=[]{}|;':\",./<>?"
    };
    
    for (const auto& example : longTextExamples) {
        auto longTextButton = std::make_shared<UI::Button>(uiContext);
        longTextButton->SetName("LongTextButton_" + std::to_string(longTextExamples.size()));
        longTextButton->SetText(example);
        longTextButton->SetSize({280.0f, 30.0f});
        longTextButton->SetNormalColor(0xFFFF9800);
        longTextButton->SetHoverColor(0xFFFFB74D);
        longTextButton->SetPressedColor(0xFFF57C00);
        
        UI::LayoutProperties longTextButtonLayout;
        longTextButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        longTextButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        longTextButtonLayout.layoutType = UI::LayoutType::None;
        longTextButtonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        longTextButton->SetLayoutProperties(longTextButtonLayout);
        
        longTextButton->SetOnClick([example](const UI::ButtonClickEvent& event) {
            Core::Log("[Font Test] Botão com texto longo clicado: " + example);
        });
        
        longTextDemoPanel->AddChild(longTextButton);
    }
    
    // ========================================
    // DEMONSTRAÇÃO 5: PERFORMANCE
    // ========================================
    auto performanceDemoPanel = std::make_shared<UI::Panel>(uiContext);
    performanceDemoPanel->SetName("PerformanceDemoPanel");
    performanceDemoPanel->SetColor(0xFF333333);
    performanceDemoPanel->SetBorderWidth(1.0f);
    performanceDemoPanel->SetBorderColor(0xFF777777);
    
    UI::LayoutProperties performanceDemoLayout;
    performanceDemoLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    performanceDemoLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    performanceDemoLayout.layoutType = UI::LayoutType::Stack;
    performanceDemoLayout.stackDirection = UI::StackDirection::Vertical;
    performanceDemoLayout.stackSpacing = 5.0f;
    performanceDemoLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    performanceDemoLayout.padding = UI::LayoutMargins(8.0f).ToVec4();
    performanceDemoPanel->SetLayoutProperties(performanceDemoLayout);
    
    demoContainer->AddChild(performanceDemoPanel);
    
    // Botões de teste de performance
    std::vector<std::string> performanceExamples = {
        "Teste de Cache de Glyphs",
        "Teste de Batching",
        "Teste de MSDF",
        "Teste de Anti-aliasing",
        "Teste de Subpixel Rendering"
    };
    
    for (const auto& example : performanceExamples) {
        auto performanceButton = std::make_shared<UI::Button>(uiContext);
        performanceButton->SetName("PerformanceButton_" + example);
        performanceButton->SetText(example);
        performanceButton->SetSize({280.0f, 30.0f});
        performanceButton->SetNormalColor(0xFF4CAF50);
        performanceButton->SetHoverColor(0xFF66BB6A);
        performanceButton->SetPressedColor(0xFF388E3C);
        
        UI::LayoutProperties performanceButtonLayout;
        performanceButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        performanceButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        performanceButtonLayout.layoutType = UI::LayoutType::None;
        performanceButtonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        performanceButton->SetLayoutProperties(performanceButtonLayout);
        
        performanceButton->SetOnClick([example](const UI::ButtonClickEvent& event) {
            Core::Log("[Font Test] Teste de performance: " + example);
        });
        
        performanceDemoPanel->AddChild(performanceButton);
    }
    
    // ========================================
    // DEMONSTRAÇÃO 6: CONTROLES
    // ========================================
    auto controlsDemoPanel = std::make_shared<UI::Panel>(uiContext);
    controlsDemoPanel->SetName("ControlsDemoPanel");
    controlsDemoPanel->SetColor(0xFF333333);
    controlsDemoPanel->SetBorderWidth(1.0f);
    controlsDemoPanel->SetBorderColor(0xFF777777);
    
    UI::LayoutProperties controlsDemoLayout;
    controlsDemoLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    controlsDemoLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    controlsDemoLayout.layoutType = UI::LayoutType::Stack;
    controlsDemoLayout.stackDirection = UI::StackDirection::Vertical;
    controlsDemoLayout.stackSpacing = 5.0f;
    controlsDemoLayout.margin = UI::LayoutMargins(5.0f).ToVec4();
    controlsDemoLayout.padding = UI::LayoutMargins(8.0f).ToVec4();
    controlsDemoPanel->SetLayoutProperties(controlsDemoLayout);
    
    demoContainer->AddChild(controlsDemoPanel);
    
    // Botões de controle
    std::vector<std::string> controlExamples = {
        "Limpar Cache",
        "Recarregar Fontes",
        "Alternar Qualidade",
        "Alternar Anti-aliasing",
        "Exportar Atlas"
    };
    
    for (const auto& example : controlExamples) {
        auto controlButton = std::make_shared<UI::Button>(uiContext);
        controlButton->SetName("ControlButton_" + example);
        controlButton->SetText(example);
        controlButton->SetSize({280.0f, 30.0f});
        controlButton->SetNormalColor(0xFFF44336);
        controlButton->SetHoverColor(0xFFEF5350);
        controlButton->SetPressedColor(0xFFD32F2F);
        
        UI::LayoutProperties controlButtonLayout;
        controlButtonLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
        controlButtonLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Center;
        controlButtonLayout.layoutType = UI::LayoutType::None;
        controlButtonLayout.margin = UI::LayoutMargins(2.0f).ToVec4();
        controlButton->SetLayoutProperties(controlButtonLayout);
        
        controlButton->SetOnClick([example](const UI::ButtonClickEvent& event) {
            Core::Log("[Font Test] Controle: " + example);
        });
        
        controlsDemoPanel->AddChild(controlButton);
    }
}

int main() {
    Core::Log("[Font Test] ==========================================");
    Core::Log("[Font Test] INICIANDO TESTE DO SISTEMA DE FONTES");
    Core::Log("[Font Test] ==========================================");

    // ================================
    // 1. INICIALIZAÇÃO DO GLFW
    // ================================
    Core::Log("[Font Test] 1. Inicializando GLFW...");
    if (!glfwInit()) {
        Core::Log("[Font Test] ERRO: Falha ao inicializar GLFW!");
        return -1;
    }
    Core::Log("[Font Test] 1. GLFW inicializado com sucesso!");

    // ================================
    // 2. CRIAÇÃO DA JANELA
    // ================================
    Core::Log("[Font Test] 2. Criando janela...");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1200, 800, "DriftEngine Font System Test", nullptr, nullptr);
    if (!window) {
        Core::Log("[Font Test] ERRO: Falha ao criar janela!");
        glfwTerminate();
        return -1;
    }
    Core::Log("[Font Test] 2. Janela criada com sucesso!");

    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        Core::Log("[Font Test] ERRO: Falha ao obter HWND!");
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
    // 7. SISTEMA DE FONTES
    // ================================
    Core::Log("[Font Test] 7. Inicializando sistema de fontes...");
    
    // Inicializa o gerenciador de fontes
    auto& fontManager = UI::FontManager::GetInstance();
    
    // Carrega fontes padrão (se disponíveis)
    // fontManager.LoadFont("default", "fonts/Roboto-Regular.ttf", 16.0f, UI::FontQuality::High);
    // fontManager.LoadFont("bold", "fonts/Roboto-Bold.ttf", 16.0f, UI::FontQuality::High);
    // fontManager.LoadFont("mono", "fonts/RobotoMono-Regular.ttf", 16.0f, UI::FontQuality::High);
    
    Core::Log("[Font Test] 7. Sistema de fontes inicializado!");

    // ================================
    // 8. CRIAÇÃO DOS TESTES DE FONTE
    // ================================
    Core::Log("[Font Test] Chamando TestFontSystem...");
    TestFontSystem(uiContext.get());
    Core::Log("[Font Test] TestFontSystem concluído!");
    
    Core::Log("[Font Test] Iniciando loop principal...");

    // ================================
    // 9. LOOP PRINCIPAL
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
            Core::Log("[Font Test] Janela redimensionada: " + std::to_string(width) + "x" + std::to_string(height));
            
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
    // 10. FINALIZAÇÃO
    // ================================
    Core::Log("[Font Test] Finalizando...");
    uiContext->Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    Core::Log("[Font Test] Teste concluído com sucesso!");

    return 0;
} 