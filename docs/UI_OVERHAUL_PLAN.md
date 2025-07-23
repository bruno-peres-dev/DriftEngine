# UI Overhaul Phases

Este documento resume as fases planejadas para o sistema de UI do DriftEngine. Elas guiam a evolução do framework e orientam as tarefas de desenvolvimento.

1. **Fundamentos do Core de Elementos**
   - Base `UIElement` com propriedades de posição, tamanho e transformação.
   - Suporte a visibilidade e opacidade.
   - Grafo de cena permitindo relação pai/filho e composições de transformações.
   - Interfaces de separação entre lógica e renderização.

2. **Layout Engine Genérico**
   - Sistema unificado de `Measure` e `Arrange` com invalidação por `dirty`.
   - Contêiners como StackPanel, Grid e Flexbox.

3. **Back-ends de Renderização**
   - Interface `IUIBatcher` abstrata com primitivas básicas.
   - Implementações específicas (DX11, futuramente Vulkan/OpenGL).

4. **Renderização de Texto**
   - Integração com bibliotecas como FreeType ou stb_truetype.
   - Geração de atlas e cálculo de métricas.

As fases posteriores incluem markup declarativo, sistema de estilos, eventos, data binding, animações e ferramentas de edição. Estas etapas servirão de referência para futuros desenvolvimentos.
