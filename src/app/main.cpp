// src/app/main_new_architecture.cpp
// Demo da nova arquitetura AAA do DriftEngine

#include "Drift/Core/Log.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/RHI/ResourceManager.h"
#include "Drift/Renderer/RenderManager.h"
#include "Drift/Renderer/TerrainPass.h"
#include "Drift/Engine/Input/Input.h"
#include "Drift/Engine/Camera/Camera.h"
#include "Drift/Engine/Viewport/Viewport.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/UIElement.h"
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
}

int main() {
    try {
        Core::Log("[App] Inicializando DriftEngine com nova arquitetura AAA...");
        
        // ================================
        // 1. INICIALIZAÇÃO BÁSICA
        // ================================
        
        // GLFW sem API gráfica (usaremos DX11)
        if (!glfwInit())
            throw std::runtime_error("Falha ao inicializar GLFW");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(1280, 720, "DriftEngine", nullptr, nullptr);
        if (!window) throw std::runtime_error("Falha ao criar janela GLFW");

        HWND hwnd = glfwGetWin32Window(window);
        if (!hwnd) throw std::runtime_error("Falha ao obter HWND");

        // ================================
        // 2. CRIAÇÃO DO RHI
        // ================================
        
        // Cria Device DX11
        RHI::DeviceDesc desc{ 1280, 720, /* vsync */ false };
        auto device = RHI::DX11::CreateDeviceDX11(desc);

        // Cria SwapChain e Context
        auto swapChain = device->CreateSwapChain(hwnd);
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

        // Cria painel simples como demo
        {
            auto root = uiContext->GetRoot();
            auto panel = std::make_shared<UI::UIElement>(uiContext.get());
            panel->SetPosition({50.0f, 50.0f});
            panel->SetSize({200.0f, 100.0f});
            panel->SetColor(0xFF0000FF); // Vermelho sólido para teste
            root->AddChild(panel);
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
        editorViewDesc.clearColor[1] = 0.2f;
        editorViewDesc.clearColor[2] = 0.3f;
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

        // ================================
        // 4.c UI BATCHER E RING BUFFER
        // ================================

        {
            ID3D11Device*       nativeDev  = static_cast<ID3D11Device*>(device->GetNativeDevice());
            ID3D11DeviceContext* nativeCtx = static_cast<ID3D11DeviceContext*>(appData.context->GetNativeContext());

            appData.uiRingBuffer = Drift::RHI::DX11::CreateRingBufferDX11(nativeDev, nativeCtx, 2 * 1024 * 1024);
            appData.uiBatcher    = Drift::RHI::DX11::CreateUIBatcherDX11(appData.uiRingBuffer, appData.context.get());
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

        Core::Log("[App] Entrando no loop principal...");
        
        double lastTime = glfwGetTime();
        double fpsTime = lastTime;
        int frameCount = 0;
        float fps = 0.0f;
        
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            // ---- TIMING ----
            double now = glfwGetTime();
            float deltaTime = float(now - lastTime);
            lastTime = now;
            
            frameCount++;
            if (now - fpsTime >= 1.0) {
                fps = frameCount / float(now - fpsTime);
                fpsTime = now;
                frameCount = 0;
                
                char title[256];
                const char* activeVp = appData.renderManager->GetActiveViewport().c_str();
                snprintf(title, sizeof(title), 
                    "DriftEngine AAA [FPS: %.1f] [Frame: %.2fms] [Viewports: %zu] [Active: %s]", 
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
            
            // ---- RENDER MANAGER UPDATE ----
            appData.renderManager->Update(deltaTime, input);

            // ---- UI UPDATE ----
            appData.uiContext->Update(deltaTime);

            // ---- CURSOR LOCK ----
            bool lockCursor = false;
            const std::string& activeName = appData.renderManager->GetActiveViewport();
            if (!activeName.empty()) {
                auto* activeVP = appData.renderManager->GetViewport(activeName);
                if (activeVP && activeVP->GetDesc().acceptsInput) {
                    lockCursor = true;
                }
            }
            appData.inputManager->SetMouseLocked(lockCursor);
            appData.inputManager->SetMouseVisible(!lockCursor);

            // ---- RENDER ----
            appData.renderManager->Render(*appData.context);

            // ---- UI RENDER (overlay) ----
            {
                appData.uiBatcher->Begin();
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

        // Cleanup automático via RAII
        glfwDestroyWindow(window);
        glfwTerminate();
        
        Core::Log("[App] Aplicação finalizada com sucesso!");
        return 0;
        
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr,
            ("Erro fatal: " + std::string(e.what())).c_str(),
            "DriftEngine AAA Error",
            MB_ICONERROR);
        return -1;
    }
} 