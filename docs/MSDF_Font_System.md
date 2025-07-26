# Sistema de Fontes MSDF - DriftEngine

## Visão Geral

O Sistema de Fontes MSDF (Multi-channel Signed Distance Field) do DriftEngine é uma implementação avançada e profissional para renderização de texto de alta qualidade. Este sistema oferece:

- **Renderização de alta qualidade** em qualquer resolução
- **Suporte completo a Unicode** e múltiplos idiomas
- **Efeitos visuais avançados** (outline, sombra, blur)
- **Otimização de performance** com cache de glyphs
- **Sistema de fallback** para fontes
- **Renderização subpixel** para melhor qualidade

## Características Principais

### 🎯 Qualidade Profissional
- **MSDF (Multi-channel Signed Distance Field)** para bordas suaves
- **Renderização subpixel** para melhor legibilidade
- **Suavização adaptativa** baseada na resolução
- **Kerning automático** para espaçamento perfeito

### 🌍 Suporte Internacional
- **Unicode completo** (UTF-8/UTF-16)
- **Múltiplos idiomas** (português, japonês, cirílico, etc.)
- **Emojis e símbolos especiais**
- **Sistema de fallback** inteligente

### ⚡ Performance Otimizada
- **Atlas de texturas** dinâmico
- **Cache de glyphs** em memória
- **Batching otimizado** para renderização
- **Pré-carregamento** de caracteres comuns

### 🎨 Efeitos Visuais
- **Outline configurável** com cor e largura
- **Sombra com blur** e offset
- **Cores personalizáveis** com alpha
- **Animações suaves** de texto

## Estrutura do Sistema

### Componentes Principais

```
DriftEngine/
├── src/ui/include/Drift/UI/FontSystem/
│   └── MSDFFont.h              # Interface principal
├── src/ui/src/FontSystem/
│   └── MSDFFont.cpp            # Implementação
├── shaders/
│   ├── MSDFFontVS.hlsl         # Vertex shader
│   └── MSDFFontPS.hlsl         # Pixel shader MSDF
└── src/app/
    └── msdf_font_example.cpp   # Exemplo de uso
```

### Classes Principais

#### `MSDFFontSystem`
Sistema principal de gerenciamento de fontes.

```cpp
class MSDFFontSystem {
    // Inicialização
    bool Initialize(Drift::RHI::IDevice* device);
    void Shutdown();
    
    // Carregamento de fontes
    bool LoadFont(const std::string& path, const std::string& name, float fontSize);
    bool LoadFontFromMemory(const uint8_t* data, size_t size, const std::string& name);
    
    // Renderização
    TextRenderResult RenderText(const std::string& text, const glm::vec2& position, 
                               const TextRenderSettings& settings = TextRenderSettings(),
                               const TextLayoutInfo& layout = TextLayoutInfo());
    
    // Medição
    glm::vec2 MeasureText(const std::string& text, const TextRenderSettings& settings);
    
    // Cache
    void PreloadGlyphs(const std::string& text, const std::string& fontName = "");
    void ClearGlyphCache();
};
```

#### `MSDFAtlas`
Gerenciador de atlas de texturas para glyphs.

```cpp
class MSDFAtlas {
    // Alocação de espaço
    bool AllocateGlyph(uint32_t width, uint32_t height, int& x, int& y);
    void FreeGlyph(int x, int y, uint32_t width, uint32_t height);
    
    // Gerenciamento de textura
    void UpdateTexture(const uint8_t* data, int x, int y, uint32_t width, uint32_t height);
    Drift::RHI::ITexture* GetTexture() const;
    
    // Estatísticas
    float GetUsageRatio() const;
    size_t GetGlyphCount() const;
};
```

## Configurações de Renderização

### `TextRenderSettings`
Configurações avançadas para renderização de texto.

```cpp
struct TextRenderSettings {
    float fontSize = 16.0f;                    // Tamanho da fonte
    float lineHeight = 1.2f;                   // Altura da linha
    float letterSpacing = 0.0f;                // Espaçamento entre letras
    float wordSpacing = 0.0f;                  // Espaçamento entre palavras
    Drift::Color color = 0xFFFFFFFF;           // Cor do texto
    
    // Efeitos
    float outlineWidth = 0.0f;                 // Largura do outline
    Drift::Color outlineColor = 0xFF000000;    // Cor do outline
    float shadowOffsetX = 0.0f;                // Deslocamento da sombra X
    float shadowOffsetY = 0.0f;                // Deslocamento da sombra Y
    float shadowBlur = 0.0f;                   // Blur da sombra
    Drift::Color shadowColor = 0x80000000;     // Cor da sombra
    
    // Configurações MSDF
    float msdfPixelRange = 2.0f;               // Range de pixels MSDF
    float msdfEdgeThreshold = 0.5f;            // Threshold de bordas
    bool enableSmoothing = true;                // Suavização
    bool enableSubpixelRendering = true;       // Renderização subpixel
    bool enableKerning = true;                 // Kerning
    bool enableLigatures = true;               // Ligatures
};
```

### `TextLayoutInfo`
Informações de layout e posicionamento.

```cpp
struct TextLayoutInfo {
    glm::vec2 position;                        // Posição inicial
    glm::vec2 size;                            // Tamanho da área
    float maxWidth = 0.0f;                     // Largura máxima
    float maxHeight = 0.0f;                    // Altura máxima
    bool wordWrap = true;                      // Quebra de linha
    bool ellipsis = false;                     // Mostrar "..."
    glm::vec2 alignment = {0.0f, 0.0f};       // Alinhamento
    float tabWidth = 4.0f;                     // Largura da tab
};
```

## Exemplos de Uso

### Inicialização Básica

```cpp
#include "Drift/UI/UI.h"

// Criar sistema de fontes
auto fontSystem = std::make_unique<Drift::UI::FontSystem>();
fontSystem->Initialize(device);

// Carregar fontes
fontSystem->LoadFont("fonts/Arial.ttf", "Arial", 16.0f);
fontSystem->SetDefaultFont("Arial");

// Configurações básicas
Drift::UI::TextSettings settings;
settings.fontSize = 24.0f;
settings.color = 0xFFFFFFFF;

// Renderizar texto
Drift::UI::TextLayout layout;
layout.position = glm::vec2(100.0f, 100.0f);
layout.alignment = glm::vec2(0.0f, 0.0f); // Centralizado

auto result = fontSystem->RenderText("Olá, Mundo!", layout.position, settings, layout);
```

### Texto com Efeitos

```cpp
// Configurações com outline e sombra
Drift::UI::TextSettings styledSettings;
styledSettings.fontSize = 32.0f;
styledSettings.color = 0xFFFFD700;           // Dourado
styledSettings.outlineWidth = 2.0f;
styledSettings.outlineColor = 0xFF000000;    // Preto
styledSettings.shadowOffsetX = 3.0f;
styledSettings.shadowOffsetY = 3.0f;
styledSettings.shadowBlur = 4.0f;
styledSettings.shadowColor = 0x60000000;     // Sombra semi-transparente

// Renderizar texto estilizado
fontSystem->RenderText("Texto Estilizado", glm::vec2(200.0f, 200.0f), styledSettings);
```

### Texto Multilinha

```cpp
std::string longText = "Este é um texto longo que será renderizado com quebra de linha automática. "
                      "O sistema MSDF suporta quebra de palavras e alinhamento profissional.";

Drift::UI::TextLayout multilineLayout;
multilineLayout.position = glm::vec2(50.0f, 300.0f);
multilineLayout.maxWidth = 400.0f;           // Largura máxima
multilineLayout.wordWrap = true;             // Quebra de linha
multilineLayout.alignment = glm::vec2(0.0f, 0.0f); // Centralizado

auto result = fontSystem->RenderText(longText, multilineLayout.position, settings, multilineLayout);
```

### Medição de Texto

```cpp
// Medir dimensões do texto
glm::vec2 textSize = fontSystem->MeasureText("Texto para medir", settings);
printf("Largura: %.1f, Altura: %.1f\n", textSize.x, textSize.y);

// Usar para posicionamento
Drift::UI::TextLayout centeredLayout;
centeredLayout.position = glm::vec2(
    (screenWidth - textSize.x) * 0.5f,  // Centralizar horizontalmente
    (screenHeight - textSize.y) * 0.5f   // Centralizar verticalmente
);
```

### Cache de Glyphs

```cpp
// Pré-carregar caracteres comuns
std::string commonChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
fontSystem->PreloadGlyphs(commonChars, "Arial");

// Pré-carregar caracteres especiais
std::wstring specialChars = L"áéíóúâêîôûãõçÁÉÍÓÚÂÊÎÔÛÃÕÇ";
fontSystem->PreloadGlyphs(specialChars, "Arial");
```

## Shaders MSDF

### Vertex Shader (`MSDFFontVS.hlsl`)
Processa vértices e aplica transformações.

**Funcionalidades:**
- Transformações de posição e escala
- Rotação com pivô configurável
- Alinhamento de texto
- Conversão para clip space

### Pixel Shader (`MSDFFontPS.hlsl`)
Implementa o algoritmo MSDF para renderização.

**Funcionalidades:**
- Amostragem MSDF com suavização
- Renderização subpixel RGB
- Efeitos de outline e sombra
- Múltiplas versões otimizadas

## Otimizações

### Atlas de Texturas
- **Alocação dinâmica** de espaço para glyphs
- **Árvore binária** para gerenciamento eficiente
- **Reutilização** de espaço liberado
- **Compressão** de texturas

### Cache de Glyphs
- **Cache em memória** de glyphs renderizados
- **Pré-carregamento** de caracteres comuns
- **LRU eviction** para gerenciamento de memória
- **Compressão** de dados de glyphs

### Batching
- **Agrupamento** de draw calls
- **Redução** de mudanças de estado
- **Otimização** de vértices e índices
- **Instancing** para texto repetitivo

## Suporte a Idiomas

### Português
```cpp
// Caracteres acentuados
fontSystem->RenderText("Olá, como você está?", position, settings);
```

### Japonês
```cpp
// Caracteres Hiragana/Katakana
std::wstring japaneseText = L"こんにちは世界";
fontSystem->RenderText(japaneseText, position, settings);
```

### Cirílico
```cpp
// Alfabeto cirílico
std::wstring russianText = L"Привет мир";
fontSystem->RenderText(russianText, position, settings);
```

### Emojis
```cpp
// Emojis e símbolos
std::wstring emojiText = L"🎮🚀⚡🎯🎨";
fontSystem->RenderText(emojiText, position, settings);
```

## Estatísticas e Debug

### Monitoramento de Performance
```cpp
auto stats = fontSystem->GetStats();
printf("Fontes carregadas: %zu\n", stats.loadedFonts);
printf("Glyphs em cache: %zu\n", stats.cachedGlyphs);
printf("Uso do atlas: %.1f%%\n", stats.atlasUsageRatio * 100.0f);
printf("Chamadas de render: %zu\n", stats.renderCalls);
```

### Debug Visual
```cpp
// Mostrar bounds do texto
uiBatcher->AddRect(textBounds.x, textBounds.y, textBounds.width, textBounds.height, 0x40FF0000);

// Mostrar glyphs individuais
for (const auto& glyph : glyphs) {
    uiBatcher->AddRect(glyph.atlasX, glyph.atlasY, glyph.atlasWidth, glyph.atlasHeight, 0x4000FF00);
}
```

## Integração com UI

### Widgets de Texto
```cpp
class TextWidget : public Drift::UI::UIElement {
private:
    std::string text;
    Drift::UI::TextSettings settings;
    Drift::UI::TextLayout layout;
    
public:
    void SetText(const std::string& newText) {
        text = newText;
        InvalidateLayout();
    }
    
    void Render(Drift::RHI::IUIBatcher* batcher) override {
        auto result = fontSystem->RenderText(text, GetPosition(), settings, layout);
        SetSize(result.bounds);
    }
};
```

### Sistema de Layout
```cpp
// Integração com LayoutEngine
class TextLayoutElement : public Drift::UI::LayoutElement {
    void Measure() override {
        auto size = fontSystem->MeasureText(text, settings);
        SetDesiredSize(size);
    }
    
    void Arrange(const Drift::UI::Rect& bounds) override {
        layout.position = bounds.position;
        layout.size = bounds.size;
    }
};
```

## Considerações de Performance

### Melhores Práticas
1. **Pré-carregue glyphs** comuns no início
2. **Reutilize configurações** de texto
3. **Use cache** para textos estáticos
4. **Limite o número** de fontes carregadas
5. **Monitore estatísticas** de uso

### Otimizações Avançadas
- **LOD (Level of Detail)** para texto distante
- **Culling** de texto fora da tela
- **Compressão** de atlas de texturas
- **Multithreading** para geração de glyphs

## Troubleshooting

### Problemas Comuns

**Texto borrado:**
- Ajuste `msdfPixelRange` e `msdfEdgeThreshold`
- Verifique se `enableSubpixelRendering` está ativo
- Aumente a resolução do atlas

**Performance baixa:**
- Reduza o número de fontes carregadas
- Aumente o tamanho do atlas
- Use cache de glyphs
- Monitore estatísticas de uso

**Caracteres não renderizados:**
- Verifique se a fonte suporta o caractere
- Configure fontes de fallback
- Verifique encoding UTF-8/UTF-16

**Vazamento de memória:**
- Chame `ClearGlyphCache()` periodicamente
- Monitore uso do atlas
- Verifique se fontes são descarregadas corretamente

## Conclusão

O Sistema de Fontes MSDF do DriftEngine oferece uma solução completa e profissional para renderização de texto em jogos e aplicações. Com suporte a múltiplos idiomas, efeitos visuais avançados e otimizações de performance, é ideal para projetos que exigem qualidade e flexibilidade na renderização de texto.

Para mais informações, consulte os exemplos no código e a documentação da API. 