# Sistema de Log e Profiler Profissional - Drift Engine

## Visão Geral

O Drift Engine agora possui um sistema de Log e Profiler completamente profissional e robusto, projetado para atender às necessidades de desenvolvimento, debugging e otimização de performance em projetos de jogos e aplicações gráficas.

## Características Principais

### Sistema de Log

#### ✅ Recursos Implementados
- **Múltiplos níveis de log**: Trace, Debug, Info, Warning, Error, Fatal
- **Formatação avançada**: Suporte a formatação estilo `fmt` com `{}`
- **Timestamps precisos**: Com precisão de milissegundos
- **Informações de contexto**: Arquivo, linha e função
- **Informações de thread**: Identificação de threads
- **Múltiplos outputs**: Console, arquivo e customizado
- **Configuração flexível**: Controle granular de recursos
- **Thread-safe**: Operação segura em ambientes multi-thread
- **Logging condicional**: Macros que verificam condições
- **Logging especializado**: RHI, Performance, Memória

#### 🔧 Configuração

```cpp
#include "Drift/Core/Log.h"

// Configuração básica
LogConfig config;
config.minLevel = LogLevel::Debug;
config.enableTimestamps = true;
config.enableThreadInfo = true;
config.enableFileInfo = true;
config.outputFile = "drift_engine.log";

g_LogSystem.Configure(config);
```

#### 📝 Uso Básico

```cpp
// Logging simples
DRIFT_LOG_INFO("Sistema inicializado");
DRIFT_LOG_WARNING("Recurso não encontrado");
DRIFT_LOG_ERROR("Falha crítica detectada");

// Logging com formatação
int valor = 42;
float pi = 3.14159f;
DRIFT_LOG_INFO("Valor: {}, Pi: {:.2f}", valor, pi);

// Logging condicional
bool debugMode = true;
LOG_DEBUG_IF(debugMode, "Debug ativado");

// Logging especializado
LOG_PERF("Operação crítica iniciada");
LOG_MEM("Alocando 1024 bytes");
LogRHI("DirectX inicializado");
```

### Sistema de Profiler

#### ✅ Recursos Implementados
- **Profiling hierárquico**: Seções aninhadas com parentesco
- **Estatísticas avançadas**: Média, desvio padrão, min/max
- **Profiling de memória**: Rastreamento de alocações
- **Profiling multi-thread**: Suporte a threads
- **Relatórios detalhados**: Exportação para arquivo
- **Configuração flexível**: Limites e recursos configuráveis
- **Thread-local storage**: Operação segura por thread
- **RAII automático**: Profiling automático com escopo
- **Profiling condicional**: Ativação baseada em condições

#### 🔧 Configuração

```cpp
#include "Drift/Core/Profiler.h"

// Configuração avançada
ProfilerConfig config;
config.enableProfiling = true;
config.enableThreadProfiling = true;
config.enableMemoryProfiling = true;
config.maxSections = 1000;
config.maxDepth = 32;
config.outputFile = "profiler_report.txt";

Profiler::GetInstance().Configure(config);
```

#### 📊 Uso Básico

```cpp
// Profiling simples
{
    PROFILE_SCOPE("Operação Crítica");
    // ... código ...
}

// Profiling de função
{
    PROFILE_FUNCTION();
    // ... código ...
}

// Profiling hierárquico
{
    PROFILE_SCOPE("Sistema Principal");
    
    {
        PROFILE_SCOPE_WITH_PARENT("Subsistema", "Sistema Principal");
        // ... código ...
    }
}

// Profiling condicional
bool enableProfiling = true;
{
    PROFILE_SCOPE_IF(enableProfiling, "Operação Detalhada");
    // ... código ...
}

// Profiling especializado
{
    PROFILE_PERF("Teste de Performance");
    PROFILE_RENDER("Renderização");
    PROFILE_UPDATE("Atualização");
    PROFILE_LOAD("Carregamento");
}
```

## Macros Disponíveis

### Logging

| Macro | Descrição | Exemplo |
|-------|-----------|---------|
| `DRIFT_LOG_TRACE(...)` | Log de trace (muito detalhado) | `DRIFT_LOG_TRACE("Entrando na função")` |
| `DRIFT_LOG_DEBUG(...)` | Log de debug | `DRIFT_LOG_DEBUG("Valor: {}", valor)` |
| `DRIFT_LOG_INFO(...)` | Log de informação | `DRIFT_LOG_INFO("Sistema pronto")` |
| `DRIFT_LOG_WARNING(...)` | Log de aviso | `DRIFT_LOG_WARNING("Recurso não encontrado")` |
| `DRIFT_LOG_ERROR(...)` | Log de erro | `DRIFT_LOG_ERROR("Falha: {}", erro)` |
| `DRIFT_LOG_FATAL(...)` | Log fatal | `DRIFT_LOG_FATAL("Erro crítico")` |
| `LOG_*_IF(cond, ...)` | Log condicional | `LOG_DEBUG_IF(debug, "Debug info")` |
| `LOG_PERF(...)` | Log de performance | `LOG_PERF("Operação lenta")` |
| `LOG_MEM(...)` | Log de memória | `LOG_MEM("Alocando {} bytes", size)` |

### Profiling

| Macro | Descrição | Exemplo |
|-------|-----------|---------|
| `PROFILE_SCOPE(name)` | Profiling de escopo | `PROFILE_SCOPE("Operação")` |
| `PROFILE_FUNCTION()` | Profiling de função | `PROFILE_FUNCTION()` |
| `PROFILE_SCOPE_WITH_PARENT(name, parent)` | Profiling hierárquico | `PROFILE_SCOPE_WITH_PARENT("Sub", "Main")` |
| `PROFILE_SCOPE_IF(cond, name)` | Profiling condicional | `PROFILE_SCOPE_IF(debug, "Debug")` |
| `PROFILE_PERF(name)` | Profiling de performance | `PROFILE_PERF("Teste")` |
| `PROFILE_RENDER(name)` | Profiling de renderização | `PROFILE_RENDER("Frame")` |
| `PROFILE_UPDATE(name)` | Profiling de atualização | `PROFILE_UPDATE("Logic")` |
| `PROFILE_LOAD(name)` | Profiling de carregamento | `PROFILE_LOAD("Asset")` |
| `PROFILE_MEMORY_ALLOC(size)` | Rastrear alocação | `PROFILE_MEMORY_ALLOC(1024)` |
| `PROFILE_MEMORY_DEALLOC(size)` | Rastrear desalocação | `PROFILE_MEMORY_DEALLOC(1024)` |

## Compatibilidade com Sistema de Fontes

O sistema mantém total compatibilidade com o sistema de fontes existente:

```cpp
// Código existente continua funcionando
DRIFT_LOG_INFO("Fonte carregada: {} ({}pt, {} glyphs)", 
               font->GetName(), size, glyphCount);

DRIFT_PROFILE_FUNCTION();
// ... código de carregamento de fonte ...
```

## Relatórios e Análise

### Relatório de Performance

```cpp
// Gerar relatório no console
Profiler::GetInstance().PrintReport();

// Exportar para arquivo
Profiler::GetInstance().ExportReport("performance.txt");

// Obter estatísticas específicas
auto stats = Profiler::GetInstance().GetSectionStats("Operação");
DRIFT_LOG_INFO("Média: {:.3f}ms", stats.GetAverageTimeMs());
DRIFT_LOG_INFO("Total: {:.3f}ms", stats.GetTotalTimeMs());
```

### Exemplo de Saída

```
=== RELATÓRIO DE PERFORMANCE ===
Gerado em: 2024-01-15 14:30:25.123
Total de seções: 15

Seção                          Calls    Avg (ms)    Total (ms)   Min (ms)    Max (ms)    Depth
----------------------------------------------------------------------------------------------------
Sistema Principal              1        150.250     150.250      150.250     150.250     0
  Renderização                 5        25.400      127.000      20.100      35.200      1
  Atualização                  5        15.200      76.000       12.500      18.300      1
  Carregamento                 3        45.667      137.000      30.100      65.400      1
----------------------------------------------------------------------------------------------------
```

## Configuração Avançada

### Output Customizado

```cpp
// Log customizado
LogConfig config;
config.customOutput = [](LogLevel level, const std::string& message) {
    // Enviar para servidor remoto
    // Atualizar interface gráfica
    // Sistema de alertas
};

// Profiler customizado
ProfilerConfig profilerConfig;
profilerConfig.customOutput = [](const std::string& report) {
    // Análise em tempo real
    // Interface de debug
    // Integração com ferramentas externas
};
```

### Configuração por Ambiente

```cpp
#ifdef _DEBUG
    LogConfig config;
    config.minLevel = LogLevel::Debug;
    config.enableFileInfo = true;
    config.outputFile = "debug.log";
#else
    LogConfig config;
    config.minLevel = LogLevel::Warning;
    config.enableFileInfo = false;
#endif

g_LogSystem.Configure(config);
```

## Performance e Overhead

### Logging
- **Overhead mínimo**: Verificação de nível antes da formatação
- **Thread-safe**: Operação segura sem bloqueios desnecessários
- **Formatação lazy**: Só formata se o nível permitir

### Profiling
- **Overhead baixo**: ~50-100ns por seção
- **Thread-local**: Sem contenção entre threads
- **Configurável**: Pode ser desabilitado em release

## Integração com Ferramentas

### Visual Studio
- Integração com Output Window
- Breakpoints condicionais baseados em log
- Análise de performance integrada

### Ferramentas Externas
- Exportação para formatos padrão
- Integração com profilers externos
- Análise de logs com ferramentas especializadas

## Boas Práticas

### Logging
1. **Use níveis apropriados**: Trace para detalhes, Error para problemas
2. **Formatação consistente**: Use `{}` para formatação
3. **Contexto útil**: Inclua informações relevantes
4. **Performance**: Evite logging excessivo em loops

### Profiling
1. **Nomes descritivos**: Use nomes que identifiquem a operação
2. **Hierarquia lógica**: Organize seções em hierarquia
3. **Granularidade adequada**: Não muito fino nem muito grosso
4. **Análise regular**: Revise relatórios periodicamente

## Migração do Código Existente

O sistema é totalmente compatível com o código existente. As macros `DRIFT_LOG_*` e `DRIFT_PROFILE_*` continuam funcionando exatamente como antes, mas agora com recursos muito mais avançados.

### Antes
```cpp
DRIFT_LOG_INFO("Fonte carregada: " + fontName);
DRIFT_PROFILE_FUNCTION();
```

### Depois (mesmo código, mais recursos)
```cpp
DRIFT_LOG_INFO("Fonte carregada: {}", fontName);  // Formatação melhorada
DRIFT_PROFILE_FUNCTION();  // Estatísticas avançadas
```

## Suporte e Manutenção

O sistema foi projetado para ser:
- **Extensível**: Fácil adição de novos recursos
- **Manutenível**: Código limpo e bem documentado
- **Robusto**: Tratamento de erros e edge cases
- **Eficiente**: Performance otimizada para uso em produção

Para dúvidas ou sugestões, consulte a documentação completa ou entre em contato com a equipe de desenvolvimento. 