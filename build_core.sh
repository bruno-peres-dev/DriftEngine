#!/bin/bash

echo "========================================"
echo "Build do Modulo Core - DriftEngine"
echo "========================================"

# Verificar se estamos no diretório correto
if [ ! -f "src/core/CMakeLists.txt" ]; then
    echo "ERRO: Execute este script no diretório raiz do DriftEngine"
    echo "Diretório atual: $(pwd)"
    exit 1
fi

# Criar diretório de build se não existir
mkdir -p build_core
cd build_core

echo ""
echo "Configurando CMake para build isolado do Core..."
cmake ../src/core -DBUILD_CORE_ONLY=ON -DCMAKE_BUILD_TYPE=Debug

if [ $? -ne 0 ]; then
    echo "ERRO: Falha na configuração do CMake"
    exit 1
fi

echo ""
echo "Compilando módulo Core..."
cmake --build . --config Debug

if [ $? -ne 0 ]; then
    echo "ERRO: Falha na compilação"
    exit 1
fi

echo ""
echo "========================================"
echo "Build concluído com sucesso!"
echo "========================================"
echo ""
echo "Executável criado: build_core/CoreTest"
echo ""
echo "Para executar o teste:"
echo "  ./CoreTest"
echo ""
echo "Para limpar o build:"
echo "  cmake --build . --target clean"
echo "" 