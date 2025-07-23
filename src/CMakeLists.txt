cmake_minimum_required(VERSION 3.15)
project(DriftEngine LANGUAGES CXX)

# ------------------------------------------------
# C++17 e compile_commands.json
# ------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------
# 1) FetchContent para GLFW e GLM
# ------------------------------------------------
include(FetchContent)

# 1.1) GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# 1.2) GLM (header‑only) — Populate only
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
FetchContent_Populate(glm)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE
  "${glm_SOURCE_DIR}"
)

# ------------------------------------------------
# 2) DirectXTK (core + WIC loader) via subdirectory
# ------------------------------------------------

add_subdirectory(../extern/DirectXTK "extern/DirectXTK-build")

# ------------------------------------------------
# 3) DriftEngine Libraries
# ------------------------------------------------

# 3.1) DriftMath (header‑only)
add_library(DriftMath INTERFACE)
target_include_directories(DriftMath INTERFACE
  "${CMAKE_SOURCE_DIR}/src/engine/include"
)

# 3.1.1) DriftEngine (engine systems)
add_library(DriftEngine STATIC
  engine/src/Input/GLFWInputManager.cpp
)
target_include_directories(DriftEngine PUBLIC
  engine/include
  rhi/include
  renderer/include
  ${glfw_SOURCE_DIR}/include
)
target_link_libraries(DriftEngine PUBLIC
  DriftCore
  DriftMath
  glm
  glfw
)

# 3.2) DriftCore (logging)
add_library(DriftCore STATIC
  core/src/Log.cpp
)
target_include_directories(DriftCore PUBLIC
  core/include
)

# 3.3) DriftRHI (interfaces)
add_library(DriftRHI STATIC
  rhi/src/DeviceStub.cpp
  rhi/src/Format.cpp
  rhi/src/ResourceManager.cpp
)
target_include_directories(DriftRHI PUBLIC
  rhi/include
)
target_link_libraries(DriftRHI PUBLIC
  DriftCore
)

# 3.4) DriftRHI_DX11 (implementação DX11)
add_library(DriftRHI_DX11 STATIC
  rhi_dx11/src/DeviceDX11.cpp
  rhi_dx11/src/ContextDX11.cpp
  rhi_dx11/src/BufferDX11.cpp
  rhi_dx11/src/PipelineStateDX11.cpp
  rhi_dx11/src/ShaderDX11.cpp
  rhi_dx11/src/TextureDX11.cpp
  rhi_dx11/src/SamplerDX11.cpp
  rhi_dx11/src/SwapChainDX11.cpp
  rhi_dx11/src/RingBufferDX11.cpp
  rhi_dx11/src/UIBatcherDX11.cpp
  rhi_dx11/src/DepthStencilStateDX11.cpp
)
target_include_directories(DriftRHI_DX11 PUBLIC
  rhi_dx11/include
)
target_link_libraries(DriftRHI_DX11 PUBLIC
  DriftRHI
  d3d11 dxgi D3DCompiler DirectXTK
)

# 3.5) DriftRenderer (passes)
add_library(DriftRenderer STATIC
  renderer/src/IRenderPass.cpp
  renderer/src/TerrainPass.cpp
  renderer/src/RenderManager.cpp
)
target_include_directories(DriftRenderer PUBLIC
  renderer/include
  ${glfw_SOURCE_DIR}/include
)
target_link_libraries(DriftRenderer PUBLIC
  DriftRHI_DX11
  DriftEngine
  DriftMath
  glm
  d3d11 dxgi D3DCompiler DirectXTK
)

# 3.6) DriftUI (sistema de interface)
add_library(DriftUI STATIC
  ui/src/UIContext.cpp
  ui/src/EventBus.cpp
  ui/src/UIElement.cpp
  ui/src/LayoutEngine.cpp
)

target_include_directories(DriftUI PUBLIC
  ui/include
  rhi/include
)

target_link_libraries(DriftUI PUBLIC
  DriftCore
  DriftMath
  glm
)

# Vincular DriftUI ao DriftEngine e DriftRenderer
# (se necessário futura integração)

# ------------------------------------------------
# 4) Executável DriftApp
# ------------------------------------------------
add_executable(DriftApp
  app/main.cpp
)

add_custom_command(TARGET DriftApp PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:DriftApp>/shaders"
)
# Garantir que NormalLineGS.hlsl está incluso
file(GLOB SHADERS "${CMAKE_SOURCE_DIR}/shaders/*.hlsl")
foreach(SHADER ${SHADERS})
  get_filename_component(SHADER_NAME ${SHADER} NAME)
  configure_file(${SHADER} ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()

target_include_directories(DriftApp PRIVATE
  ${glfw_SOURCE_DIR}/include
)
target_link_libraries(DriftApp PRIVATE
  DriftCore
  DriftEngine
  DriftRHI_DX11
  DriftRenderer
  DriftUI
  glfw
)

# ------------------------------------------------
# Instruções:
#   mkdir build && cd build
#   cmake .. -DCMAKE_BUILD_TYPE=Debug
#   cmake --build . --config Debug
# ------------------------------------------------
