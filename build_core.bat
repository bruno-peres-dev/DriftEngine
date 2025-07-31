@echo off
echo ========================================
echo Build do Modulo Core - DriftEngine
echo ========================================

REM Verificar se estamos no diretório correto
if not exist "src\core\CMakeLists.txt" (
    echo ERRO: Execute este script no diretório raiz do DriftEngine
    echo Diretório atual: %CD%
    exit /b 1
)

REM Criar diretório de build se não existir
if not exist "build_core" mkdir build_core
cd build_core

echo.
echo Configurando CMake para build isolado do Core...
cmake ..\src\core -DBUILD_CORE_ONLY=ON -DCMAKE_BUILD_TYPE=Debug
if %ERRORLEVEL% neq 0 (
    echo ERRO: Falha na configuração do CMake
    exit /b 1
)

echo.
echo Compilando módulo Core...
cmake --build . --config Debug
if %ERRORLEVEL% neq 0 (
    echo ERRO: Falha na compilação
    exit /b 1
)

echo.
echo ========================================
echo Build concluído com sucesso!
echo ========================================
echo.
echo Executável criado: build_core\Debug\CoreTest.exe
echo.
echo Para executar o teste:
echo   build_core\Debug\CoreTest.exe
echo.
echo Para limpar o build:
echo   cmake --build . --target clean
echo.

exit /b 0
