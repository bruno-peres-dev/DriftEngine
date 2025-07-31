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
# 2) GLI (header-only) via subdirectory
# ------------------------------------------------

add_subdirectory(../extern/gli "extern/gli-build")

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
  engine/src/EventBus.cpp
)
target_include_directories(DriftEngine PUBLIC
  engine/include
  rhi/include
  renderer/include
  ui/include
  ${glfw_SOURCE_DIR}/include
)
target_link_libraries(DriftEngine PUBLIC
  DriftCore
  DriftMath
  glm
  glfw
)

# 3.2) DriftCore (logging + assets + threading)
add_library(DriftCore STATIC
  core/src/Log.cpp
  core/src/Profiler.cpp
  core/src/Assets/AssetsSystem.cpp
  core/src/Assets/AssetsExample.cpp
  core/src/Threading/ThreadingSystem.cpp
  core/src/Threading/ThreadingExample.cpp
)
target_include_directories(DriftCore PUBLIC
  core/include
  rhi/include
  ui/include
  ${glm_SOURCE_DIR}
)
target_link_libraries(DriftCore PUBLIC
  glm
)

# 3.3) DriftRHI (interfaces)
add_library(DriftRHI STATIC
  rhi/src/DeviceStub.cpp
  rhi/src/Format.cpp
  rhi/src/ResourceManager.cpp
  rhi/src/Scissor.cpp
  rhi/src/UIBatcherBase.cpp
)
target_include_directories(DriftRHI PUBLIC
  rhi/include
  core/include
)
target_link_libraries(DriftRHI PUBLIC
  DriftCore
  glm
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
  ui/include
  ${CMAKE_SOURCE_DIR}/extern/gli
  ${CMAKE_SOURCE_DIR}/extern/stb
)
target_link_libraries(DriftRHI_DX11 PUBLIC
  DriftRHI
  d3d11 dxgi D3DCompiler
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
  d3d11 dxgi D3DCompiler
)

# 3.6) DriftUI (sistema de interface)
add_library(DriftUI STATIC
  ui/src/UIContext.cpp
  ui/src/UIElement.cpp
  ui/src/LayoutEngine.cpp
  ui/src/UIInputHandler.cpp
  ui/src/FontSystem/FontSystem.cpp
  ui/src/FontSystem/Font.cpp
  ui/src/FontSystem/FontManager.cpp
  ui/src/FontSystem/FontAtlas.cpp
  ui/src/FontSystem/FontMetrics.cpp
  #ui/src/FontSystem/FontRendering.cpp
  ui/src/FontSystem/FontSystemExample.cpp
  ui/src/DataDriven/UIComponentRegistry.cpp
  ui/src/Widgets/Button.cpp
  ui/src/Widgets/Label.cpp
  ui/src/Widgets/Panel.cpp
  ui/src/Widgets/Image.cpp
  ui/src/Widgets/StackPanel.cpp
  ui/src/Widgets/Grid.cpp
)

# Corrigir problema de PDB no Visual Studio
if(MSVC)
    target_compile_options(DriftUI PRIVATE /FS)
endif()

target_include_directories(DriftUI PUBLIC
  ui/include
  rhi/include
  engine/include
  core/include
  ${CMAKE_SOURCE_DIR}/extern/stb
)

# Definir STB_IMAGE_IMPLEMENTATION para compilar stb_image
target_compile_definitions(DriftUI PRIVATE STB_IMAGE_IMPLEMENTATION)
target_compile_definitions(DriftUI PRIVATE STB_TRUETYPE_IMPLEMENTATION)

target_link_libraries(DriftUI PUBLIC
  DriftCore
  DriftMath
  DriftEngine
  glm
)

# ------------------------------------------------
# 5) Executável Principal
# ------------------------------------------------

add_executable(DriftEngineApp
  app/main.cpp
)

target_include_directories(DriftEngineApp PUBLIC
  core/include
  engine/include
  ui/include
  rhi/include
  rhi_dx11/include
  renderer/include
)

target_link_libraries(DriftEngineApp PUBLIC
  DriftCore
  DriftMath
  DriftEngine
  DriftRHI
  DriftRHI_DX11
  DriftRenderer
  DriftUI
  glfw
  d3d11 dxgi D3DCompiler
)



# ------------------------------------------------
# 6) Copiar recursos necessários para o executável
# ------------------------------------------------

# Copiar pasta Fonts
add_custom_command(TARGET DriftEngineApp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/fonts
  $<TARGET_FILE_DIR:DriftEngineApp>/fonts
  COMMENT "Copiando pasta Fonts..."
)

# Copiar pasta Textures
add_custom_command(TARGET DriftEngineApp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/textures
  $<TARGET_FILE_DIR:DriftEngineApp>/textures
  COMMENT "Copiando pasta Textures..."
)

# Copiar pasta Shaders
add_custom_command(TARGET DriftEngineApp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/shaders
  $<TARGET_FILE_DIR:DriftEngineApp>/shaders
  COMMENT "Copiando pasta Shaders..."
)

# ------------------------------------------------
# Instruções:
#   mkdir build && cd build
#   cmake .. -DCMAKE_BUILD_TYPE=Debug
#   cmake --build . --config Debug
# ------------------------------------------------
