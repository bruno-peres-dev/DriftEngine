// src/app/main.cpp

#include "Drift/Core/Log.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include "Drift/Renderer/TerrainPass.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <Windows.h>
#include <filesystem>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

struct AppData {
    std::shared_ptr<Drift::RHI::ISwapChain>   swapChain;
    std::shared_ptr<Drift::RHI::IContext>     context;
    std::unique_ptr<Drift::Renderer::IRenderPass> pass;
};

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (width <= 0 || height <= 0) return;

    auto app = static_cast<AppData*>(glfwGetWindowUserPointer(window));
    // 1) redimensiona swap chain
    app->swapChain->Resize(width, height);
    // 2) recria RTV/DSV + viewport
    app->context->Resize(width, height);
    // 3) atualiza aspect do pass
    app->pass->SetAspect(float(width) / float(height));
}

int main() {
    try {
        // GLFW sem API gráfica (usaremos DX11)
        if (!glfwInit())
            throw std::runtime_error("Falha ao inicializar GLFW");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(1280, 720, "DriftEngine", nullptr, nullptr);
        if (!window) throw std::runtime_error("Falha ao criar janela GLFW");

        HWND hwnd = glfwGetWin32Window(window);
        if (!hwnd) throw std::runtime_error("Falha ao obter HWND");

        // Verifica existência dos shaders
        auto checkShader = [&](const std::string& path) {
            if (!std::filesystem::exists(path)) {
                MessageBoxA(nullptr, ("Shader não encontrado: " + path).c_str(),
                    "Erro", MB_ICONERROR);
                return false;
            }
            return true;
            };
        if (!checkShader("shaders/TerrainVS.hlsl") ||
            !checkShader("shaders/TerrainPS.hlsl"))
        {
            glfwDestroyWindow(window);
            glfwTerminate();
            return -1;
        }

        // Cria Device DX11
        Drift::RHI::DeviceDesc desc{ 1280, 720, /* vsync */ false };
        auto device = Drift::RHI::DX11::CreateDeviceDX11(desc);

        // Cria SwapChain e Context
        auto swapChain = device->CreateSwapChain(hwnd);
        auto context = device->CreateContext();

        // Cria TerrainPass (passa RHI puro: device + context)
        auto terrainPass = std::make_unique<Drift::Renderer::TerrainPass>(
            *device, *context, L"textures/grass.png", 100, 100, 50.0f, false
        );

        // Armazena em AppData para callbacks
        AppData appData;
        appData.swapChain = std::move(swapChain);
        appData.context = std::move(context);
        appData.pass = std::move(terrainPass);

        // Configura callback de resize
        glfwSetWindowUserPointer(window, &appData);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

        // Inicializa tamanho e aspect
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        FramebufferSizeCallback(window, fbw, fbh);

        // Loop principal
        double lastTime = glfwGetTime();
        double fpsTime = lastTime;
        int frameCount = 0;
        float fps = 0.0f;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            double now = glfwGetTime();
            float dt = float(now - lastTime);
            lastTime = now;
            frameCount++;
            if (now - fpsTime >= 1.0) {
                fps = frameCount / float(now - fpsTime);
                fpsTime = now;
                frameCount = 0;
                char title[256];
                snprintf(title, sizeof(title), "DriftEngine [FPS: %.1f]", fps);
                glfwSetWindowTitle(window, title);
            }
            // Chama Update do TerrainPass para movimentação de câmera
            if (auto* terrainPass = dynamic_cast<Drift::Renderer::TerrainPass*>(appData.pass.get())) {
                terrainPass->Update(dt, window);
            }
            appData.pass->Execute();
            appData.context->Present();
        }

        // Cleanup
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr,
            ("Erro fatal: " + std::string(e.what())).c_str(),
            "DriftEngine Error",
            MB_ICONERROR);
        return -1;
    }
}
