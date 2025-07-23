# Exemplo de Integração do Sistema de UI no Main.cpp

## Inclusões Necessárias

```cpp
// Incluir o sistema de UI completo
#include "Drift/UI/UI.h"

// Sistema de input da Engine
#include "Drift/Engine/Input/Input.h"

// RHI para renderização
#include "Drift/RHI/DX11/UIBatcherDX11.h"
#include "Drift/RHI/DX11/RingBufferDX11.h"
```

## Estrutura da Aplicação

```cpp
struct AppData {
    // Sistema de input da Engine
    std::unique_ptr<Drift::Engine::Input::IInputManager> inputManager;
    
    // Sistema de UI
    std::unique_ptr<Drift::UI::UIContext> uiContext;
    
    // Renderização UI
    std::shared_ptr<Drift::RHI::IRingBuffer> uiRingBuffer;
    std::unique_ptr<Drift::RHI::IUIBatcher> uiBatcher;
    
    // RHI
    std::shared_ptr<Drift::RHI::ISwapChain> swapChain;
    std::shared_ptr<Drift::RHI::IContext> context;
};
```

## Inicialização

```cpp
int main() {
    try {
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
        Drift::RHI::DeviceDesc desc{ 1280, 720, /* vsync */ false };
        auto device = Drift::RHI::DX11::CreateDeviceDX11(desc);

        // Cria SwapChain e Context
        auto swapChain = device->CreateSwapChain(hwnd);
        auto context = device->CreateContext();
        
        // ================================
        // 3. SISTEMA DE INPUT
        // ================================
        
        // Cria o sistema de input da Engine
        auto inputManager = Drift::Engine::Input::CreateGLFWInputManager(window);
        
        // ================================
        // 4. SISTEMA DE UI
        // ================================
        
        // Cria o contexto de UI
        auto uiContext = std::make_unique<Drift::UI::UIContext>();
        uiContext->Initialize();
        
        // Conecta o sistema de input
        uiContext->SetInputManager(inputManager.get());
        
        // ================================
        // 5. RENDERIZAÇÃO UI
        // ================================
        
        // Cria ring buffer para UI
        auto uiRingBuffer = Drift::RHI::DX11::CreateRingBufferDX11(device.get(), 1024 * 1024);
        
        // Cria UIBatcher
        auto uiBatcher = Drift::RHI::DX11::CreateUIBatcherDX11(uiRingBuffer, context.get());
        uiBatcher->SetScreenSize(1280.0f, 720.0f);
        
        // ================================
        // 6. CRIAÇÃO DA INTERFACE
        // ================================
        
        // Método 1: Criação programática
        CreateUIProgrammatically(uiContext.get());
        
        // Método 2: Criação data-driven (recomendado)
        CreateUIDataDriven(uiContext.get());
        
        // ================================
        // 7. LOOP PRINCIPAL
        // ================================
        
        while (!glfwWindowShouldClose(window)) {
            float deltaTime = GetDeltaTime();
            
            // Atualizar input
            inputManager->Update();
            
            // Atualizar UI
            uiContext->Update(deltaTime);
            
            // Renderizar cena 3D
            // ... (código de renderização 3D)
            
            // Renderizar UI
            uiBatcher->Begin();
            uiContext->Render(*uiBatcher);
            uiBatcher->End();
            
            // Apresentar frame
            context->Present();
            
            glfwPollEvents();
        }
        
        // ================================
        // 8. LIMPEZA
        // ================================
        
        uiContext->Shutdown();
        glfwTerminate();
        
    } catch (const std::exception& e) {
        Drift::Core::Log("Erro: " + std::string(e.what()));
        return -1;
    }
    
    return 0;
}
```

## Criação Programática de UI

```cpp
void CreateUIProgrammatically(Drift::UI::UIContext* uiContext) {
    // Criar botões
    auto playButton = std::make_shared<Drift::UI::Button>(uiContext);
    playButton->SetText("Play Game");
    playButton->SetPosition(glm::vec2(100, 100));
    playButton->SetSize(glm::vec2(200, 50));
    
    auto settingsButton = std::make_shared<Drift::UI::Button>(uiContext);
    settingsButton->SetText("Settings");
    settingsButton->SetPosition(glm::vec2(100, 170));
    settingsButton->SetSize(glm::vec2(200, 50));
    
    auto quitButton = std::make_shared<Drift::UI::Button>(uiContext);
    quitButton->SetText("Quit");
    quitButton->SetPosition(glm::vec2(100, 240));
    quitButton->SetSize(glm::vec2(200, 50));
    
    // Configurar callbacks
    playButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
        Drift::Core::Log("Play button clicked!");
    });
    
    settingsButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
        Drift::Core::Log("Settings button clicked!");
    });
    
    quitButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
        Drift::Core::Log("Quit button clicked!");
        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
    });
    
    // Adicionar à hierarquia
    auto* root = uiContext->GetRoot().get();
    root->AddChild(playButton);
    root->AddChild(settingsButton);
    root->AddChild(quitButton);
}
```

## Criação Data-Driven de UI

```cpp
void CreateUIDataDriven(Drift::UI::UIContext* uiContext) {
    // Carregar folha de estilos
    auto styleSheet = std::make_shared<Drift::UI::StyleSheet>();
    styleSheet->LoadFromFile("ui/main_menu.css");
    
    // Carregar UI a partir de UXML
    auto parser = std::make_unique<Drift::UI::UXMLParser>(uiContext);
    parser->SetStyleSheet(styleSheet);
    
    auto rootElement = parser->LoadFromFile("ui/main_menu.uxml");
    uiContext->GetRoot()->AddChild(rootElement);
}
```

## Arquivos de Exemplo

### main_menu.uxml
```xml
<panel class="main-menu" id="root">
    <button class="primary" text="Play Game" />
    <button class="secondary" text="Settings" />
    <button class="danger" text="Quit" />
</panel>
```

### main_menu.css
```css
.main-menu {
    background-color: #2a2a2a;
    padding: 20px;
    display: flex;
    flex-direction: column;
    gap: 10px;
}

.button.primary {
    background-color: #4CAF50;
    color: white;
    padding: 10px 20px;
}

.button.secondary {
    background-color: #2196F3;
    color: white;
    padding: 10px 20px;
}

.button.danger {
    background-color: #F44336;
    color: white;
    padding: 10px 20px;
}
```

## Compilação

O sistema está configurado no CMakeLists.txt e pode ser compilado com:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

## Benefícios da Integração

1. **Sistema Unificado**: Usa o sistema de input existente da Engine
2. **Flexibilidade**: Suporte a criação programática e data-driven
3. **Performance**: Batching otimizado para renderização
4. **Extensibilidade**: Fácil adição de novos widgets
5. **WYSIWYG Ready**: Preparado para editor visual futuro 