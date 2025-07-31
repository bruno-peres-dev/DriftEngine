# CMakeLists.txt para o módulo Core
# Permite build isolado do módulo Core para testes

# Configurações básicas
cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opção para build isolado do Core
option(BUILD_CORE_ONLY "Build only the Core module for testing" OFF)

if(BUILD_CORE_ONLY)
    # Se estamos fazendo build isolado, configurar como projeto independente
    project(DriftCore LANGUAGES CXX)
    
    # Incluir GLM como header-only
    find_package(GLM QUIET)
    if(NOT GLM_FOUND)
        # Fallback: usar caminho relativo se GLM não for encontrado
        set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../engine/include")
    endif()
    
    # Definir DriftCore como biblioteca
    add_library(DriftCore STATIC
        src/Log.cpp
        src/Profiler.cpp
        src/Assets/AssetsSystem.cpp
        src/Assets/AssetsExample.cpp
        src/Threading/ThreadingSystem.cpp
        src/Threading/ThreadingExample.cpp
    )
    
    # Configurar include directories
    target_include_directories(DriftCore PUBLIC
        include
        ${GLM_INCLUDE_DIR}
    )
    
    # Link libraries
    if(GLM_FOUND)
        target_link_libraries(DriftCore PUBLIC GLM::GLM)
    endif()
    
    # Criar executável de teste do Core
    add_executable(CoreTest
        src/LogProfilerExample.cpp
    )
    
    target_include_directories(CoreTest PUBLIC
        include
        ${GLM_INCLUDE_DIR}
    )
    
    target_link_libraries(CoreTest PUBLIC
        DriftCore
    )
    
    if(GLM_FOUND)
        target_link_libraries(CoreTest PUBLIC GLM::GLM)
    endif()
    
    # Configurações específicas para Windows
    if(WIN32)
        target_compile_definitions(DriftCore PRIVATE WIN32_LEAN_AND_MEAN)
        target_compile_definitions(CoreTest PRIVATE WIN32_LEAN_AND_MEAN)
    endif()
    
    # Configurações de debug
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(DriftCore PRIVATE _DEBUG)
        target_compile_definitions(CoreTest PRIVATE _DEBUG)
    endif()
    
    message(STATUS "Build isolado do módulo Core configurado")
    message(STATUS "Para compilar: cmake --build . --config Debug")
    message(STATUS "Para executar: ./CoreTest (Linux/Mac) ou CoreTest.exe (Windows)")
    
else()
    # Se não for build isolado, apenas definir a biblioteca
    # (será usado pelo CMakeLists.txt principal)
    add_library(DriftCore STATIC
        src/Log.cpp
        src/Profiler.cpp
        src/Assets/AssetsSystem.cpp
        src/Assets/AssetsExample.cpp
        src/Threading/ThreadingSystem.cpp
        src/Threading/ThreadingExample.cpp
    )
    
    target_include_directories(DriftCore PUBLIC
        include
    )
endif() 