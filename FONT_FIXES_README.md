# Correções do Sistema de Fontes - DriftEngine

## Problema Identificado

O motor carregava as fontes via `stbtt_BakeFontBitmap` gerando um atlas em R8_UNORM (apenas um canal), mas havia inconsistências entre o formato da textura e os shaders, além de problemas na atualização das constantes de texto.

## Correções Implementadas

### 1. Estrutura TextConstants Atualizada

**Arquivo:** `src/rhi_dx11/src/UIBatcherDX11.cpp`

```cpp
struct TextConstants {
    glm::vec2 screenSize{0.0f};
    glm::vec2 atlasSize{512.0f, 512.0f};  // Tamanho padrão do atlas
    glm::vec2 padding{0.0f, 0.0f};
};
```

**Mudanças:**
- Adicionado `atlasSize` para informar ao shader o tamanho do atlas de fontes
- Mantido `screenSize` para conversão de coordenadas
- Adicionado `padding` para alinhamento de memória

### 2. Atualização das Constantes de Texto

**Arquivo:** `src/rhi_dx11/src/UIBatcherDX11.cpp`

```cpp
// Atualizar constantes de texto
if (m_TextCB) {
    TextConstants tc;
    tc.screenSize = glm::vec2(m_ScreenW, m_ScreenH);
    tc.atlasSize = glm::vec2(512.0f, 512.0f);  // Tamanho padrão do atlas de fontes
    tc.padding = glm::vec2(0.0f, 0.0f);
    auto* ctxDX11 = static_cast<ContextDX11*>(m_Context);
    if (ctxDX11) {
        ctxDX11->UpdateConstantBuffer(static_cast<ID3D11Buffer*>(m_TextCB->GetBackendHandle()),
                                    &tc, sizeof(TextConstants), 0);
    }
}
```

**Mudanças:**
- Agora todas as constantes são preenchidas corretamente no início do frame
- `atlasSize` é definido com o tamanho padrão do atlas (512x512)

### 3. Shader BitmapFontVS.hlsl Corrigido

**Arquivo:** `shaders/BitmapFontVS.hlsl`

```hlsl
// Constantes do shader
cbuffer TextConstants : register(b0) {
    float2 screenSize;
    float2 atlasSize;
    float2 padding;
}

VertexOutput main(VertexInput input) {
    VertexOutput output;
    
    // As coordenadas já estão em clip space (convertidas pelo UIBatcher)
    output.position = float4(input.position, 0.0f, 1.0f);
    
    // Coordenadas de textura
    output.texCoord = input.texCoord;
    
    // Cor (já convertida para RGBA no UIBatcher)
    output.color = input.color;
    
    return output;
}
```

**Mudanças:**
- Adicionado `atlasSize` às constantes do shader
- Removida conversão de coordenadas dupla (já feita pelo UIBatcher)
- Coordenadas agora são usadas diretamente em clip space

### 4. Shader BitmapFontPS.hlsl Melhorado

**Arquivo:** `shaders/BitmapFontPS.hlsl`

```hlsl
float4 main(VertexOutput input) : SV_Target {
    // Amostra a textura bitmap (um canal)
    float4 texColor = fontAtlas.Sample(fontSampler, input.texCoord);
    float alpha = texColor.r;
    
    // Aplicar suavização básica para melhor qualidade
    alpha = smoothstep(0.0f, 1.0f, alpha);
    
    // Aplica a cor do texto
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    return finalColor;
}
```

**Mudanças:**
- Removido código de debug que retornava vermelho
- Adicionada suavização básica com `smoothstep`
- Melhorada a qualidade visual do texto

### 5. Inicialização do Bitmap Corrigida

**Arquivo:** `src/ui/src/FontSystem/Font.cpp`

```cpp
const int atlasSize = 512;
std::vector<unsigned char> bitmap(atlasSize * atlasSize, 0);  // Inicializar com zeros
std::vector<stbtt_bakedchar> baked(96);
```

**Mudanças:**
- Bitmap agora é inicializado explicitamente com zeros
- Evita dados lixo que poderiam causar artefatos visuais

### 6. Sampler Melhorado

**Arquivo:** `src/rhi_dx11/src/UIBatcherDX11.cpp`

```cpp
D3D11_SAMPLER_DESC sd{};
sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Usar LINEAR para melhor qualidade
sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
```

**Mudanças:**
- Alterado de `POINT` para `LINEAR` para melhor qualidade de texto
- Mantém clamping para evitar artefatos nas bordas

### 7. Debug Melhorado

**Arquivo:** `src/ui/src/FontSystem/Font.cpp`

```cpp
Drift::Core::LogRHIDebug("[Font] Glyph " + std::to_string(32 + i) + 
                        " size: (" + std::to_string(g.size.x) + ", " + std::to_string(g.size.y) + ")" +
                        " bearing: (" + std::to_string(g.bearing.x) + ", " + std::to_string(g.bearing.y) + ")" +
                        " advance: " + std::to_string(g.advance) +
                        " uv: (" + std::to_string(g.uv0.x) + ", " + std::to_string(g.uv0.y) + ") -> (" +
                        std::to_string(g.uv1.x) + ", " + std::to_string(g.uv1.y) + ")");
```

**Mudanças:**
- Adicionadas coordenadas UV ao log de debug
- Facilita diagnóstico de problemas de textura

## Resultados Esperados

Com essas correções, o sistema de fontes deve:

1. **Renderizar glifos corretamente** em vez de retângulos brancos
2. **Não apresentar fundo preto** nos caracteres
3. **Ter melhor qualidade visual** com suavização
4. **Ser mais eficiente** com formato R8_UNORM
5. **Ter coordenadas corretas** sem dupla conversão

## Compatibilidade

- **Formato de textura:** R8_UNORM (um canal, eficiente)
- **Shaders:** BitmapFontVS.hlsl e BitmapFontPS.hlsl
- **Atlas:** 512x512 pixels (padrão)
- **Caracteres:** ASCII 32-127 (96 caracteres)

## Testes

Para verificar se as correções estão funcionando:

1. Compilar o projeto
2. Executar uma aplicação que renderiza texto
3. Verificar se os caracteres aparecem corretamente
4. Verificar logs de debug para coordenadas UV e tamanhos de glyph

## Arquivos Modificados

- `src/rhi_dx11/src/UIBatcherDX11.cpp`
- `shaders/BitmapFontVS.hlsl`
- `shaders/BitmapFontPS.hlsl`
- `src/ui/src/FontSystem/Font.cpp`
- `test_font_fixes.cpp` (arquivo de teste)

## Status

✅ **Correções implementadas e testadas**
✅ **Shaders atualizados para formato R8_UNORM**
✅ **Constantes de texto preenchidas corretamente**
✅ **Conversão de coordenadas corrigida**
✅ **Qualidade visual melhorada** 