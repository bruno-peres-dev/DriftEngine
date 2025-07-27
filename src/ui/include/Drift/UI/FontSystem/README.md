# FontSystem - Sistema de Fontes Otimizado

## Visão Geral

O FontSystem do Drift Engine foi completamente reestruturado e otimizado para fornecer performance de nível AAA. Este sistema gerencia o carregamento, cache e renderização de fontes TTF com foco em eficiência e qualidade.

## Componentes Principais

### 1. Font (Font.h/cpp)
Representa uma fonte TTF individual com seus glyphs.

**Melhorias implementadas:**
- **Otimização de memória**: Uso de `std::vector` para caracteres ASCII comuns (32-127) e `unordered_map` apenas para Unicode
- **Qualidade configurável**: Diferentes tamanhos de atlas baseados na qualidade (256x256 a 2048x2048)
- **Documentação completa**: Todos os métodos documentados com Doxygen

### 2. FontManager (FontManager.h/cpp)
Gerencia o cache e carregamento de fontes com sistema LRU inteligente.

**Melhorias implementadas:**
- **Cache LRU**: Sistema de cache com remoção automática de itens menos usados
- **Compartilhamento de dados TTF**: Múltiplas fontes podem compartilhar os mesmos dados TTF
- **Configuração flexível**: Configuração de limites de cache e comportamentos
- **Estatísticas**: Monitoramento de hits/misses e uso de memória
- **Thread-safe**: Operações thread-safe com mutex

### 3. TextRenderer (TextRenderer.h/cpp)
Renderiza texto na tela com cache de medidas otimizado.

**Melhorias implementadas:**
- **Cache de medidas**: Cache de medidas de texto para evitar recálculos
- **Renderização otimizada**: Separação da renderização de glyphs individuais
- **Configuração de qualidade**: Suporte a kerning e renderização subpixel
- **Limpeza automática**: Cache auto-gerenciado com LRU

## Otimizações de Performance

### 1. Estrutura de Dados Otimizada
```cpp
// Antes: unordered_map para todos os glyphs
std::unordered_map<uint32_t, GlyphInfo> m_Glyphs;

// Depois: vector para ASCII + map para Unicode
std::vector<GlyphInfo> m_GlyphsASCII;  // O(1) para ASCII
std::unordered_map<uint32_t, GlyphInfo> m_GlyphsExtended;  // O(1) para Unicode
```

### 2. Cache Inteligente
- **LRU (Least Recently Used)**: Remove automaticamente itens menos usados
- **Compartilhamento de dados**: Múltiplas fontes compartilham dados TTF
- **Configuração flexível**: Limites configuráveis para cache

### 3. Medidas de Texto em Cache
```cpp
// Cache de medidas de texto
std::unordered_map<TextCacheKey, TextMeasureCache, TextCacheKeyHash> m_TextCache;
```

### 4. Logs Otimizados
- Remoção de logs desnecessários em produção
- Logs apenas para erros críticos
- Logs de debug apenas quando necessário

## Configuração

### FontCacheConfig
```cpp
struct FontCacheConfig {
    size_t maxFonts = 100;           // Máximo de fontes em cache
    size_t maxTTFData = 50;          // Máximo de arquivos TTF em cache
    bool enableLazyLoading = true;   // Carregamento sob demanda
    bool enablePreloading = true;    // Pré-carregamento
};
```

### TextRenderConfig
```cpp
struct TextRenderConfig {
    bool enableKerning = true;       // Kerning entre caracteres
    bool enableSubpixelRendering = true; // Renderização subpixel
    bool enableTextCache = true;     // Cache de medidas
    size_t maxCachedStrings = 1000;  // Máximo de strings em cache
};
```

## Uso

### Carregamento de Fonte
```cpp
auto& fm = FontManager::GetInstance();
fm.SetDevice(device);
fm.SetCacheConfig(config);

// Carregamento direto
auto font = fm.LoadFont("Arial", "fonts/arial.ttf", 16.0f, FontQuality::High);

// Lazy loading
auto font = fm.GetOrLoadFont("Arial", "fonts/arial.ttf", 16.0f);
```

### Renderização de Texto
```cpp
TextRenderer renderer;
renderer.SetBatcher(batcher);
renderer.SetConfig(textConfig);

renderer.AddText("Hello World", glm::vec2(100, 100), "Arial", 16.0f, glm::vec4(1, 1, 1, 1));
```

### Monitoramento
```cpp
auto stats = fm.GetCacheStats();
std::cout << "Cache hits: " << stats.cacheHits << std::endl;
std::cout << "Cache misses: " << stats.cacheMisses << std::endl;
std::cout << "Memory usage: " << stats.totalMemoryUsage << " bytes" << std::endl;
```

## Benefícios das Melhorias

1. **Performance**: Redução significativa no tempo de carregamento e renderização
2. **Memória**: Uso otimizado de memória com cache inteligente
3. **Escalabilidade**: Suporte a múltiplas fontes e tamanhos
4. **Qualidade**: Renderização de alta qualidade configurável
5. **Manutenibilidade**: Código bem documentado e organizado
6. **Thread-safety**: Operações seguras em ambiente multi-thread

## Compatibilidade

O sistema mantém compatibilidade com a API anterior, mas oferece novas funcionalidades opcionais para otimização. Todas as melhorias são retrocompatíveis. 