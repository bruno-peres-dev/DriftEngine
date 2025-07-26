# Sistema de Fontes Refatorado - DriftEngine

## Visão Geral

O sistema de fontes do DriftEngine foi completamente refatorado para oferecer uma solução profissional, moderna e altamente otimizada para renderização de texto em jogos AAA. O novo sistema implementa MSDF (Multi-channel Signed Distance Field), suporte a múltiplas fontes, cache inteligente e otimizações avançadas de performance.

## Características Principais

### 🎯 Arquitetura Moderna
- **Design modular** com separação clara de responsabilidades
- **Thread safety** completo para desenvolvimento multithread
- **Memory management** inteligente com orçamento configurável
- **Extensibilidade** para futuras melhorias

### 📊 Qualidades de Renderização
- **Low**: 8x MSDF - Performance máxima
- **Medium**: 16x MSDF - Equilíbrio qualidade/performance
- **High**: 32x MSDF - Qualidade padrão (recomendado)
- **Ultra**: 64x MSDF - Qualidade máxima

### 🔧 Recursos Avançados
- **MSDF** para renderização nítida em qualquer escala
- **Anti-aliasing** subpixel configurável
- **Kerning** automático entre caracteres
- **Ligaduras** para fontes que suportam
- **Hinting** para melhor legibilidade
- **Correção gamma** configurável
- **Contraste** e suavização ajustáveis
- **Contornos** e efeitos visuais

## Arquitetura do Sistema

### Componentes Principais

#### FontSystem.h
Cabeçalho principal que define toda a interface pública do sistema:
- Estruturas de dados otimizadas
- Enums e configurações
- Classes principais (Font, FontManager, etc.)
- Utilitários e helpers

#### FontManager
Gerenciador singleton responsável por:
- Cache de fontes com LRU inteligente
- Carregamento e descarregamento de fontes
- Estatísticas de uso e performance
- Gerenciamento de memória automático
- Pré-carregamento e lazy loading

#### Font
Representa uma fonte carregada:
- Métricas da fonte (ascender, descender, line height)
- Cache de glyphs otimizado
- Atlas de textura MSDF
- Lazy loading de glyphs
- Thread safety completo

#### FontAtlas
Gerencia atlas de texturas MSDF:
- Alocação eficiente de regiões para glyphs
- Upload otimizado de dados MSDF
- Sistema de batching para performance
- Gerenciamento de espaço automático

#### MSDFGenerator
Gera campos de distância assinados:
- Conversão de TTF/OTF para MSDF
- Aplicação de filtros de qualidade
- Suporte a múltiplos canais
- Otimizações matemáticas avançadas

#### TextRenderer
Sistema de renderização otimizado:
- Batching inteligente de comandos
- Frustum culling automático
- Instancing para performance
- Integração com UIBatcher
- Estatísticas detalhadas

#### TextLayoutEngine
Motor de layout de texto:
- Quebra de linha automática
- Layout justificado
- Word wrapping inteligente
- Truncamento de texto
- Cálculo de posições de glyphs

## Uso Básico

### Inicialização

```cpp
#include "Drift/UI/FontSystem/FontSystem.h"

using namespace Drift::UI;

// Obter instância do FontManager
auto& fontManager = FontManager::GetInstance();

// Configurar cache otimizado
FontCacheConfig cacheConfig;
cacheConfig.maxFonts = 32;
cacheConfig.maxGlyphsPerFont = 2048;
cacheConfig.maxAtlasSize = 2048;
cacheConfig.enablePreloading = true;
cacheConfig.enableLazyLoading = true;
cacheConfig.memoryBudgetMB = 128.0f;
fontManager.SetCacheConfig(cacheConfig);

// Configurar qualidade padrão
fontManager.SetDefaultQuality(FontQuality::High);
fontManager.SetDefaultSize(16.0f);
fontManager.SetDefaultFontName("Arial");
```

### Carregamento de Fontes

```cpp
// Carregar fonte com qualidade alta
auto font = fontManager.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::High);

// Verificar se carregou com sucesso
if (font) {
    LOG_INFO("Fonte carregada: " + font->GetName());
    
    // Obter métricas
    const auto& metrics = font->GetMetrics();
    LOG_INFO("Line height: " + std::to_string(metrics.lineHeight));
    
    // Obter glyph
    const auto* glyph = font->GetGlyph('A');
    if (glyph && glyph->isValid) {
        LOG_INFO("Glyph 'A' carregado");
    }
}
```

### Renderização de Texto

```cpp
// Criar renderizador
TextRenderConfig renderConfig;
renderConfig.maxCommands = 512;
renderConfig.maxBatches = 32;
renderConfig.enableBatching = true;
renderConfig.enableFrustumCulling = true;

TextRenderer renderer(renderConfig);

// Configurar viewport
renderer.SetViewport(1920, 1080);

// Iniciar ciclo de renderização
renderer.BeginTextRendering();

// Adicionar texto simples
renderer.AddText("Hello World!", glm::vec2(100, 100), "Arial", 16.0f, 
                 glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

// Adicionar texto com configurações avançadas
TextRenderSettings settings;
settings.quality = FontQuality::High;
settings.enableKerning = true;
settings.enableSubpixel = true;
settings.gamma = 2.2f;
settings.contrast = 0.1f;
settings.smoothing = 0.1f;
settings.outlineWidth = 2.0f;
settings.outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

renderer.AddText("Texto Avançado", glm::vec2(100, 150), "Arial", 24.0f, 
                 glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), settings);

// Finalizar e processar
renderer.EndTextRendering();
```

### Medição de Texto

```cpp
// Medir tamanho de texto
glm::vec2 size = renderer.MeasureText("Hello World!", "Arial", 16.0f);
LOG_INFO("Tamanho: " + std::to_string(size.x) + "x" + std::to_string(size.y));

// Usar layout engine para cálculos avançados
TextLayoutEngine layoutEngine;

// Layout simples
TextRenderInfo layout = layoutEngine.CalculateLayout("Texto longo...", font, 400.0f);

// Layout com quebra de linha
std::vector<TextRenderInfo> multiLine = layoutEngine.CalculateMultiLineLayout("Texto muito longo...", font, 400.0f);

// Word wrap
std::vector<std::string> lines = layoutEngine.WordWrap("Texto para quebrar...", font, 400.0f);
```

## MSDF - Multi-channel Signed Distance Field

### Conceito

MSDF é uma técnica avançada para renderização de texto que oferece:
- **Qualidade nítida** em qualquer escala
- **Performance otimizada** comparada a outras técnicas
- **Anti-aliasing** automático
- **Suporte a múltiplos canais** para melhor qualidade

### Uso do MSDFGenerator

```cpp
// Criar gerador MSDF
MSDFConfig config;
config.width = 64;
config.height = 64;
config.range = 4.0f;
config.enableSubpixel = true;
config.enableSupersampling = true;
config.supersampleFactor = 4;

MSDFGenerator generator(config);

// Processar fonte
FontProcessor processor;
if (processor.LoadFont("fonts/arial.ttf")) {
    // Extrair contornos
    std::vector<Contour> contours;
    if (processor.ExtractGlyphContours('A', contours)) {
        // Gerar MSDF
        MSDFData msdfData;
        if (generator.GenerateFromContours(contours, msdfData)) {
            // Aplicar filtros de qualidade
            generator.ApplyQualityFilters(msdfData, TextRenderSettings{});
            
            // Converter para formato de textura
            std::vector<uint8_t> rgba8Data;
            generator.ConvertToRGBA8(msdfData, rgba8Data);
            
            LOG_INFO("MSDF gerado com sucesso");
        }
    }
}
```

### Pipeline Completo

```cpp
// Pipeline completo de processamento
FontProcessingPipeline pipeline;

// Processar fonte completa
pipeline.ProcessFont("fonts/arial.ttf", "output/arial_msdf.png");

// Processar glyph individual
MSDFData glyphData;
pipeline.ProcessGlyph('A', glyphData);

// Processar múltiplos glyphs
std::vector<uint32_t> codepoints = {'A', 'B', 'C', 'D', 'E'};
std::vector<MSDFData> glyphsData;
pipeline.ProcessGlyphs(codepoints, glyphsData);
```

## Otimizações de Performance

### Cache Inteligente

```cpp
// Configurar cache otimizado
FontCacheConfig cacheConfig;
cacheConfig.maxFonts = 32;           // Máximo de fontes
cacheConfig.maxGlyphsPerFont = 2048; // Máximo de glyphs por fonte
cacheConfig.maxAtlasSize = 2048;     // Tamanho máximo do atlas
cacheConfig.enablePreloading = true; // Habilitar pré-carregamento
cacheConfig.enableLazyLoading = true; // Habilitar lazy loading
cacheConfig.memoryBudgetMB = 128.0f; // Orçamento de memória

fontManager.SetCacheConfig(cacheConfig);
```

### Pré-carregamento

```cpp
// Pré-carregar fonte em múltiplos tamanhos
std::vector<float> sizes = {12.0f, 16.0f, 24.0f, 32.0f, 48.0f};
fontManager.PreloadFont("Arial", "fonts/arial.ttf", sizes, FontQuality::High);

// Pré-carregar caracteres específicos
std::vector<uint32_t> chars = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
fontManager.PreloadCharacters("Arial", chars, 16.0f, FontQuality::High);
```

### Batching Otimizado

```cpp
// Configurar renderizador para batching
TextRenderConfig config;
config.maxCommands = 1024;        // Máximo de comandos por frame
config.maxBatches = 64;           // Máximo de batches por frame
config.enableBatching = true;     // Habilitar batching
config.enableInstancing = true;   // Habilitar instancing
config.enableFrustumCulling = true; // Habilitar frustum culling

TextRenderer renderer(config);
```

### Estatísticas e Monitoramento

```cpp
// Obter estatísticas do sistema
FontStats fontStats = fontManager.GetStats();
LOG_INFO("Fontes carregadas: " + std::to_string(fontStats.totalFonts));
LOG_INFO("Glyphs carregados: " + std::to_string(fontStats.totalGlyphs));
LOG_INFO("Uso de memória: " + std::to_string(fontStats.memoryUsageBytes / 1024 / 1024) + " MB");
LOG_INFO("Taxa de acerto do cache: " + std::to_string(fontStats.cacheHitRate * 100) + "%");

// Obter estatísticas de renderização
TextRenderStats renderStats = renderer.GetStats();
LOG_INFO("Comandos renderizados: " + std::to_string(renderStats.commandsRendered));
LOG_INFO("Batches renderizados: " + std::to_string(renderStats.batchesRendered));
LOG_INFO("Vértices renderizados: " + std::to_string(renderStats.verticesRendered));
LOG_INFO("Draw calls: " + std::to_string(renderStats.drawCalls));
LOG_INFO("Tempo de renderização: " + std::to_string(renderStats.renderTime) + " ms");
```

## Configurações Avançadas

### Qualidades de Renderização

```cpp
// Configurações por qualidade
TextRenderSettings lowQuality;
lowQuality.quality = FontQuality::Low;
lowQuality.enableSubpixel = false;
lowQuality.enableKerning = false;
lowQuality.smoothing = 0.0f;

TextRenderSettings highQuality;
highQuality.quality = FontQuality::High;
highQuality.enableSubpixel = true;
highQuality.enableKerning = true;
highQuality.gamma = 2.2f;
highQuality.contrast = 0.1f;
highQuality.smoothing = 0.1f;

TextRenderSettings ultraQuality;
ultraQuality.quality = FontQuality::Ultra;
ultraQuality.enableSubpixel = true;
ultraQuality.enableKerning = true;
ultraQuality.enableLigatures = true;
ultraQuality.gamma = 2.2f;
ultraQuality.contrast = 0.15f;
ultraQuality.smoothing = 0.15f;
ultraQuality.outlineWidth = 1.0f;
```

### Configurações de Layout

```cpp
// Configurações de alinhamento
TextRenderSettings leftAlign;
leftAlign.alignment = TextAlignment::Left;

TextRenderSettings centerAlign;
centerAlign.alignment = TextAlignment::Center;

TextRenderSettings rightAlign;
rightAlign.alignment = TextAlignment::Right;

TextRenderSettings justifyAlign;
justifyAlign.alignment = TextAlignment::Justify;

// Configurações de espaçamento
TextRenderSettings spaced;
spaced.lineSpacing = 1.2f;      // Espaçamento entre linhas
spaced.letterSpacing = 2.0f;    // Espaçamento entre letras
spaced.wordSpacing = 5.0f;      // Espaçamento entre palavras
```

## Integração com RHI

### UIBatcher Integration

```cpp
// Criar adaptador para UIBatcher
UIBatcherTextRenderer uiRenderer(uiBatcher);

// Configurar tamanho da tela
uiRenderer.SetScreenSize(1920, 1080);

// Iniciar renderização
uiRenderer.BeginTextRendering();

// Adicionar texto simples
uiRenderer.AddText(100, 100, "Hello World!", 0xFFFFFFFF);

// Adicionar texto avançado
uiRenderer.AddText("Texto Avançado", glm::vec2(100, 150), "Arial", 16.0f, 
                   glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), settings);

// Finalizar renderização
uiRenderer.EndTextRendering();
```

### Shader Integration

```cpp
// Shader para MSDF (exemplo GLSL)
const char* msdfVertexShader = R"(
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 projection;
    mat4 view;
    mat4 model;
} ubo;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
    outColor = inColor;
}
)";

const char* msdfFragmentShader = R"(
#version 450

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D fontTexture;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
    vec2 unitRange = vec2(4.0);
    vec2 screenTexSize = vec2(textureSize(fontTexture, 0));
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main() {
    vec3 msd = texture(fontTexture, inTexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    outColor = inColor * vec4(1.0, 1.0, 1.0, opacity);
}
)";
```

## Melhores Práticas

### 1. Configuração Inicial

```cpp
// Sempre configurar o sistema no início da aplicação
void InitializeFontSystem() {
    auto& fontManager = FontManager::GetInstance();
    
    // Configurar cache apropriado para a plataforma
    FontCacheConfig config;
    #ifdef MOBILE
        config.maxFonts = 16;
        config.memoryBudgetMB = 64.0f;
    #else
        config.maxFonts = 64;
        config.memoryBudgetMB = 256.0f;
    #endif
    
    fontManager.SetCacheConfig(config);
    
    // Pré-carregar fontes essenciais
    fontManager.PreloadFont("UI", "fonts/ui.ttf", {12, 16, 24}, FontQuality::High);
    fontManager.PreloadFont("Title", "fonts/title.ttf", {32, 48, 64}, FontQuality::Ultra);
}
```

### 2. Gerenciamento de Memória

```cpp
// Monitorar uso de memória
void CheckMemoryUsage() {
    auto& fontManager = FontManager::GetInstance();
    FontStats stats = fontManager.GetStats();
    
    float memoryUsageMB = static_cast<float>(stats.memoryUsageBytes) / (1024.0f * 1024.0f);
    if (memoryUsageMB > 200.0f) { // 200 MB
        LOG_WARNING("Uso de memória alto: " + std::to_string(memoryUsageMB) + " MB");
        fontManager.TrimCache();
    }
}
```

### 3. Qualidade Adaptativa

```cpp
// Ajustar qualidade baseado na performance
void AdaptiveQuality() {
    static float frameTime = 0.0f;
    frameTime = GetFrameTime(); // Implementar função para obter tempo do frame
    
    FontQuality targetQuality;
    if (frameTime > 16.67f) { // Mais de 60 FPS
        targetQuality = FontQuality::Low;
    } else if (frameTime > 33.33f) { // Mais de 30 FPS
        targetQuality = FontQuality::Medium;
    } else {
        targetQuality = FontQuality::High;
    }
    
    // Aplicar qualidade apenas se mudou
    static FontQuality currentQuality = FontQuality::High;
    if (currentQuality != targetQuality) {
        currentQuality = targetQuality;
        // Atualizar configurações de renderização
    }
}
```

### 4. Cache de Renderização

```cpp
// Cache de comandos de renderização
class TextRenderCache {
private:
    std::unordered_map<std::string, TextRenderCommand> m_Cache;
    
public:
    void AddCommand(const std::string& key, const TextRenderCommand& command) {
        m_Cache[key] = command;
    }
    
    bool GetCommand(const std::string& key, TextRenderCommand& command) {
        auto it = m_Cache.find(key);
        if (it != m_Cache.end()) {
            command = it->second;
            return true;
        }
        return false;
    }
    
    void Clear() {
        m_Cache.clear();
    }
};
```

## Troubleshooting

### Problemas Comuns

#### 1. Fonte não carrega
```cpp
// Verificar se arquivo existe
if (!std::filesystem::exists("fonts/arial.ttf")) {
    LOG_ERROR("Arquivo de fonte não encontrado");
    return;
}

// Verificar formato suportado
// O sistema suporta TTF e OTF
```

#### 2. Performance baixa
```cpp
// Verificar estatísticas
FontStats stats = fontManager.GetStats();
if (stats.cacheHitRate < 0.8f) {
    LOG_WARNING("Taxa de acerto do cache baixa: " + std::to_string(stats.cacheHitRate * 100) + "%");
    // Considerar pré-carregar mais fontes
}

// Verificar uso de memória
if (stats.memoryUsageBytes > cacheConfig.memoryBudgetMB * 1024 * 1024) {
    LOG_WARNING("Uso de memória alto");
    fontManager.TrimCache();
}
```

#### 3. Qualidade visual ruim
```cpp
// Aumentar qualidade da fonte
settings.quality = FontQuality::Ultra;

// Ajustar configurações de renderização
settings.gamma = 2.2f;
settings.contrast = 0.1f;
settings.smoothing = 0.1f;
settings.enableSubpixel = true;
```

#### 4. Vazamento de memória
```cpp
// Verificar se fontes estão sendo descarregadas
fontManager.UnloadAllFonts();

// Verificar se renderizador está sendo limpo
renderer.ClearBatches();
```

## Conclusão

O sistema de fontes refatorado do DriftEngine oferece uma solução completa, moderna e otimizada para renderização de texto em jogos AAA. Com suas otimizações de performance, cache inteligente, suporte a MSDF e configurações avançadas, ele atende às demandas dos jogos modernos mantendo alta qualidade visual e performance.

### Principais Benefícios

1. **Performance máxima** com cache LRU e batching otimizado
2. **Qualidade visual superior** com MSDF e anti-aliasing avançado
3. **Gerenciamento de memória eficiente** com orçamento automático
4. **Thread safety completo** para desenvolvimento moderno
5. **Extensibilidade** para futuras melhorias
6. **Documentação profissional** para fácil manutenção

O sistema está pronto para uso em jogos AAA de qualquer escala, desde mobile até desktop de alta performance, oferecendo a melhor experiência possível para renderização de texto. 