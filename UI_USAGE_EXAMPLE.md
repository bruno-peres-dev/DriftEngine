# Exemplo de Uso do Sistema de UI Evoluído

## Configuração Básica

### 1. Inicialização do Sistema

```cpp
#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/Engine/Input/Input.h"

// No main.cpp ou na inicialização da aplicação
auto uiContext = std::make_unique<Drift::UI::UIContext>();
uiContext->Initialize();

// Conectar ao sistema de input da Engine
auto inputManager = Drift::Engine::Input::CreateGLFWInputManager(window);
uiContext->SetInputManager(inputManager.get());
```

### 2. Criação de Interface

```cpp
// Criar botões
auto playButton = std::make_shared<Drift::UI::Button>(uiContext.get());
playButton->SetText("Play Game");
playButton->SetPosition(glm::vec2(100, 100));
playButton->SetSize(glm::vec2(200, 50));

auto settingsButton = std::make_shared<Drift::UI::Button>(uiContext.get());
settingsButton->SetText("Settings");
settingsButton->SetPosition(glm::vec2(100, 170));
settingsButton->SetSize(glm::vec2(200, 50));

auto quitButton = std::make_shared<Drift::UI::Button>(uiContext.get());
quitButton->SetText("Quit");
quitButton->SetPosition(glm::vec2(100, 240));
quitButton->SetSize(glm::vec2(200, 50));

// Adicionar à hierarquia
auto* root = uiContext->GetRoot().get();
root->AddChild(playButton);
root->AddChild(settingsButton);
root->AddChild(quitButton);
```

### 3. Configuração de Eventos

```cpp
// Callbacks para os botões
playButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
    Drift::Core::Log("Play button clicked!");
    // Iniciar jogo
});

settingsButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
    Drift::Core::Log("Settings button clicked!");
    // Abrir menu de configurações
});

quitButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
    Drift::Core::Log("Quit button clicked!");
    // Sair do jogo
    glfwSetWindowShouldClose(window, GLFW_TRUE);
});

// Eventos de hover (opcional)
playButton->SetOnHover([](const Drift::UI::ButtonHoverEvent& event) {
    if (event.isHovering) {
        Drift::Core::Log("Mouse entered play button");
    } else {
        Drift::Core::Log("Mouse left play button");
    }
});
```

### 4. Loop Principal

```cpp
// No loop principal da aplicação
while (!glfwWindowShouldClose(window)) {
    float deltaTime = GetDeltaTime();
    
    // Atualizar sistema de input da Engine
    inputManager->Update();
    
    // Atualizar UI (input é processado automaticamente)
    uiContext->Update(deltaTime);
    
    // Renderizar cena 3D
    // ...
    
    // Renderizar UI
    uiBatcher->Begin();
    uiContext->Render(*uiBatcher);
    uiBatcher->End();
    
    // Apresentar frame
    context->Present();
}
```

## Layout Automático

### Usando LayoutEngine

```cpp
// Criar um painel com layout vertical
auto mainPanel = std::make_shared<Drift::UI::UIElement>(uiContext.get());
mainPanel->SetSize(glm::vec2(400, 300));
mainPanel->SetPosition(glm::vec2(50, 50));

// Configurar layout vertical
Drift::UI::LayoutConfig config;
config.type = Drift::UI::LayoutType::Vertical;
config.spacing = 10.0f;
config.padding = Drift::UI::LayoutMargins(20.0f);
config.horizontalAlign = Drift::UI::HorizontalAlignment::Center;

// Adicionar botões ao painel
mainPanel->AddChild(playButton);
mainPanel->AddChild(settingsButton);
mainPanel->AddChild(quitButton);

// O LayoutEngine automaticamente posicionará os botões
```

## Personalização Visual

### Cores Personalizadas

```cpp
// Temas personalizados
playButton->SetNormalColor(0xFF4CAF50);   // Verde
playButton->SetHoverColor(0xFF45A049);    // Verde escuro
playButton->SetPressedColor(0xFF3D8B40);  // Verde mais escuro

settingsButton->SetNormalColor(0xFF2196F3);   // Azul
settingsButton->SetHoverColor(0xFF1976D2);    // Azul escuro
settingsButton->SetPressedColor(0xFF1565C0);  // Azul mais escuro

quitButton->SetNormalColor(0xFFF44336);   // Vermelho
quitButton->SetHoverColor(0xFFD32F2F);    // Vermelho escuro
quitButton->SetPressedColor(0xFFC62828);  // Vermelho mais escuro
```

### Estados Desabilitados

```cpp
// Desabilitar botão temporariamente
playButton->SetEnabled(false);

// O botão automaticamente mudará para a cor desabilitada
// e não responderá a eventos de mouse
```

## Integração com Sistema de Eventos

### Eventos Customizados

```cpp
// Definir eventos customizados
struct GameStartEvent {
    std::string levelName;
    int difficulty;
};

struct SettingsChangedEvent {
    float musicVolume;
    float sfxVolume;
    bool fullscreen;
};

// Publicar eventos
playButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
    GameStartEvent gameEvent{"Level1", 1};
    eventBus->Publish(gameEvent);
});

settingsButton->SetOnClick([](const Drift::UI::ButtonClickEvent& event) {
    SettingsChangedEvent settingsEvent{0.8f, 0.6f, true};
    eventBus->Publish(settingsEvent);
});

// Escutar eventos
auto handlerId = eventBus->Subscribe<GameStartEvent>([](const GameStartEvent& event) {
    Drift::Core::Log("Starting game: " + event.levelName + " (difficulty: " + 
                     std::to_string(event.difficulty) + ")");
});
```

## Uso Data-Driven (WYSIWYG)

### 1. Definição UXML
```xml
<!-- main_menu.uxml -->
<panel class="main-menu" id="root">
    <button class="primary" text="Play Game" />
    <button class="secondary" text="Settings" />
    <button class="danger" text="Quit" />
</panel>
```

### 2. Estilos CSS
```css
/* main_menu.css */
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

### 3. Carregamento Data-Driven
```cpp
#include "Drift/UI/DataDriven/UIXMLParser.h"
#include "Drift/UI/Styling/StyleSheet.h"

// Carregar estilos
auto styleSheet = std::make_shared<Drift::UI::StyleSheet>();
styleSheet->LoadFromFile("main_menu.css");

// Carregar UI
auto parser = std::make_unique<Drift::UI::UXMLParser>(uiContext.get());
parser->SetStyleSheet(styleSheet);

auto rootElement = parser->LoadFromFile("main_menu.uxml");
uiContext->GetRoot()->AddChild(rootElement);
```

## Próximos Passos

### 1. Sistema de Texto
```cpp
// Quando implementado
playButton->SetText("Play Game");
playButton->SetFont("Arial");
playButton->SetFontSize(16);
playButton->SetTextColor(0xFFFFFFFF);
```

### 2. Sistema de Temas
```cpp
// Quando implementado
auto theme = std::make_shared<Drift::UI::Theme>();
theme->colors.primary = glm::vec4(0.2f, 0.6f, 1.0f, 1.0f);
theme->colors.secondary = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
theme->spacing.margin = 10.0f;
theme->spacing.padding = 5.0f;

themeManager->ApplyTheme(theme);
```

### 3. Animações
```cpp
// Quando implementado
playButton->SetAnimation(Drift::UI::AnimationType::FadeIn, 0.5f);
playButton->SetTransition(Drift::UI::TransitionType::Scale, 0.2f);
```

## Benefícios do Sistema Evoluído

### Para Desenvolvedores
- **Produtividade**: Widgets prontos para uso
- **Consistência**: Comportamento padronizado
- **Flexibilidade**: Fácil customização
- **Manutenibilidade**: Código bem estruturado

### Para Usuários
- **Experiência**: Interface responsiva e moderna
- **Feedback Visual**: Estados claros (hover, pressed, disabled)
- **Performance**: Renderização otimizada
- **Acessibilidade**: Estados visuais bem definidos

### Para o Engine
- **Escalabilidade**: Arquitetura preparada para crescimento
- **Extensibilidade**: Fácil adição de novos widgets
- **Integração**: Sistema de eventos unificado
- **Performance**: Batching e culling otimizados 