# UI Overhaul Phases

Este documento descreve as etapas planejadas para a reformulação do sistema de interface do DriftEngine. Cada fase possui tarefas principais e prioridades indicadas.

## Fase 1 – Fundamentos do Core de Elementos (Prioridade Máxima)

### UIElement Base
- Definir UIElement com propriedades centrais: posição, tamanho, visibilidade, opacidade, transformações.
- Implementar getters/setters eficientes (cache de layouts “dirty”).

### Árvore de Cena (Scene Graph)
- Suportar composição pai⇔filho e encadeamento de transformações.

### Métodos de travessia
- Pre-Render: atualizar transform, layout “dirty”.
- Post-Render: limpeza de flags e descarte de elementos invisíveis.

### Separacão Lógica × Render
- Definir interface abstrata `IUIBatcher` (batch de primitivas).
- UIElements enviam comandos ao batcher, sem chamar APIs gráficas diretas.

## Fase 2 – Layout Engine Genérico (Alta Prioridade)

### LayoutConfig e Propriedades
- Margens, paddings, alinhamentos (start/center/end/stretch), overflow.
- Propriedade de container: orientado (hor./vert.), wrap, proporções.

### Implementação de Layouts
- StackPanel (vertical & horizontal): empilhar elementos.
- Grid: linhas/colunas com sizing fixo, auto e proporções (“star”).
- Flexbox-like: wrap, flex-grow, flex-shrink, base-line.

### Engine Unificado
- `LayoutEngine::Measure()` e `LayoutEngine::Arrange()` recursivos.
- Sistema de invalidação: quando marcado “dirty”, reaplicar somente sub-árvore.

## Fase 3 – Abstração e Back-ends de Renderização

### Interface de Primitivas
- Métodos: `DrawRect()`, `DrawLine()`, `DrawTriangle()`, `DrawTexturedQuad()`.
- Estado de render: blend modes, scissor, transform stack.

### UIBatcher DX11 (refatorar código existente)
- Implementar pooling de buffers, uploads dinâmicos e instancing.

### UIBatcher Genérico
- Abstrair DX11 em `UIBatcherDX11` que implementa `IUIBatcher`.
- Planejar adaptação futura para Vulkan, OpenGL, Metal.

## Fase 4 – Renderização de Texto (Alta Prioridade)

### Integração de Biblioteca
- FreeType ou stb_truetype para carregar fontes TTF/OTF.

### Atlas e Métricas
- Gerar atlas estático ou dinâmico; armazenar advance, kerning.

### Layout de Texto
- Wrapping, alinhamento (left/center/right/justify).
- Suporte a bi-di e scripts complexos (UTF-8, glyph shaping, se necessário).

## Fase 5 – Markup Declarativo (Média Prioridade)

### Sintaxe XML/UXML
- Definir DTD/Schema mínimo para `<Grid>`, `<Stack>`, `<Button>`, etc.

### Parser Leve
- Ler XML, instanciar tipos por reflexão ou factory.
- Mapear atributos de XML para propriedades de `UIElement`.

### Recursos Embutidos
- Carregamento de texturas, fontes e estilos referenciados no markup.

## Fase 6 – Sistema de Estilos (USS/CSS-Like, Média)

### Seletores
- Por tipo (`Button`), ID (`#mainPanel`), classe (`.warning`).

### Properties
- Cores, fontes, bordas, sombras, espaçamentos.

### Cascade e Herança
- Ordem de precedência: inline > ID > classe > tipo > default.
- Temas (skins) carregáveis dinamicamente.

## Fase 7 – Input, Eventos e Foco (Média)

### Roteamento de Eventos
- Tunneling (de raiz até alvo) e Bubbling (do alvo até raiz).

### Estados de Interação
- Hover, pressed, disabled, focused – flags em `UIElement`.

### Commands e Callbacks
- Bindable commands (ICommand-like) ou callbacks diretas.

## Fase 8 – Data Binding / MVVM (Menor Prioridade)

### INotifyPropertyChanged
- Mecanismo de observação em view-model.

### Bindings
- Unidirecional e bidirecional, path traversal, atualização automática de UI.

### Conversores
- Funções para conversão de valor (bool→Visibility, número→string).

## Fase 9 – Sistema de Animações (Menor)

### Key-frames & Interpoladores
- Tipos: linear, ease-in/out, custom curves.

### Timeline & Gerenciamento
- Grupos de animação, retardo, repetição, ping-pong.

### Atuação em Propriedades
- Animação de transform, opacidade, cores, margens, etc.

## Fase 10 – Ferramentas Editor & Hot-Reload (Menor)

### Viewer/Editor In-Engine
- Janela que carrega UXML/USS e mostra árvore de cena editável.

### File Watcher
- Detectar mudanças em arquivos e recarregar árvore sem reiniciar.

### Painel de Propriedades
- Inspecionar e editar propriedades de qualquer `UIElement` em runtime.

## Fase 11 – Vetoriais e SVG (Opcional)

### Parsing SVG
- NanoSVG ou librsvg para ler caminhos.

### Tesselation
- Converter curvas em triângulos para enviar ao batcher.

### Primitivas
- Suportar círculos, elipses, polilinhas, paths complexos.

## Fase 12 – Temas Dinâmicos e Skins (Opcional)

### Gerenciamento de Skins
- Conjuntos de USS + recursos para dark/light ou variações.

### Runtime Switch
- Mudar tema global e reaplicar estilos imediatamente.
