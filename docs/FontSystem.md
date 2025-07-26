# FontSystem - Sistema de Fontes Otimizado para AAA

## Visão Geral

O FontSystem do DriftEngine é um sistema completo e otimizado para renderização de texto em jogos AAA. Ele oferece suporte a múltiplas qualidades de renderização, cache inteligente, carregamento lazy e otimizações de memória.

## Características Principais

### 🎯 Otimizações AAA
- **Cache LRU inteligente** com gerenciamento automático de memória
- **Carregamento lazy** de glyphs para economia de memória
- **Pré-carregamento** de fontes e caracteres essenciais
- **Thread safety** completo para renderização multithread
- **MSDF (Multi-channel Signed Distance Field)** para qualidade máxima
- **Atlas de texturas** otimizado para performance

### 📊 Qualidades de Renderização
- **Low**: 8x MSDF - Performance máxima
- **Medium**: 16x MSDF - Equilíbrio qualidade/performance
- **High**: 32x MSDF - Qualidade padrão (recomendado)
- **Ultra**: 64x MSDF - Qualidade máxima

### 🔧 Recursos Avançados
- **Kerning** automático entre caracteres
- **Ligaduras** para fontes que suportam
- **Hinting** para melhor legibilidade
- **Anti-aliasing** subpixel
- **Correção gamma** configurável
- **Contraste** e suavização ajustáveis

## Arquitetura

### Componentes Principais

#### FontManager
Gerenciador singleton responsável por:
- Cache de fontes com LRU
- Carregamento e descarregamento de fontes
- Estatísticas de uso
- Gerenciamento de memória

#### Font
Representa uma fonte carregada:
- Métricas da fonte (ascender, descender, line height)
- Cache de glyphs
- Atlas de textura
- Lazy loading de glyphs

#### FontAtlas
Gerencia atlas de texturas:
- Alocação de regiões para glyphs
- Upload de dados MSDF
- Otimização de espaço

#### MSDFGenerator
Gera campos de distância assinados:
- Conversão de TTF para MSDF
- Aplicação de filtros de qualidade
- Suporte a múltiplos canais

#### TextRenderer
Sistema de renderização:
- Batching de comandos de texto
- Integração com UIBatcher
- Processamento otimizado

## Uso Básico

### Carregando uma Fonte

```cpp
#include "Drift/UI/FontSystem/FontManager.h"

using namespace Drift::UI;

// Obter instância do FontManager
auto& fontManager = FontManager::GetInstance();

// Carregar fonte com qualidade alta
auto font = fontManager.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::High);

// Usar a fonte
if (font) {
    glm::vec2 size = font->MeasureText("Hello World");
    const Glyph* glyph = font->GetGlyph('A');
}
```

### Renderizando Texto

```cpp
#include "Drift/UI/FontSystem/TextRenderer.h"

// Configurar renderização
TextRenderer renderer;
renderer.BeginTextRendering();

// Adicionar texto
TextRenderSettings settings;
settings.quality = FontQuality::High;
settings.enableKerning = true;
settings.gamma = 2.2f;

renderer.AddText("Hello World", glm::vec2(100, 100), "Arial", 16.0f, 
                 glm::vec4(1.0f), settings);

// Finalizar e processar
renderer.EndTextRendering();
```

### Configurações Avançadas

```cpp
// Configurar cache
FontCacheConfig cacheConfig;
cacheConfig.maxFonts = 64;
cacheConfig.maxGlyphsPerFont = 4096;
cacheConfig.memoryBudgetMB = 256.0f;
fontManager.SetCacheConfig(cacheConfig);

// Pré-carregar fontes
std::vector<float> sizes = {12.0f, 16.0f, 24.0f, 32.0f};
fontManager.PreloadFont("Arial", "fonts/arial.ttf", sizes, FontQuality::High);

// Pré-carregar caracteres específicos
std::vector<uint32_t> chars = {'A', 'B', 'C', 'D', 'E'};
fontManager.PreloadCharacters("Arial", chars, 16.0f, FontQuality::High);
```

## Otimizações de Performance

### Cache Inteligente
- **LRU (Least Recently Used)**: Remove fontes não utilizadas
- **Orçamento de memória**: Limita uso de memória automaticamente
- **Cache hits/misses**: Estatísticas para otimização

### Carregamento Lazy
- Glyphs são carregados apenas quando necessários
- Reduz tempo de inicialização
- Economiza memória

### Batching
- Comandos de texto são agrupados em batches
- Reduz chamadas de renderização
- Melhora performance GPU

### Atlas Otimizado
- Múltiplos glyphs em uma única textura
- Reduz mudanças de textura
- Melhora performance de renderização

## Configurações de Qualidade

### FontQuality::Low
- **Uso**: UI de debug, elementos distantes
- **Performance**: Máxima
- **Memória**: Mínima
- **Qualidade**: Básica

### FontQuality::Medium
- **Uso**: UI geral, HUD
- **Performance**: Alta
- **Memória**: Baixa
- **Qualidade**: Boa

### FontQuality::High (Padrão)
- **Uso**: Texto principal, menus
- **Performance**: Equilibrada
- **Memória**: Média
- **Qualidade**: Excelente

### FontQuality::Ultra
- **Uso**: Texto crítico, títulos
- **Performance**: Baixa
- **Memória**: Alta
- **Qualidade**: Máxima

## Estatísticas e Monitoramento

```cpp
// Obter estatísticas do sistema
FontStats stats = fontManager.GetStats();

std::cout << "Fontes carregadas: " << stats.totalFonts << std::endl;
std::cout << "Glyphs carregados: " << stats.totalGlyphs << std::endl;
std::cout << "Uso de memória: " << stats.memoryUsageBytes / 1024 / 1024 << " MB" << std::endl;
std::cout << "Taxa de acerto do cache: " << stats.cacheHitRate * 100 << "%" << std::endl;
```

## Thread Safety

O FontSystem é completamente thread-safe:
- **FontManager**: Mutex para operações de cache
- **Font**: Mutex para acesso a glyphs
- **TextRenderer**: Thread-safe para adição de comandos

## Integração com RHI

O sistema se integra com o RHI (Render Hardware Interface):
- **Texturas**: Criação e gerenciamento de atlas
- **UIBatcher**: Renderização otimizada
- **Shaders**: Suporte a MSDF

## Melhores Práticas

### 1. Pré-carregamento
```cpp
// Carregar fontes essenciais no início
fontManager.PreloadFont("UI", "fonts/ui.ttf", {12, 16, 24}, FontQuality::High);
fontManager.PreloadFont("Title", "fonts/title.ttf", {32, 48, 64}, FontQuality::Ultra);
```

### 2. Qualidade Apropriada
```cpp
// Usar qualidade baixa para elementos distantes
renderer.AddText(debugText, pos, "Debug", 12.0f, color, FontQuality::Low);

// Usar qualidade alta para texto principal
renderer.AddText(mainText, pos, "Main", 16.0f, color, FontQuality::High);
```

### 3. Gerenciamento de Memória
```cpp
// Configurar orçamento apropriado
fontManager.SetMemoryBudget(512.0f); // 512 MB

// Monitorar uso
if (stats.memoryUsageBytes > 400 * 1024 * 1024) {
    fontManager.TrimCache();
}
```

### 4. Cache de Fontes
```cpp
// Reutilizar fontes carregadas
auto font = fontManager.GetFont("Arial", 16.0f, FontQuality::High);
if (!font) {
    font = fontManager.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::High);
}
```

## Troubleshooting

### Problemas Comuns

#### Fonte não carrega
```cpp
// Verificar se arquivo existe
if (!std::filesystem::exists("fonts/arial.ttf")) {
    LOG_ERROR("Font file not found");
    return;
}

// Verificar permissões de arquivo
// Verificar formato de fonte suportado (TTF/OTF)
```

#### Performance baixa
```cpp
// Verificar estatísticas
FontStats stats = fontManager.GetStats();
if (stats.cacheHitRate < 0.8f) {
    // Considerar pré-carregar mais fontes
}

// Verificar uso de memória
if (stats.memoryUsageBytes > cacheConfig.memoryBudgetMB * 1024 * 1024) {
    // Reduzir qualidade ou número de fontes
}
```

#### Qualidade visual ruim
```cpp
// Aumentar qualidade da fonte
settings.quality = FontQuality::Ultra;

// Ajustar configurações de renderização
settings.gamma = 2.2f;
settings.contrast = 0.1f;
settings.smoothing = 0.1f;
```

## Conclusão

O FontSystem do DriftEngine oferece uma solução completa e otimizada para renderização de texto em jogos AAA. Com suas otimizações de performance, cache inteligente e suporte a múltiplas qualidades, ele atende às demandas dos jogos modernos mantendo alta qualidade visual e performance.

Para mais informações, consulte os arquivos de cabeçalho em `src/ui/include/Drift/UI/FontSystem/`. 