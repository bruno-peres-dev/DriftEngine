// src/app/main_new_architecture.cpp
// Demo da nova arquitetura AAA do DriftEngine

#include "Drift/Core/Log.h"
#include "Drift/Core/Threading/ThreadingSystem.h"
#include "Drift/Core/Threading/ThreadingExample.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/ResourceManager.h"
#include "Drift/RHI/RHIException.h"
#include "Drift/Renderer/RenderManager.h"
#include "Drift/Renderer/TerrainPass.h"
#include "Drift/Engine/Input/Input.h"
#include "Drift/Engine/Camera/Camera.h"
#include "Drift/Engine/Viewport/Viewport.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/Widgets/Label.h"
#include "Drift/RHI/DX11/RingBufferDX11.h"
#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include <d3d11.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <Windows.h>
#include <filesystem>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

using namespace Drift;

struct AppData {
    std::shared_ptr<RHI::ISwapChain> swapChain;
    std::shared_ptr<RHI::IContext> context;
    std::unique_ptr<Engine::Input::IInputManager> inputManager;
    std::unique_ptr<Renderer::RenderManager> renderManager;
    std::shared_ptr<RHI::IRingBuffer> uiRingBuffer;
    std::unique_ptr<RHI::IUIBatcher> uiBatcher;
    std::unique_ptr<UI::UIContext> uiContext;
    
    // Elementos de teste da UI
    std::shared_ptr<UI::Label> testLabel;
    std::shared_ptr<UI::Label> infoLabel;
    std::shared_ptr<UI::Label> debugLabel;
    std::shared_ptr<UI::Label> performanceLabel;
    std::shared_ptr<UI::Label> instructionLabel;
    
    // Contadores para animação
    float animationTime = 0.0f;
    int frameCounter = 0;
};

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (width <= 0 || height <= 0) return;

    auto app = static_cast<AppData*>(glfwGetWindowUserPointer(window));
    
    // Redimensiona swap chain
    app->swapChain->Resize(width, height);
    
    // Redimensiona contexto RHI
    app->context->Resize(width, height);
    
    // Redimensiona todas as viewports
    app->renderManager->ResizeAllViewports(width, height);
    
    // Atualiza tamanho da tela no sistema UI
    if (app->uiContext) {
        app->uiContext->SetScreenSize(static_cast<float>(width), static_cast<float>(height));
    }
    
    // Atualiza tamanho da tela no UIBatcher
    if (app->uiBatcher) {
        app->uiBatcher->SetScreenSize(static_cast<float>(width), static_cast<float>(height));
    }
}

int main() {
    // Configurar tratamento global de exceções
    std::set_terminate([]() {
        Core::LogError("Exceção não tratada detectada!");
        std::abort();
    });
    
    try {
        Core::Log("[App] Iniciando DriftEngine...");
        
        // Configurar nível de log para Debug (para ver logs de debug do binding)
        Core::SetLogLevel(Core::LogLevel::Debug);
        
        // ================================
        // 0. SISTEMA DE THREADING
        // ================================
        
        Core::Log("[App] Inicializando sistema de threading...");
        auto& threadingSystem = Drift::Core::Threading::ThreadingSystem::GetInstance();
        
        // Configuração otimizada para jogos
        Drift::Core::Threading::ThreadingConfig threadingConfig;
        threadingConfig.threadCount = std::thread::hardware_concurrency() - 1; // Deixa um core livre
        threadingConfig.enableWorkStealing = true;
        threadingConfig.enableAffinity = true;
        threadingConfig.enableProfiling = true; // Habilita profiling para debug
        threadingConfig.threadNamePrefix = "DriftEngine";
        
        threadingSystem.Initialize(threadingConfig);
        Core::Log("[App] Sistema de threading inicializado com " + 
                  std::to_string(threadingSystem.GetThreadCount()) + " threads");
        
        // Executa exemplo básico do sistema de threading
        Core::Log("[App] Executando exemplo do sistema de threading...");
        Drift::Core::Threading::ThreadingExample::RunBasicExample();
        
        // ================================
        // 1. INICIALIZAÇÃO BÁSICA
        // ================================
        
        // GLFW sem API gráfica (usaremos DX11)
        if (!glfwInit())
            throw std::runtime_error("Falha ao inicializar GLFW");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(1280, 720, "DriftEngine - Teste de Fontes", nullptr, nullptr);
        if (!window) throw std::runtime_error("Falha ao criar janela GLFW");

        HWND hwnd = glfwGetWin32Window(window);
        if (!hwnd) throw std::runtime_error("Falha ao obter HWND");

        // ================================
        // 2. CRIAÇÃO DO RHI
        // ================================
        
        Core::LogRHI("Iniciando criação do sistema RHI...");
        
        // Cria Device DX11
        RHI::DeviceDesc desc{ 1280, 720, /* vsync */ false };
        auto device = RHI::DX11::CreateDeviceDX11(desc);

        // Cria SwapChain e Context
        Core::LogRHI("Criando SwapChain...");
        auto swapChain = device->CreateSwapChain(hwnd);
        
        Core::LogRHI("Criando Context...");
        auto context = device->CreateContext();
        
        // ================================
        // 3. SISTEMA DE INPUT ABSTRATO
        // ================================
        
        auto inputManager = Engine::Input::CreateGLFWInputManager(window);
        
        // ================================
        // 4. RENDER MANAGER E VIEWPORTS
        // ================================
        
        auto renderManager = std::make_unique<Renderer::RenderManager>();

        // ================================
        // 4.b UI CONTEXT
        // ================================

        auto uiContext = std::make_unique<UI::UIContext>();
        uiContext->Initialize();
        
        Core::Log("[App] Device criado: " + std::to_string(device != nullptr));
        
        // Configurar device para o sistema de fontes
        Core::Log("[App] Chamando SetDevice...");
        uiContext->SetDevice(device.get());
        Core::Log("[App] SetDevice concluído");
        
        // Carregar tamanhos prioritários da fonte 'default' para evitar glitches de glifos
        Core::Log("[App] Carregando fontes de teste...");
        {
            auto& fontManager = UI::FontManager::GetInstance();
            std::vector<float> tamanhos = {12.0f, 16.0f, 20.0f, 24.0f, 32.0f, 48.0f};
            for (float size : tamanhos) {
                auto font = fontManager.LoadFont("default", "fonts/Arial-Regular.ttf", size, UI::FontQuality::High);
                if (!font) {
                    Core::Log("[App] ERRO: Fonte 'default' tamanho " + std::to_string(size) + " não carregada!");
                } else {
                    Core::Log("[App] Fonte 'default' tamanho " + std::to_string(size) + " carregada com sucesso!");
                }
            }
        }
        
        // Conecta o sistema de input
        uiContext->SetInputManager(inputManager.get());
        
        // Configurar tamanho inicial da tela no sistema UI
        uiContext->SetScreenSize(1280.0f, 720.0f);

        // Cria elementos de teste da UI
        Core::Log("[App] Criando elementos de teste da UI...");
        {
            auto& fontManager = UI::FontManager::GetInstance();
            auto root = uiContext->GetRoot();
            
            // Label principal de teste
            auto testLabel = std::make_shared<UI::Label>(uiContext.get());
            testLabel->SetName("TestLabel");
            testLabel->SetText("DriftEngine - Sistema de Fontes");
            testLabel->SetPosition({50.0f, 30.0f});
            testLabel->SetFontFamily("default");
            testLabel->SetFontSize(32.0f);
            testLabel->SetTextColor(Drift::Color(0xFFFFFFFF)); // Branco
            testLabel->SetColor(0x00000000); // Fundo transparente
            testLabel->MarkDirty();
            root->AddChild(testLabel);
            
            // Label de informações
            auto infoLabel = std::make_shared<UI::Label>(uiContext.get());
            infoLabel->SetName("InfoLabel");
            infoLabel->SetText("Testando diferentes tamanhos e cores de fonte");
            infoLabel->SetPosition({50.0f, 80.0f});
            infoLabel->SetFontFamily("default");
            infoLabel->SetFontSize(20.0f);
            infoLabel->SetTextColor(Drift::Color(0xFF00FFFF)); // Ciano
            infoLabel->SetColor(0x00000000);
            infoLabel->MarkDirty();
            root->AddChild(infoLabel);
            
            // Label de debug
            auto debugLabel = std::make_shared<UI::Label>(uiContext.get());
            debugLabel->SetName("DebugLabel");
            debugLabel->SetText("Debug: Sistema funcionando");
            debugLabel->SetPosition({50.0f, 120.0f});
            debugLabel->SetFontFamily("default");
            debugLabel->SetFontSize(16.0f);
            debugLabel->SetTextColor(Drift::Color(0xFF00FF00)); // Verde
            debugLabel->SetColor(0x00000000);
            debugLabel->MarkDirty();
            root->AddChild(debugLabel);
            
            // Label de performance
            auto performanceLabel = std::make_shared<UI::Label>(uiContext.get());
            performanceLabel->SetName("PerformanceLabel");
            performanceLabel->SetText("Performance: Aguardando dados...");
            performanceLabel->SetPosition({50.0f, 150.0f});
            performanceLabel->SetFontFamily("default");
            performanceLabel->SetFontSize(14.0f);
            performanceLabel->SetTextColor(Drift::Color(0xFFFFFF00)); // Amarelo
            performanceLabel->SetColor(0x00000000);
            performanceLabel->MarkDirty();
            root->AddChild(performanceLabel);
            
            // Label de instruções
            auto instructionLabel = std::make_shared<UI::Label>(uiContext.get());
            instructionLabel->SetName("InstructionLabel");
            instructionLabel->SetText("Controles: F1 = Wireframe, ESC = Sair, R = Recarregar fontes, T = Teste Threading");
            instructionLabel->SetPosition({50.0f, 680.0f});
            instructionLabel->SetFontFamily("default");
            instructionLabel->SetFontSize(16.0f);
            instructionLabel->SetTextColor(Drift::Color(0xFFFF8000)); // Laranja
            instructionLabel->SetColor(0x00000000);
            instructionLabel->MarkDirty();
            root->AddChild(instructionLabel);
        }
        
        // --------------------------------
        // 4.1. VIEWPORT PRINCIPAL (Game)
        // --------------------------------
        
        // Cria câmera perspectiva
        auto gameCamera = std::make_unique<Engine::Camera::PerspectiveCamera>(
            glm::vec3{500.0f, 50.0f, 800.0f},   // position
            glm::vec3{500.0f, 0.0f, 500.0f},    // target
            glm::vec3{0.0f, 1.0f, 0.0f},        // up
            glm::radians(45.0f),                 // fovY
            16.0f / 9.0f,                        // aspect
            0.1f,                                // nearPlane
            10000.0f                             // farPlane
        );
        
        // Cria controlador de câmera livre
        auto gameCameraController = std::make_unique<Engine::Camera::FreeCameraController>(
            std::move(gameCamera)
        );
        gameCameraController->SetMovementSpeed(100.0f);
        gameCameraController->SetMouseSensitivity(0.1f);
        
        // Cria render passes
        std::vector<std::shared_ptr<Renderer::IRenderPass>> gamePasses;
        auto terrainPass = std::make_shared<Renderer::TerrainPass>(
            *device, 
            L"textures/grass.png", 
            100, 100, 50.0f, false
        );
        gamePasses.push_back(terrainPass);

        // --------------------------------
        // 4.2. VIEWPORT SECUNDÁRIA (Editor)
        // --------------------------------

        // Cria câmera para a viewport de editor (sem input por enquanto)
        auto editorCamera = std::make_unique<Engine::Camera::PerspectiveCamera>(
            glm::vec3{500.0f, 150.0f, 300.0f}, // posição
            glm::vec3{500.0f, 0.0f, 500.0f},   // alvo
            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::radians(45.0f),
            16.0f / 9.0f,
            0.1f,
            10000.0f
        );

        auto editorController = std::make_unique<Engine::Camera::FreeCameraController>(
            std::move(editorCamera)
        );
        editorController->SetMovementSpeed(50.0f);
        editorController->SetMouseSensitivity(0.08f);

        Engine::Viewport::ViewportDesc editorViewDesc;
        editorViewDesc.name = "EditorView";
        editorViewDesc.x = 0; // será ajustado pelo layout
        editorViewDesc.y = 0;
        editorViewDesc.width = 1280;
        editorViewDesc.height = 720;
        editorViewDesc.acceptsInput = true; // apenas visualização
        editorViewDesc.clearColor[0] = 0.0f;
        editorViewDesc.clearColor[1] = 0.0f;
        editorViewDesc.clearColor[2] = 0.1f;
        editorViewDesc.clearColor[3] = 1.0f;

        auto editorViewport = std::make_unique<Engine::Viewport::BasicViewport>(
            editorViewDesc,
            std::move(editorController),
            gamePasses // reutiliza os mesmos passes (terrain)
        );

        renderManager->AddViewport("EditorView", std::move(editorViewport));

        // Configura layout horizontal lado a lado
        renderManager->SetLayout(Renderer::ViewportLayout::Single, 1280, 720);
        
        // ================================
        // 5. CONFIGURAÇÃO DA APLICAÇÃO
        // ================================
        
        AppData appData;
        appData.swapChain = std::move(swapChain);
        appData.context = std::move(context);
        appData.inputManager = std::move(inputManager);
        appData.renderManager = std::move(renderManager);
        appData.uiContext = std::move(uiContext);
        
        // Referências para os elementos de teste
        appData.testLabel = std::static_pointer_cast<UI::Label>(appData.uiContext->GetRoot()->FindChildByName("TestLabel"));
        appData.infoLabel = std::static_pointer_cast<UI::Label>(appData.uiContext->GetRoot()->FindChildByName("InfoLabel"));
        appData.debugLabel = std::static_pointer_cast<UI::Label>(appData.uiContext->GetRoot()->FindChildByName("DebugLabel"));
        appData.performanceLabel = std::static_pointer_cast<UI::Label>(appData.uiContext->GetRoot()->FindChildByName("PerformanceLabel"));
        appData.instructionLabel = std::static_pointer_cast<UI::Label>(appData.uiContext->GetRoot()->FindChildByName("InstructionLabel"));

        // ================================
        // 4.c UI BATCHER E RING BUFFER
        // ================================

        {
            ID3D11Device*       nativeDev  = static_cast<ID3D11Device*>(device->GetNativeDevice());
            ID3D11DeviceContext* nativeCtx = static_cast<ID3D11DeviceContext*>(appData.context->GetNativeContext());

            appData.uiRingBuffer = Drift::RHI::DX11::CreateRingBufferDX11(nativeDev, nativeCtx, 2 * 1024 * 1024);
            appData.uiBatcher    = Drift::RHI::DX11::CreateUIBatcherDX11(appData.uiRingBuffer, appData.context.get());
            
            // Configurar tamanho inicial da tela no UIBatcher
            appData.uiBatcher->SetScreenSize(1280.0f, 720.0f);
        }

        // Configura callback de resize
        glfwSetWindowUserPointer(window, &appData);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

        // Inicializa tamanho e aspect
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        FramebufferSizeCallback(window, fbw, fbh);
        
        // ================================
        // 6. LOOP PRINCIPAL AAA
        // ================================

        // Entrando no loop principal...
        // Controles: F1 = Wireframe, ESC = Sair, R = Recarregar fontes
        
        double lastTime = glfwGetTime();
        double fpsTime = lastTime;
        int frameCount = 0;
        float fps = 0.0f;
        
        Core::Log("[App] Iniciando loop principal...");
        
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            // ---- TIMING ----
            double now = glfwGetTime();
            float deltaTime = float(now - lastTime);
            lastTime = now;
            
            appData.animationTime += deltaTime;
            appData.frameCounter++;
            
            frameCount++;
            if (now - fpsTime >= 1.0) {
                fps = frameCount / float(now - fpsTime);
                fpsTime = now;
                frameCount = 0;
                
                char title[256];
                const char* activeVp = appData.renderManager->GetActiveViewport().c_str();
                snprintf(title, sizeof(title), 
                    "DriftEngine - Teste de Fontes [FPS: %.1f] [Frame: %.2fms] [Viewports: %zu] [Active: %s]", 
                    fps, 
                    appData.renderManager->GetStats().frameTime,
                    appData.renderManager->GetViewportCount(),
                    activeVp);
                glfwSetWindowTitle(window, title);
            }
            
            // ---- INPUT UPDATE ----
            appData.inputManager->Update();
            const auto& input = appData.inputManager->GetCurrentFrame();
            
            // Global controls
            if (input.IsKeyPressed(Engine::Input::Key::Escape)) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            
            // Toggle wireframe with F1
            if (input.IsKeyPressed(Engine::Input::Key::F1)) {
                appData.renderManager->SetWireframeMode(
                    !appData.renderManager->IsWireframeMode()
                );
                Core::Log("[App] Wireframe mode: " + 
                    std::string(appData.renderManager->IsWireframeMode() ? "ON" : "OFF"));
            }
            
            // Recarregar fontes com R
            if (input.IsKeyPressed(Engine::Input::Key::R)) {
                Core::Log("[App] Recarregando fontes...");
                auto& fontManager = UI::FontManager::GetInstance();
                fontManager.ClearCache();
                
                std::vector<float> tamanhos = {12.0f, 16.0f, 20.0f, 24.0f, 32.0f, 48.0f};
                for (float size : tamanhos) {
                    auto font = fontManager.LoadFont("default", "fonts/Arial-Regular.ttf", size, UI::FontQuality::High);
                    if (font) {
                        Core::Log("[App] Fonte recarregada: tamanho " + std::to_string(size));
                    }
                }
            }
            
            // Teste do sistema de threading com T
            if (input.IsKeyPressed(Engine::Input::Key::T)) {
                Core::Log("[App] Executando teste de threading...");
                
                // Exemplo de processamento paralelo
                std::vector<int> data(1000000);
                for (int i = 0; i < data.size(); ++i) {
                    data[i] = i;
                }
                
                std::vector<int> result(data.size());
                const size_t chunkSize = data.size() / 8;
                std::vector<Drift::Core::Threading::TaskFuture<void>> futures;
                
                auto startTime = std::chrono::steady_clock::now();
                
                // Processa chunks em paralelo
                for (size_t i = 0; i < 8; ++i) {
                    size_t start = i * chunkSize;
                    size_t end = (i == 7) ? data.size() : (i + 1) * chunkSize;
                    
                    // Cria info da tarefa
                    auto info = Drift::Core::Threading::TaskInfo{};
                    info.name = "ProcessChunk_" + std::to_string(i);
                    
                    // Submete a tarefa
                    auto future = threadingSystem.SubmitWithInfo(info, [&data, &result, start, end]() {
                        for (size_t j = start; j < end; ++j) {
                            result[j] = data[j] * data[j] + data[j];
                        }
                    });
                    
                    futures.push_back(std::move(future));
                }
                
                // Aguarda todos terminarem
                DRIFT_WAIT_FOR_ALL();
                
                auto endTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                
                Core::Log("[App] Processamento paralelo concluído em " + std::to_string(duration.count()) + "ms");
                
                // Verifica resultado
                int sum = 0;
                for (int value : result) {
                    sum += value;
                }
                Core::Log("[App] Soma total: " + std::to_string(sum));
                
                // Mostra estatísticas do sistema
                threadingSystem.LogStats();
            }
            
            // ---- RENDER MANAGER UPDATE ----
            appData.renderManager->Update(deltaTime, input);

            // ---- UI UPDATE ----
            appData.uiContext->Update(deltaTime);
            
            // Atualizar labels de teste com animação
            if (appData.debugLabel) {
                std::string debugText = "Debug: Frame " + std::to_string(appData.frameCounter) + 
                                      " | Tempo: " + std::to_string(appData.animationTime) + "s";
                appData.debugLabel->SetText(debugText);
            }
            
            if (appData.performanceLabel) {
                std::string perfText = "Performance: FPS " + std::to_string(static_cast<int>(fps)) + 
                                     " | Delta: " + std::to_string(deltaTime * 1000.0f) + "ms";
                appData.performanceLabel->SetText(perfText);
            }

            // ---- CURSOR LOCK ----
            // Por enquanto, desabilita o lock do cursor para permitir interação com UI
            // TODO: Implementar sistema de foco para alternar entre viewport e UI
            bool lockCursor = false;
            appData.inputManager->SetMouseLocked(lockCursor);
            appData.inputManager->SetMouseVisible(!lockCursor);

            // ---- RENDER ----
            appData.renderManager->Render(*appData.context);

            // ---- UI RENDER (overlay) ----
            {
                // Configurar viewport para UI (tela inteira)
                appData.context->SetViewport(0, 0, 1280, 720);

                appData.uiBatcher->Begin();
                
                // Teste: Adicionar retângulos coloridos de fundo
                appData.uiBatcher->AddRect(40.0f, 20.0f, 400.0f, 200.0f, Drift::Color(0x80000000)); // Fundo semi-transparente
                appData.uiBatcher->AddRect(40.0f, 650.0f, 600.0f, 60.0f, Drift::Color(0x80000000)); // Fundo para instruções
                
                // Teste: Adicionar texto diretamente via UIBatcher
                appData.uiBatcher->AddText(50.0f, 200.0f, "Texto direto via UIBatcher - Tamanho 16", Drift::Color(0xFFFF00FF));
                appData.uiBatcher->AddText(50.0f, 230.0f, "Texto direto via UIBatcher - Tamanho 20", Drift::Color(0xFF00FFFF));
                appData.uiBatcher->AddText(50.0f, 260.0f, "Texto direto via UIBatcher - Tamanho 24", Drift::Color(0xFFFFFF00));
                
                // Teste: Texto com caracteres especiais
                appData.uiBatcher->AddText(50.0f, 300.0f, "Caracteres especiais: áéíóú çãõ ñ", Drift::Color(0xFFFFFFFF));
                appData.uiBatcher->AddText(50.0f, 330.0f, "Números: 0123456789", Drift::Color(0xFFFF8000));
                appData.uiBatcher->AddText(50.0f, 360.0f, "Símbolos: !@#$%^&*()_+-=[]{}|;':\",./<>?", Drift::Color(0xFF00FF00));
                
                // Renderizar UI context (elementos criados via sistema de UI)
                appData.uiContext->Render(*appData.uiBatcher);
                
                appData.uiBatcher->End();
            }
            
            // ---- PRESENT ----
            appData.context->Present();
        }

        // ================================
        // 7. CLEANUP
        // ================================
        
        Core::Log("[App] Finalizando aplicação...");
        
        // Estatísticas do sistema de fontes
        if (appData.uiContext) {
            auto* textRenderer = appData.uiContext->GetTextRenderer();
            if (textRenderer) {
                // TODO: Implementar GetStats() no TextRenderer
                Core::Log("[FontSystem] Estatísticas de renderização de texto: (não implementado)");
            }
            auto& fontManager = Drift::UI::FontManager::GetInstance();
            // TODO: Implementar GetStats() no FontManager
            Core::Log("[FontSystem] Estatísticas do cache de fontes: (não implementado)");
        }
        
        // ================================
        // 7.a DEMONSTRAÇÃO DO RESOURCE MANAGER
        // ================================
        
        // Obtém estatísticas do Resource Manager
        auto resourceStats = RHI::g_resourceManager.GetGlobalStats();
        Core::Log("[ResourceManager] Estatísticas finais:");
        Core::Log("[ResourceManager] - Dispositivos: " + std::to_string(resourceStats.deviceCount));
        Core::Log("[ResourceManager] - Total de recursos: " + std::to_string(resourceStats.totalResources));
        Core::Log("[ResourceManager] - Uso de memória: " + std::to_string(resourceStats.totalMemoryUsage / (1024 * 1024)) + " MB");
        
        // Exemplo de configuração de limites
        auto deviceDX11 = static_cast<RHI::DX11::DeviceDX11*>(device.get());
        auto& shaderCache = RHI::g_resourceManager.GetCache<RHI::ShaderDesc, RHI::IShader>(device->GetNativeDevice());
        shaderCache.SetMaxSize(500);  // Máximo 500 shaders
        shaderCache.SetMaxMemoryUsage(256 * 1024 * 1024);  // 256MB para shaders
        
        auto& textureCache = RHI::g_resourceManager.GetCache<RHI::TextureDesc, RHI::ITexture>(device->GetNativeDevice());
        textureCache.SetMaxMemoryUsage(1024 * 1024 * 1024);  // 1GB para texturas
        
        Core::Log("[ResourceManager] Limites configurados:");
        Core::Log("[ResourceManager] - Shaders: max " + std::to_string(shaderCache.GetStats().maxSize) + " recursos");
        Core::Log("[ResourceManager] - Texturas: max " + std::to_string(textureCache.GetStats().maxMemoryUsage / (1024 * 1024)) + " MB");
        
        appData.uiContext->Shutdown();
        
        // Shutdown do sistema de threading
        Core::Log("[App] Finalizando sistema de threading...");
        threadingSystem.Shutdown();

        // Cleanup automático via RAII
        glfwDestroyWindow(window);
        glfwTerminate();
        
        Core::Log("[App] Aplicação finalizada com sucesso!");
        return 0;
        
    }
    catch (const Drift::RHI::RHIException& e) {
        Core::LogException("RHI Exception", e);
        MessageBoxA(nullptr,
            ("Erro RHI: " + std::string(e.what())).c_str(),
            "DriftEngine RHI Error",
            MB_ICONERROR);
        return -1;
    }
    catch (const Drift::RHI::DeviceException& e) {
        Core::LogException("Device Exception", e);
        MessageBoxA(nullptr,
            ("Erro de Device: " + std::string(e.what())).c_str(),
            "DriftEngine Device Error",
            MB_ICONERROR);
        return -1;
    }
    catch (const Drift::RHI::ContextException& e) {
        Core::LogException("Context Exception", e);
        MessageBoxA(nullptr,
            ("Erro de Context: " + std::string(e.what())).c_str(),
            "DriftEngine Context Error",
            MB_ICONERROR);
        return -1;
    }
    catch (const Drift::RHI::SwapChainException& e) {
        Core::LogException("SwapChain Exception", e);
        MessageBoxA(nullptr,
            ("Erro de SwapChain: " + std::string(e.what())).c_str(),
            "DriftEngine SwapChain Error",
            MB_ICONERROR);
        return -1;
    }
    catch (const std::exception& e) {
        Core::LogException("General Exception", e);
        MessageBoxA(nullptr,
            ("Erro fatal: " + std::string(e.what())).c_str(),
            "DriftEngine AAA Error",
            MB_ICONERROR);
        return -1;
    }
} 