# Sistema de Fontes Profissional - Drift Engine

## Visão Geral

O Sistema de Fontes Profissional do Drift Engine é uma solução completa e moderna para renderização de texto em jogos e aplicações gráficas. Integrado ao sistema de assets, oferece características de nível AAA com performance otimizada e qualidade superior.

## Características Principais

### 🎯 **Integração com AssetsSystem**
- Carregamento assíncrono de fontes
- Cache inteligente com LRU
- Sistema de prioridades
- Pré-carregamento automático

### 🎨 **Qualidade Profissional**
- Suporte a múltiplos formatos (TTF, OTF, WOFF, WOFF2)
- Renderização com anti-aliasing
- Suporte a MSDF (Multi-channel Signed Distance Field)
- Kerning e ligaduras tipográficas

### 🌍 **Internacionalização**
- Suporte completo a Unicode
- Sistema de fallback de fontes
- Caracteres especiais e acentuados
- Múltiplas direções de texto (LTR, RTL, TTB)

### ⚡ **Performance Otimizada**
- Batching automático de draw calls
- Cache de medidas de texto
- Atlas de glyphs compartilhados
- Renderização instanciada

### 🎭 **Efeitos Avançados**
- Sombras e outlines
- Gradientes de texto
- Efeitos de glow e emboss
- Múltiplos efeitos simultâneos

## Arquitetura do Sistema

```
FontSystem/
├── FontSystem.h          # Interface principal e configuração
├── Font.h               # Classe Font integrada ao AssetsSystem
├── FontManager.h        # Gerenciador de fontes com cache
├── FontAtlas.h          # Sistema de atlas de glyphs
├── FontMetrics.h        # Cálculos de layout e métricas
├── FontRendering.h      # Renderização de alta qualidade
└── TextRenderer.h       # Interface de alto nível
```

## Componentes Principais

### 1. FontSystem
Interface principal que coordena todos os componentes do sistema.

```cpp
// Inicialização
FontSystemConfig config;
config.enableAsyncLoading = true;
config.enablePreloading = true;
config.defaultQuality = FontQuality::High;

InitializeFontSystem(config);
```

### 2. Font
Representa uma fonte individual, integrada ao sistema de assets.

```cpp
// Carregamento via AssetsSystem
FontLoadConfig config;
config.size = 16.0f;
config.quality = FontQuality::High;
config.enableKerning = true;

auto font = DRIFT_LOAD_FONT_ASSET("fonts/Arial-Regular.ttf", config);
```

### 3. FontManager
Gerencia o cache e carregamento de fontes com otimizações.

```cpp
auto& fontManager = FontManager::GetInstance();

// Carregamento com fallback
auto font = fontManager.GetOrLoadFont("fonts/Arial-Regular.ttf", {16.0f, FontQuality::High});

// Pré-carregamento
fontManager.PreloadCommonSizes("fonts/Arial-Regular.ttf", {8, 12, 16, 24, 32, 48});
```

### 4. FontAtlas
Sistema de atlas de glyphs otimizado para performance.

```cpp
FontAtlasConfig atlasConfig;
atlasConfig.width = 1024;
atlasConfig.height = 1024;
atlasConfig.renderType = GlyphRenderType::MSDF;

auto atlas = FontAtlasManager::GetInstance().GetAtlas(atlasConfig);
```

### 5. FontMetrics
Cálculos precisos de layout e métricas tipográficas.

```cpp
FontMetrics metrics;
TextLayoutConfig layoutConfig;
layoutConfig.maxWidth = 300.0f;
layoutConfig.horizontalAlign = TextAlign::Justify;

auto layout = metrics.CalculateLayout(text, font, layoutConfig);
```

### 6. FontRendering
Renderização de alta qualidade com efeitos.

```cpp
FontRendering renderer(device);
renderer.Initialize();

TextRenderConfig renderConfig;
renderConfig.enableSubpixelRendering = true;
renderConfig.enableAntiAliasing = true;

renderer.RenderText(text, position, font, renderConfig);
```

### 7. TextRenderer
Interface de alto nível para renderização simplificada.

```cpp
TextRenderer textRenderer(device);
textRenderer.Initialize();

// Renderização simples
textRenderer.RenderText("Olá, Mundo!", glm::vec2(100, 100), font, glm::vec4(1.0f));

// Renderização com efeitos
textRenderer.RenderTextWithShadow(text, position, font, color, shadowOffset, shadowColor);
```

## Uso Básico

### 1. Inicialização

```cpp
#include "Drift/UI/FontSystem/FontSystem.h"
#include "Drift/Core/Assets/AssetsSystem.h"

// Inicializa o sistema de assets
auto& assetsSystem = Assets::AssetsSystem::GetInstance();
assetsSystem.Initialize();

// Registra o loader de fontes
auto fontLoader = std::make_unique<FontLoader>(device);
assetsSystem.RegisterLoader<Font>(std::move(fontLoader));

// Inicializa o sistema de fontes
FontSystemConfig config;
config.enableAsyncLoading = true;
config.defaultQuality = FontQuality::High;
InitializeFontSystem(config);
```

### 2. Carregamento de Fontes

```cpp
// Carregamento síncrono
auto font = DRIFT_LOAD_FONT("fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);

// Carregamento assíncrono
auto fontFuture = DRIFT_LOAD_FONT_ASYNC("fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);
auto font = fontFuture.get();

// Carregamento via assets
FontLoadConfig config;
config.size = 16.0f;
config.quality = FontQuality::High;
config.enableKerning = true;
auto font = DRIFT_LOAD_FONT_ASSET("fonts/Arial-Regular.ttf", config);
```

### 3. Renderização de Texto

```cpp
// Renderização básica
textRenderer.RenderText("Olá, Mundo!", glm::vec2(100, 100), font, glm::vec4(1.0f));

// Renderização com efeitos
textRenderer.RenderTextWithShadow(text, position, font, color, shadowOffset, shadowColor);
textRenderer.RenderTextWithOutline(text, position, font, color, outlineWidth, outlineColor);

// Renderização com layout
TextLayoutConfig layoutConfig;
layoutConfig.maxWidth = 300.0f;
layoutConfig.horizontalAlign = TextAlign::Center;
auto layout = textRenderer.CalculateLayout(text, font, layoutConfig);
textRenderer.RenderText(layout, position, font);
```

### 4. Medidas de Texto

```cpp
// Medida simples
auto size = textRenderer.MeasureText("Texto", font);

// Medida com layout
TextLayoutConfig layoutConfig;
layoutConfig.maxWidth = 300.0f;
auto size = textRenderer.MeasureText("Texto longo", font, layoutConfig);
```

## Configurações Avançadas

### FontSystemConfig

```cpp
FontSystemConfig config;

// Cache
config.maxFonts = 100;
config.maxAtlasCount = 20;
config.enableLazyLoading = true;

// Qualidade
config.enableSubpixelRendering = true;
config.enableKerning = true;
config.enableLigatures = true;

// Fallback
config.fallbackFonts = {"fonts/Arial-Regular.ttf", "fonts/Minecraft.ttf"};
config.enableUnicodeFallback = true;

// Performance
config.maxConcurrentLoads = 4;
config.enableAtlasSharing = true;
```

### TextRendererConfig

```cpp
TextRendererConfig config;

// Qualidade
config.enableSubpixelRendering = true;
config.enableAntiAliasing = true;
config.enableGammaCorrection = true;
config.gamma = 2.2f;

// Cache
config.enableTextCache = true;
config.maxCachedStrings = 1000;
config.maxCacheMemory = 10 * 1024 * 1024; // 10MB

// Performance
config.enableBatching = true;
config.enableInstancing = true;
config.maxBatchSize = 1000;

// Efeitos
config.enableEffects = true;
config.enableShadows = true;
config.enableOutlines = true;
config.enableGradients = true;
```

## Efeitos de Texto

### Sombra

```cpp
textRenderer.RenderTextWithShadow(
    "Texto com Sombra",
    glm::vec2(100, 100),
    font,
    glm::vec4(1.0f),
    glm::vec2(2.0f, 2.0f),
    glm::vec4(0.0f, 0.0f, 0.0f, 0.5f)
);
```

### Outline

```cpp
textRenderer.RenderTextWithOutline(
    "Texto com Outline",
    glm::vec2(100, 100),
    font,
    glm::vec4(1.0f),
    2.0f,
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
);
```

### Gradiente

```cpp
textRenderer.RenderTextWithGradient(
    "Texto com Gradiente",
    glm::vec2(100, 100),
    font,
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f)
);
```

### Múltiplos Efeitos

```cpp
TextRenderConfig config;
config.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

// Sombra
TextEffectConfig shadowEffect;
shadowEffect.type = TextEffect::Shadow;
shadowEffect.shadowOffset = glm::vec2(3.0f, 3.0f);
shadowEffect.shadowColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.7f);
config.effects.push_back(shadowEffect);

// Glow
TextEffectConfig glowEffect;
glowEffect.type = TextEffect::Glow;
glowEffect.glowRadius = 5.0f;
glowEffect.glowColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.5f);
config.effects.push_back(glowEffect);

textRenderer.RenderText("Texto com Efeitos", glm::vec2(100, 100), font, config);
```

## Layout de Texto

### Configuração de Layout

```cpp
TextLayoutConfig config;

// Alinhamento
config.horizontalAlign = TextAlign::Center;
config.verticalAlign = TextVerticalAlign::Middle;

// Espaçamento
config.lineSpacing = 1.2f;
config.wordSpacing = 0.0f;
config.letterSpacing = 0.0f;
config.paragraphSpacing = 1.5f;

// Quebra de linha
config.maxWidth = 300.0f;
config.enableWordWrap = true;
config.enableHyphenation = false;

// Renderização
config.enableKerning = true;
config.enableLigatures = true;
config.enableSubpixelRendering = true;
```

### Quebra de Linha

```cpp
auto lines = textRenderer.BreakTextIntoLines(
    "Texto longo que será quebrado em múltiplas linhas",
    font,
    300.0f
);
```

## Performance e Otimização

### Batching

```cpp
textRenderer.BeginTextRendering();

for (int i = 0; i < 100; ++i) {
    textRenderer.RenderText(
        "Texto " + std::to_string(i),
        glm::vec2(100 + i * 80, 100),
        font,
        glm::vec4(1.0f)
    );
}

textRenderer.EndTextRendering();
```

### Cache

```cpp
// Limpa cache de texto
textRenderer.ClearTextCache();

// Limpa cache de fontes
FontManager::GetInstance().ClearCache();

// Obtém estatísticas
auto stats = textRenderer.GetStats();
auto fontStats = FontManager::GetInstance().GetStats();
```

### Pré-carregamento

```cpp
auto& fontManager = FontManager::GetInstance();

// Pré-carrega tamanhos comuns
std::vector<float> sizes = {8, 12, 16, 24, 32, 48};
fontManager.PreloadCommonSizes("fonts/Arial-Regular.ttf", sizes);

// Pré-carrega caracteres específicos
std::vector<uint32_t> chars = {'A', 'B', 'C', 'D', 'E'};
fontManager.PreloadCharSet("fonts/Arial-Regular.ttf", chars);
```

## Macros Úteis

```cpp
// Carregamento
auto font = DRIFT_LOAD_FONT("fonts/Arial-Regular.ttf", 16.0f, FontQuality::High);
auto fontAsset = DRIFT_LOAD_FONT_ASSET("fonts/Arial-Regular.ttf", {16.0f, FontQuality::High});

// Renderização
DRIFT_RENDER_TEXT("Texto", glm::vec2(100, 100), "Arial", 16.0f, glm::vec4(1.0f));

// Medidas
auto size = DRIFT_MEASURE_TEXT("Texto", "Arial", 16.0f);

// Pré-carregamento
DRIFT_PRELOAD_FONT("fonts/Minecraft.ttf", {12.0f, FontQuality::Medium});
```

## Estatísticas e Debug

### Estatísticas do Sistema

```cpp
auto fontStats = FontManager::GetInstance().GetStats();
auto textStats = textRenderer.GetStats();

Core::Log("Fontes carregadas: " + std::to_string(fontStats.loadedFonts));
Core::Log("Cache hits: " + std::to_string(fontStats.cacheHits));
Core::Log("Uso de memória: " + std::to_string(fontStats.totalMemoryUsage / 1024) + " KB");
Core::Log("Caracteres renderizados: " + std::to_string(textStats.charactersRendered));
Core::Log("Draw calls: " + std::to_string(textStats.drawCalls));
```

### Log de Estatísticas

```cpp
FontManager::GetInstance().LogStats();
textRenderer.LogStats();
```

## Suporte a Formatos

### Formatos Suportados

- **TTF** (TrueType Font)
- **OTF** (OpenType Font)
- **WOFF** (Web Open Font Format)
- **WOFF2** (Web Open Font Format 2.0)
- **BMF** (Bitmap Font) - Futuro

### Qualidades de Renderização

- **Low**: Atlas 256x256
- **Medium**: Atlas 512x512
- **High**: Atlas 1024x1024
- **Ultra**: Atlas 2048x2048

## Exemplos Completos

Veja o arquivo `FontSystemExample.cpp` para exemplos completos de uso do sistema.

## Considerações de Performance

1. **Use batching** para renderizar múltiplos textos
2. **Pré-carregue** fontes e caracteres comuns
3. **Configure cache** adequadamente para seu uso
4. **Monitore estatísticas** para otimizações
5. **Use qualidades apropriadas** para cada caso de uso

## Troubleshooting

### Problemas Comuns

1. **Fonte não carrega**: Verifique se o arquivo existe e o formato é suportado
2. **Caracteres não aparecem**: Configure fontes de fallback
3. **Performance ruim**: Habilite batching e cache
4. **Qualidade baixa**: Aumente a qualidade da fonte

### Debug

```cpp
// Habilita logs detalhados
Core::Log::SetLevel(LogLevel::Debug);

// Verifica status da fonte
if (font->IsLoaded()) {
    Core::Log("Fonte carregada com sucesso");
} else {
    Core::Log("Falha ao carregar fonte");
}
```

## Conclusão

O Sistema de Fontes Profissional do Drift Engine oferece uma solução completa e moderna para renderização de texto, com características de nível AAA e integração perfeita com o sistema de assets. Sua arquitetura modular permite uso simples para casos básicos e configuração avançada para necessidades específicas. 