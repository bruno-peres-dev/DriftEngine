# Sistema de Stitching de Índices - Terreno AAA

## Visão Geral

O sistema de stitching de índices foi implementado para resolver problemas de continuidade visual entre tiles de terreno com diferentes níveis de LOD (Level of Detail). Este sistema garante transições suaves e sem cracks entre tiles adjacentes que possuem resoluções diferentes.

## Funcionalidades Implementadas

### 1. IndexStitcher Class

A classe `IndexStitcher` é responsável por gerar índices de transição entre tiles com diferentes resoluções:

- **Lookup Tables**: Padrões pré-computados para casos típicos (½, ⅓, ¼ da resolução)
- **Geração Dinâmica**: Algoritmos que criam índices de transição em tempo real
- **Suporte Completo**: Funciona para qualquer combinação de resoluções de LOD

### 2. Casos Suportados

O sistema suporta todos os casos de transição entre os LODs definidos:
- **LOD0**: 65x65 vértices (resolução máxima)
- **LOD1**: 33x33 vértices 
- **LOD2**: 17x17 vértices
- **LOD3**: 9x9 vértices (resolução mínima)

### 3. Algoritmo de Stitching

#### Para Tile com Resolução Maior que o Vizinho:
- Cria triângulos em leque conectando múltiplos vértices do tile atual a um vértice do vizinho
- Garante que não haja gaps visuais nas bordas

#### Para Tile com Resolução Menor que o Vizinho:
- Cria triângulos conectando um vértice do tile atual a múltiplos vértices do vizinho
- Mantém a continuidade da superfície

### 4. Otimizações

- **Cache de Índices**: Índices stitched são gerados apenas quando necessário
- **Buffers GPU**: Cada borda stitched tem seu próprio buffer de índices
- **Atualização Condicional**: Stitching é recalculado apenas quando LODs mudam

## Controles de Debug

### Teclas de Função:
- **F1**: Toggle wireframe
- **F2**: Toggle linhas normais
- **F3**: Toggle cores de LOD
- **F4**: Toggle visualização de stitching (NEW)
- **F5**: Toggle transições de LOD (NEW)
- **F6**: Toggle estatísticas

### Visualização de Stitching (F4):
Quando ativado, as bordas stitched são renderizadas em wireframe, permitindo visualizar exatamente onde as transições estão ocorrendo.

## Estatísticas de Performance

O sistema adiciona as seguintes métricas ao log de estatísticas:
- **Stitched**: Número de tiles que precisam de stitching
- **StitchedEdges**: Número total de bordas com stitching ativo

## Implementação Técnica

### Estruturas de Dados:

```cpp
// Em TerrainTile:
std::array<std::shared_ptr<Drift::RHI::IBuffer>, 4> stitchedIndexBuffers;
std::array<std::vector<uint32_t>, 4> stitchedIndices;
std::array<bool, 4> hasStitchedEdge;
```

### Fluxo de Execução:

1. **UpdateNeighborLODs()**: Verifica LODs dos tiles vizinhos
2. **GenerateStitchedIndicesForTile()**: Gera índices de transição se necessário
3. **Renderização**: Renderiza tile principal + bordas stitched separadamente

### Algoritmo de Mapeamento de Índices:

```cpp
uint32_t MapVertexIndex(uint32_t index, uint32_t fromRes, uint32_t toRes) {
    float ratio = static_cast<float>(toRes - 1) / static_cast<float>(fromRes - 1);
    return static_cast<uint32_t>(index * ratio + 0.5f);
}
```

## Casos de Uso

### Cenário Típico:
1. Player se aproxima de uma área - tiles próximos mudam para LOD0 (alta resolução)
2. Tiles distantes permanecem em LOD2/LOD3 (baixa resolução)
3. Sistema automaticamente gera stitching nas bordas entre diferentes LODs
4. Transição visual é suave e sem artifacts

### Performance:
- **Overhead Mínimo**: Stitching é computado apenas quando necessário
- **Memória Eficiente**: Buffers são criados sob demanda
- **GPU Friendly**: Usa buffers de índices separados para máxima eficiência

## Resolução de Problemas

### Cracks Visuais:
- Verificar se `needsStitching` está sendo definido corretamente
- Confirmar que vizinhos estão sendo detectados adequadamente
- Validar geração de índices de transição

### Performance:
- Monitor estatísticas de stitching (F6)
- Ajustar distâncias de LOD se necessário
- Verificar se buffers estão sendo reutilizados

### Debug Visual:
- Use F4 para visualizar bordas stitched
- Combine com F3 (cores de LOD) para identificar transições
- F1 (wireframe) ajuda a ver a topologia resultante

## Extensões Futuras

1. **Stitching Temporal**: Suavizar mudanças de LOD ao longo do tempo
2. **Stitching Adaptativo**: Ajustar densidade de triângulos baseado na curvatura
3. **Multi-threading**: Paralelizar geração de índices de stitching
4. **Caching Inteligente**: Reutilizar padrões de stitching comuns

---

**Nota**: Este sistema implementa padrões da indústria AAA para renderização de terreno, garantindo qualidade visual profissional com performance otimizada.