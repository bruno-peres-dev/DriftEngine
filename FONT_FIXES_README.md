# Correções do Sistema de Fontes

## Problemas Identificados

### 1. Mismatch entre formato do atlas e expectativas do shader

**Problema**: O `Font.cpp` gerava um atlas usando `stbtt_BakeFontBitmap` e armazenava como textura R8_UNORM (um canal), mas o `TextPS.hlsl` estava escrito para dados MSDF (Multi-channel Signed Distance Field) que espera 3 canais RGB.

**Sintomas**: 
- Texto aparecia como retângulos sólidos (geralmente pretos)
- Shader interpretava canais ausentes como zeros, resultando em distância zero em todos os lugares

**Solução**:
- Modificado `Font.cpp` para converter bitmap R8 para RGBA8_UNORM
- Simplificado `TextPS.hlsl` para funcionar com texturas bitmap em vez de MSDF
- Removidas dependências MSDF desnecessárias

### 2. Posicionamento vertical incorreto de glyphs

**Problema**: Durante a renderização, o offset vertical de cada glyph (`bc.yoff`) estava sendo subtraído da baseline quando deveria ser adicionado.

**Sintomas**:
- Glyphs apareciam desalinhados verticalmente
- Baseline incorreta causava texto "flutuando" ou "afundando"

**Solução**:
- Corrigido em `TextRenderer.cpp` linha 47: `baseline - g->bearing.y` → `baseline + g->bearing.y`

## Arquivos Modificados

### 1. `src/ui/src/FontSystem/Font.cpp`
- **Linha 85-95**: Adicionada conversão de R8 para RGBA8
- **Linha 97**: Alterado formato de `R8_UNORM` para `R8G8B8A8_UNORM`
- **Linha 103**: Atualizado stride para 4 bytes por pixel
- **Linha 75-85**: Adicionado logging detalhado para debug de glyphs

### 2. `src/ui/src/FontSystem/TextRenderer.cpp`
- **Linha 47**: Corrigido posicionamento vertical: `baseline - g->bearing.y` → `baseline + g->bearing.y`

### 3. `shaders/TextPS.hlsl`
- Simplificado para funcionar com texturas bitmap
- Removidas funções MSDF complexas
- Mantida apenas funcionalidade básica de alpha blending

### 4. `shaders/TextVS.hlsl`
- Simplificado para remover dependências MSDF
- Removidas transformações complexas desnecessárias
- Mantida apenas transformação básica de view-projection

### 5. `shaders/BitmapFontPS.hlsl`
- Criado shader alternativo para fontes bitmap
- Funciona especificamente com texturas R8_UNORM

## Como Testar

1. Compile o projeto
2. Execute o teste: `test_font_fixes.cpp`
3. Verifique se o texto aparece corretamente sem retângulos pretos
4. Confirme que o alinhamento vertical está correto

## Próximos Passos (Opcional)

Para melhorar ainda mais a qualidade do texto, considere:

1. **Implementar MSDF real**: Adicionar biblioteca MSDF (como msdfgen) para gerar atlases MSDF de alta qualidade
2. **Anti-aliasing avançado**: Implementar subpixel rendering e ClearType
3. **Cache de glyphs**: Implementar cache dinâmico para glyphs não pré-baked
4. **Suporte a Unicode**: Expandir suporte para caracteres não-ASCII

## Notas Técnicas

- O `stb_truetype` `yoff` já é o offset correto da baseline para o topo do glyph
- Texturas RGBA8_UNORM são mais compatíveis com shaders modernos
- A conversão R8→RGBA8 duplica o valor alpha em todos os canais, mantendo compatibilidade 