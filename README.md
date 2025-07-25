# DriftEngine

Um motor de jogos moderno em C++ com foco em renderização de alta qualidade e interface de usuário responsiva.

## Características Principais

### 🎮 **Sistema de Renderização**
- **DirectX 11** com pipeline moderno
- **Sistema de passes** modular e extensível
- **Shaders HLSL** otimizados
- **Renderização de terreno** com texturas

### 🎨 **Sistema de UI**
- **Layout engine** responsivo e flexível
- **Widgets** pré-definidos (Button, Panel, StackPanel, Grid)
- **Sistema de clipping** avançado
- **Padding e margens** funcionando corretamente
- **Bordas proporcionais** opcionais

### 🔤 **Sistema de Fontes Profissional**
- **MSDF (Multi-channel Signed Distance Field)** para anti-aliasing AAA
- **Subpixel rendering** para nitidez máxima
- **Font Atlas** para performance otimizada
- **Cache de glyphs** inteligente
- **Múltiplas qualidades** (Low, Medium, High, Ultra)

### ⚡ **Performance**
- **Batching** otimizado para UI
- **Ring buffer** para gerenciamento de memória
- **Layout incremental** (só recalcula o que mudou)
- **Cache inteligente** de recursos

## Compilação

### Pré-requisitos
- **Visual Studio 2019/2022** ou **CMake 3.15+**
- **Windows 10/11** (DirectX 11)
- **Git** para baixar dependências

### Passos de Compilação

```bash
# 1. Clone o repositório
git clone <repository-url>
cd DriftEngine

# 2. Crie e entre na pasta build
mkdir build
cd build

# 3. Configure o projeto
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 4. Compile
cmake --build . --config Debug
```

### Estrutura de Build
Após a compilação, você encontrará os seguintes executáveis na pasta `build/Debug/`:

- **DriftEngineApp.exe** - Aplicação principal
- **LayoutTest.exe** - Teste do sistema de layout
- **ClippingTest.exe** - Teste do sistema de clipping
- **FontTest.exe** - Teste do sistema de fontes profissional
- **LayoutFixesTest.exe** - Teste das correções de layout

## Executando os Testes

### 1. Teste de Layout Básico
```bash
./LayoutTest.exe
```
Demonstra o sistema de layout com diferentes tipos de containers e widgets.

### 2. Teste de Clipping
```bash
./ClippingTest.exe
```
Mostra o sistema de clipping funcionando com elementos que ultrapassam os limites dos containers.

### 3. Teste do Sistema de Fontes
```bash
./FontTest.exe
```
Demonstra o sistema de fontes profissional com:
- Diferentes tamanhos de fonte
- Múltiplas cores de texto
- Qualidades de renderização (Low, Medium, High, Ultra)
- Texto longo e formatação
- Testes de performance
- Controles de configuração

### 4. Teste das Correções de Layout
```bash
./LayoutFixesTest.exe
```
Mostra as correções implementadas:
- **Padding respeitado** pelos elementos filhos
- **Bordas proporcionais** que se ajustam ao tamanho
- **Clipping preciso** sem sobreposições
- **Layout responsivo** que se adapta ao redimensionamento

## Estrutura do Projeto

```
DriftEngine/
├── src/
│   ├── app/                    # Aplicações e testes
│   │   ├── main.cpp           # Aplicação principal
│   │   ├── layout_test.cpp    # Teste de layout
│   │   ├── clipping_test.cpp  # Teste de clipping
│   │   ├── font_test.cpp      # Teste de fontes
│   │   └── layout_fixes_test.cpp # Teste das correções
│   ├── core/                  # Sistema de logging
│   ├── engine/                # Sistemas do motor
│   ├── rhi/                   # Interface de renderização
│   ├── rhi_dx11/              # Implementação DirectX 11
│   ├── renderer/              # Sistema de renderização
│   └── ui/                    # Sistema de interface
│       ├── FontSystem/        # Sistema de fontes profissional
│       └── Widgets/           # Widgets da interface
├── shaders/                   # Shaders HLSL
│   ├── UIBatch.hlsl          # Shader para UI
│   ├── TextVS.hlsl           # Vertex shader para texto
│   └── TextPS.hlsl           # Pixel shader para texto
├── docs/                      # Documentação
└── extern/                    # Dependências externas
```

## Documentação

### Guias Específicos
- **[Sistema de Fontes](docs/FONT_SYSTEM_GUIDE.md)** - Guia completo do sistema de fontes profissional
- **[Correções de Layout](docs/LAYOUT_FIXES.md)** - Documentação das correções implementadas
- **[Melhorias de UI](docs/UI_LAYOUT_IMPROVEMENTS.md)** - Melhorias no sistema de layout

### Exemplos de Uso

#### Sistema de Fontes
```cpp
#include "Drift/UI/FontSystem/FontManager.h"

// Carregar fonte
auto& fontManager = UI::FontManager::GetInstance();
auto font = fontManager.LoadFont("default", "fonts/Roboto-Regular.ttf", 16.0f, UI::FontQuality::High);

// Renderizar texto
UI::TextRenderer::DrawText("Hello World", {100, 100}, "default", 16.0f);
```

#### Layout com Padding
```cpp
// Container com padding
auto container = std::make_shared<UI::Panel>(uiContext);
UI::LayoutProperties layout;
layout.padding = UI::LayoutMargins(20.0f).ToVec4();
container->SetLayoutProperties(layout);

// Filhos respeitam automaticamente o padding
auto button = std::make_shared<UI::Button>(uiContext);
container->AddChild(button);
```

#### Bordas Proporcionais
```cpp
// Painel com bordas proporcionais
auto panel = std::make_shared<UI::Panel>(uiContext);
panel->SetProportionalBorders(true);
panel->SetBorderProportion(0.015f); // 1.5% do menor lado
```

## Contribuindo

### Padrões de Código
- **C++17** com recursos modernos
- **Nomes descritivos** para variáveis e funções
- **Documentação** em português
- **Comentários** explicativos em código complexo

### Estrutura de Commits
- **feat:** Nova funcionalidade
- **fix:** Correção de bug
- **docs:** Documentação
- **refactor:** Refatoração de código
- **test:** Adição ou correção de testes

## Licença

Este projeto está sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.

## Roadmap

### Próximas Funcionalidades
- [ ] Suporte a Vulkan
- [ ] Sistema de partículas
- [ ] Áudio 3D
- [ ] Networking multiplayer
- [ ] Editor visual
- [ ] Suporte a VR/AR

### Melhorias Planejadas
- [ ] Compressão de texturas
- [ ] Streaming de assets
- [ ] Sistema de animação
- [ ] Interface de debug
- [ ] Profiling avançado

## Suporte

Para dúvidas, sugestões ou problemas:
1. Verifique a documentação na pasta `docs/`
2. Execute os testes para verificar se o problema é reproduzível
3. Abra uma issue no repositório com detalhes do problema

---

**DriftEngine** - Construindo o futuro dos jogos, um pixel de cada vez. 