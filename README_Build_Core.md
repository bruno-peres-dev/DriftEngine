# Build Isolado do Módulo Core - DriftEngine

Este guia explica como compilar e testar apenas o módulo Core do DriftEngine, que contém os sistemas de Log e Profiler profissionais que foram implementados.

## Pré-requisitos

- **CMake** (versão 3.15 ou superior)
- **Compilador C++** com suporte a C++17:
  - Windows: Visual Studio 2019/2022 ou MinGW
  - Linux: GCC 7+ ou Clang 5+
  - macOS: Xcode 10+ ou Clang

## Métodos de Build

### 1. Usando Scripts Automatizados (Recomendado)

#### Windows
```bash
# Execute no diretório raiz do DriftEngine
build_core.bat
```

#### Linux/macOS
```bash
# Execute no diretório raiz do DriftEngine
./build_core.sh
```

### 2. Build Manual com CMake

```bash
# 1. Criar diretório de build
mkdir build_core
cd build_core

# 2. Configurar CMake
cmake ../src/core -DBUILD_CORE_ONLY=ON -DCMAKE_BUILD_TYPE=Debug

# 3. Compilar
cmake --build . --config Debug
```

## Estrutura do Build

Após o build bem-sucedido, você terá:

```
build_core/
├── Debug/                    # Arquivos de build (Windows)
│   ├── CoreTest.exe         # Executável de teste
│   ├── DriftCore.lib        # Biblioteca estática
│   └── [outros arquivos de build]
├── CMakeCache.txt
├── CMakeFiles/
└── [outros arquivos CMake]
```

## Executando os Testes

### Windows
```bash
build_core\Debug\CoreTest.exe
```

### Linux/macOS
```bash
./CoreTest
```

## O que o Teste Demonstra

O executável `CoreTest` demonstra todas as funcionalidades dos sistemas de Log e Profiler:

### Sistema de Log
- ✅ Configuração de diferentes níveis de log
- ✅ Formatação com `fmt`-like syntax (`"{}"`)
- ✅ Logging contextual (arquivo, linha, função)
- ✅ Logging condicional
- ✅ Logging especializado (RHI, Performance, Memória)
- ✅ Logging de exceções e HRESULT
- ✅ Múltiplas saídas (console, arquivo)

### Sistema de Profiler
- ✅ Profiling básico e hierárquico
- ✅ Profiling condicional
- ✅ Profiling de memória
- ✅ Estatísticas avançadas (média, variância, desvio padrão)
- ✅ Relatórios detalhados
- ✅ Profiling multi-thread
- ✅ Profiling especializado (render, update, load)

## Solução de Problemas

### Erro: "CMake não encontrado"
```bash
# Instalar CMake
# Windows: Baixar de https://cmake.org/download/
# Linux: sudo apt install cmake
# macOS: brew install cmake
```

### Erro: "Compilador não suporta C++17"
```bash
# Atualizar para uma versão mais recente do compilador
# Windows: Visual Studio 2019+ ou MinGW 8+
# Linux: GCC 7+ ou Clang 5+
# macOS: Xcode 10+
```

### Erro: "GLM não encontrado"
O build isolado do Core não requer GLM, mas se aparecer este erro:
```bash
# O sistema tentará usar o caminho relativo
# Se persistir, instale GLM:
# Windows: vcpkg install glm
# Linux: sudo apt install libglm-dev
# macOS: brew install glm
```

### Erro de Compilação
Verifique se todos os arquivos fonte estão presentes:
- `src/core/src/Log.cpp`
- `src/core/src/Profiler.cpp`
- `src/core/src/LogProfilerExample.cpp`
- `src/core/include/Drift/Core/Log.h`
- `src/core/include/Drift/Core/Profiler.h`

## Limpeza do Build

```bash
cd build_core
cmake --build . --target clean
```

Ou simplesmente delete a pasta `build_core`:
```bash
rm -rf build_core  # Linux/macOS
# ou
rmdir /s build_core  # Windows
```

## Integração com o Projeto Principal

O módulo Core pode ser compilado de duas formas:

1. **Isoladamente** (para testes): Use `-DBUILD_CORE_ONLY=ON`
2. **Como parte do projeto principal**: Use o CMakeLists.txt principal

Para integrar com o projeto principal, o Core é automaticamente incluído como dependência dos outros módulos (RHI, UI, etc.).

## Próximos Passos

Após testar o Core com sucesso:

1. **Testar integração**: Compile o projeto completo
2. **Verificar compatibilidade**: Teste com o sistema de fontes
3. **Otimizar configurações**: Ajuste níveis de log e profiling conforme necessário
4. **Documentar**: Atualize a documentação do projeto

## Suporte

Se encontrar problemas:
1. Verifique se está no diretório correto
2. Confirme que o CMake está instalado e funcionando
3. Verifique se o compilador suporta C++17
4. Consulte os logs de erro do CMake 