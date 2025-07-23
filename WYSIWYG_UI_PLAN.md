# Plano WYSIWYG para Sistema de UI do DriftEngine

## Visão Geral

Este documento descreve a evolução do sistema de UI do DriftEngine para uma ferramenta WYSIWYG (What You See Is What You Get) completa, permitindo criação visual de interfaces e hot-reload.

## Componentes Implementados

### ✅ **1. Sistema de Input Unificado**
- **Problema Resolvido**: Evitou duplicação do sistema de input
- **Solução**: `UIInputHandler` que usa o `IInputManager` existente da Engine
- **Benefícios**: Integração perfeita com o sistema de input existente

### ✅ **2. Registro de Componentes**
- **Problema Resolvido**: Criação data-driven de widgets
- **Solução**: `UIComponentRegistry` com factory pattern
- **Benefícios**: Widgets podem ser criados por nome, suporte a plugins

### ✅ **3. Sistema de Estilos CSS-like**
- **Problema Resolvido**: Estilização consistente e flexível
- **Solução**: `StyleSheet` com propriedades CSS-like
- **Benefícios**: Separação de layout e estilo, reutilização

### ✅ **4. Parser UXML**
- **Problema Resolvido**: Definições data-driven de UI
- **Solução**: `UXMLParser` para XML-like UI definitions
- **Benefícios**: Hot-reload, edição externa ao código

## Próximos Passos para WYSIWYG Completo

### 🔄 **Fase 1: LayoutEngine Avançado**

#### 1.1 **Flexbox Layout**
```cpp
// Implementar Flexbox completo
class FlexLayoutEngine {
    void LayoutFlexContainer(UIElement* container);
    void CalculateFlexItems(UIElement* container);
    void ApplyFlexProperties(UIElement* item);
};
```

#### 1.2 **Grid Layout**
```cpp
// Implementar CSS Grid
class GridLayoutEngine {
    void LayoutGridContainer(UIElement* container);
    void CalculateGridAreas(UIElement* container);
    void ApplyGridProperties(UIElement* item);
};
```

#### 1.3 **Dirty Flags System**
```cpp
// Otimização de performance
class DirtyFlagSystem {
    void MarkDirty(UIElement* element);
    void MarkDirtyRecursive(UIElement* element);
    void RecalculateOnlyDirty();
};
```

### 🔄 **Fase 2: Editor WYSIWYG**

#### 2.1 **Editor Standalone**
```cpp
// Editor independente
class UIEditor {
    void LoadUXMLFile(const std::string& filename);
    void SaveUXMLFile(const std::string& filename);
    void LivePreview();
    void HotReload();
};
```

#### 2.2 **Interface do Editor**
- **Paleta de Widgets**: Drag & drop de componentes
- **Inspetor de Propriedades**: Edição visual de atributos
- **Árvore de Elementos**: Hierarquia visual
- **Preview em Tempo Real**: Renderização instantânea

#### 2.3 **Integração com Runtime**
```cpp
// Mesmo UIContext usado no editor e runtime
class UIContext {
    void SetEditorMode(bool isEditor);
    void EnableHotReload(bool enable);
    void WatchFile(const std::string& filename);
};
```

### 🔄 **Fase 3: Sistema de Temas Avançado**

#### 3.1 **ThemeManager**
```cpp
class ThemeManager {
    void LoadTheme(const std::string& themeName);
    void ApplyTheme(UIElement* root);
    void CreateTheme(const std::string& name);
    void ExportTheme(const std::string& name);
};
```

#### 3.2 **Variáveis CSS**
```css
/* Suporte a variáveis CSS */
:root {
    --primary-color: #4CAF50;
    --secondary-color: #2196F3;
    --danger-color: #F44336;
    --spacing-unit: 8px;
}

.button.primary {
    background-color: var(--primary-color);
    margin: var(--spacing-unit);
}
```

#### 3.3 **Ícones Vetoriais**
```cpp
// Suporte a SVG
class IconSystem {
    void LoadSVGIcon(const std::string& name, const std::string& svgPath);
    void RenderIcon(const std::string& name, const glm::vec2& position);
};
```

### 🔄 **Fase 4: EventBus Expandido**

#### 4.1 **Eventos de Input**
```cpp
// Eventos específicos para UI
struct UIMouseEvent {
    glm::vec2 position;
    MouseButton button;
    UIElement* target;
};

struct UIKeyboardEvent {
    Key key;
    bool isPressed;
    UIElement* focusedElement;
};
```

#### 4.2 **Eventos de Editor**
```cpp
// Eventos para o editor WYSIWYG
struct ElementSelectedEvent {
    UIElement* element;
    bool isSelected;
};

struct PropertyChangedEvent {
    UIElement* element;
    std::string propertyName;
    std::string newValue;
};
```

### 🔄 **Fase 5: Performance e Batching**

#### 5.1 **Batching Avançado**
```cpp
// Otimizações de renderização
class AdvancedUIBatcher {
    void BatchQuads();
    void BatchText();
    void BatchSprites();
    void UseGPUInstancing();
};
```

#### 5.2 **Cache de Recursos**
```cpp
// Cache de texturas e fontes
class UIResourceCache {
    void CacheTexture(const std::string& name);
    void CacheFont(const std::string& name);
    void PreloadResources();
};
```

#### 5.3 **Culling Otimizado**
```cpp
// Renderizar apenas elementos visíveis
class UICullingSystem {
    void CullInvisibleElements(UIElement* root);
    void UpdateVisibility(UIElement* element);
};
```

## Implementação Prática

### Exemplo de Uso WYSIWYG

#### 1. **Definição UXML**
```xml
<!-- main_menu.uxml -->
<panel class="main-menu" id="root">
    <button class="primary" text="Play Game" />
    <button class="secondary" text="Settings" />
    <button class="danger" text="Quit" />
</panel>
```

#### 2. **Estilos CSS**
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
    background-color: var(--primary-color);
    color: white;
    padding: 10px 20px;
    transition: background-color 0.2s ease;
}

.button.primary:hover {
    background-color: var(--primary-color-dark);
}
```

#### 3. **Código C++**
```cpp
// Carregamento data-driven
auto parser = std::make_unique<UXMLParser>(uiContext.get());
parser->SetStyleSheet(styleSheet);

auto rootElement = parser->LoadFromFile("main_menu.uxml");
uiContext->GetRoot()->AddChild(rootElement);

// Hot-reload
uiContext->WatchFile("main_menu.uxml");
uiContext->WatchFile("main_menu.css");
```

### Editor WYSIWYG

#### Interface do Editor
```
┌─────────────────────────────────────────────────────────┐
│ UI Editor - DriftEngine                                 │
├─────────────────┬─────────────────┬─────────────────────┤
│ Widget Palette  │ Element Tree    │ Property Inspector  │
│                 │                 │                     │
│ [Button]        │ └─ root         │ Element: Button     │
│ [Panel]         │   ├─ button1    │ Text: "Play Game"   │
│ [Label]         │   ├─ button2    │ Class: "primary"    │
│ [Image]         │   └─ button3    │ Size: 200x50        │
│                 │                 │                     │
└─────────────────┴─────────────────┴─────────────────────┘
```

#### Funcionalidades
- **Drag & Drop**: Arrastar widgets da paleta para a árvore
- **Live Preview**: Visualização em tempo real
- **Property Editing**: Edição visual de propriedades
- **Hot Reload**: Atualização automática ao salvar arquivos
- **Undo/Redo**: Histórico de alterações
- **Export**: Geração de código C++

## Benefícios do Sistema WYSIWYG

### Para Desenvolvedores
- **Produtividade**: Criação visual rápida de interfaces
- **Iteração Rápida**: Hot-reload para desenvolvimento ágil
- **Separação de Responsabilidades**: Layout, estilo e lógica separados
- **Reutilização**: Componentes e estilos reutilizáveis

### Para Designers
- **Controle Visual**: Edição visual sem código
- **Prototipagem Rápida**: Criação rápida de protótipos
- **Consistência**: Sistema de temas unificado
- **Feedback Imediato**: Visualização instantânea

### Para o Engine
- **Escalabilidade**: Suporte a milhares de elementos
- **Performance**: Batching e culling otimizados
- **Extensibilidade**: Sistema de plugins
- **Manutenibilidade**: Código bem estruturado

## Cronograma de Implementação

### Mês 1: LayoutEngine Avançado
- Implementar Flexbox completo
- Implementar CSS Grid
- Sistema de dirty flags

### Mês 2: Editor WYSIWYG
- Interface básica do editor
- Drag & drop de widgets
- Property inspector

### Mês 3: Sistema de Temas
- ThemeManager completo
- Variáveis CSS
- Suporte a ícones SVG

### Mês 4: Performance e Integração
- Batching avançado
- Cache de recursos
- Integração completa

### Mês 5: Polimento e Documentação
- Testes e otimizações
- Documentação completa
- Exemplos e tutoriais

## Conclusão

O sistema WYSIWYG transformará o DriftEngine em uma ferramenta completa para criação de interfaces, combinando a flexibilidade de desenvolvimento visual com a performance e controle de um engine AAA. A arquitetura modular permite implementação incremental e fácil extensão futura. 