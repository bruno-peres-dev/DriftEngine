// src/app/main_new_architecture.cpp
// Demo da nova arquitetura AAA do DriftEngine

#include "Drift/Core/Log.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/Renderer/RenderManager.h"
#include "Drift/Renderer/TerrainPass.h"
#include "Drift/Engine/Input/Input.h"
#include "Drift/Engine/Camera/Camera.h"
#include "Drift/Engine/Viewport/Viewport.h"

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
    
    Core::Log("[App] Redimensionado para " + std::to_string(width) + "x" + std::to_string(height));
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
        
        // Cria viewport do jogo
        Engine::Viewport::ViewportDesc gameViewDesc;
        gameViewDesc.name = "GameView";
        gameViewDesc.x = 0;
        gameViewDesc.y = 0;
        gameViewDesc.width = 1280;
        gameViewDesc.height = 720;
        gameViewDesc.acceptsInput = true;
        gameViewDesc.clearColor[0] = 0.1f; // Azul escuro
        gameViewDesc.clearColor[1] = 0.1f;
        gameViewDesc.clearColor[2] = 0.2f;
        gameViewDesc.clearColor[3] = 1.0f;
        
        auto gameViewport = std::make_unique<Engine::Viewport::BasicViewport>(
            gameViewDesc, 
            std::move(gameCameraController), 
            gamePasses
        );
        
        renderManager->AddViewport("GameView", std::move(gameViewport));
        
        // --------------------------------
        // 4.2. VIEWPORT SECUNDÁRIA (Editor - futuro)
        // --------------------------------
        /*
        // Exemplo de como adicionar uma segunda viewport para editor:
        
        auto editorCamera = std::make_unique<Engine::Camera::PerspectiveCamera>();
        auto editorController = std::make_unique<Engine::Camera::OrbitCameraController>(
            std::move(editorCamera)
        );
        
        Engine::Viewport::ViewportDesc editorViewDesc;
        editorViewDesc.name = "EditorView";
        editorViewDesc.x = 640;
        editorViewDesc.y = 0;
        editorViewDesc.width = 640;
        editorViewDesc.height = 720;
        editorViewDesc.acceptsInput = false; // Editor viewport não aceita input por enquanto
        
        auto editorViewport = std::make_unique<Engine::Viewport::BasicViewport>(
            editorViewDesc,
            std::move(editorController),
            gamePasses // Reutiliza os mesmos passes por enquanto
        );
        
        renderManager->AddViewport("EditorView", std::move(editorViewport));
        
        // Aplica layout split horizontal
        renderManager->SetLayout(Renderer::ViewportLayout::SplitHorizontal, 1280, 720);
        */
        
        // ================================
        // 5. CONFIGURAÇÃO DA APLICAÇÃO
        // ================================
        
        AppData appData;
        appData.swapChain = std::move(swapChain);
        appData.context = std::move(context);
        appData.inputManager = std::move(inputManager);
        appData.renderManager = std::move(renderManager);

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
                snprintf(title, sizeof(title), 
                    "DriftEngine AAA [FPS: %.1f] [Frame: %.2fms] [Viewports: %zu]", 
                    fps, 
                    appData.renderManager->GetStats().frameTime,
                    appData.renderManager->GetViewportCount()
                );
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
            
            // ---- RENDER ----
            appData.renderManager->Render(*appData.context);
            
            // ---- PRESENT ----
            appData.context->Present();
            
            // ---- DEBUG OUTPUT (esporádico) ----
            static float debugTimer = 0.0f;
            debugTimer += deltaTime;
            if (debugTimer >= 5.0f) { // A cada 5 segundos
                const auto& stats = appData.renderManager->GetStats();
                Core::Log("[App] Stats: " + 
                    std::to_string(stats.viewportsRendered) + " viewports, " +
                    std::to_string(stats.frameTime) + "ms frame time");
                debugTimer = 0.0f;
            }
        }

        // ================================
        // 7. CLEANUP
        // ================================
        
        Core::Log("[App] Finalizando aplicação...");
        
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