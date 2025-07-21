// src/renderer/src/TerrainPass.cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Drift/Renderer/TerrainPass.h"
#include "Drift/RHI/Types.h"
#include "Drift/Core/Log.h"
#include "Drift/Math/Math.h"
#include "Drift/RHI/DX11/RingBufferDX11.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <set> // Para std::set usado no sistema profissional

namespace RHI = Drift::RHI;
using namespace Drift::Renderer;

//-----------------------------------------------------------------------------
// TerrainManager - Sistema Profissional Inspirado em Engines AAA
//-----------------------------------------------------------------------------

TerrainManager::TerrainManager(RHI::IDevice& device,
                               RHI::IContext& context,
                               int tileSize,
                               int visibleRadius)
  : _device(device)
  , _context(context)
  , tileSize(tileSize)
  , visibleRadius(visibleRadius)
{}

void TerrainManager::GenerateTileMesh(TerrainTile& tile) {
    // SOLUÇÃO DEFINITIVA: Vértices nas bordas devem ser MATEMATICAMENTE IDÊNTICOS
    // entre tiles adjacentes para eliminar gaps completamente
    
    const int resolution = 33; // 33x33 vértices (0-32) - número ímpar para centro exato
    const float tileWorldSize = 128.0f;
    
    // Coordenadas world EXATAS do tile
    double tileStartX = static_cast<double>(tile.tileCoord.x) * tileWorldSize;
    double tileStartZ = static_cast<double>(tile.tileCoord.y) * tileWorldSize;
    
    tile.vertices.clear();
    tile.indices.clear();
    tile.vertices.reserve(resolution * resolution);
    tile.indices.reserve((resolution-1) * (resolution-1) * 6);
    
    // Gerar vértices com precisão dupla para eliminar erros de ponto flutuante
    for(int z = 0; z < resolution; ++z) {
        for(int x = 0; x < resolution; ++x) {
            // Usar aritmética de precisão dupla
            double stepX = static_cast<double>(x) / (resolution - 1);
            double stepZ = static_cast<double>(z) / (resolution - 1);
            
            // Posição world com precisão dupla
            double worldX = tileStartX + stepX * tileWorldSize;
            double worldZ = tileStartZ + stepZ * tileWorldSize;
            
            // CRÍTICO: Garantir bordas exatas
            if(x == 0) worldX = tileStartX; // borda esquerda
            if(x == resolution - 1) worldX = tileStartX + tileWorldSize; // borda direita
            if(z == 0) worldZ = tileStartZ; // borda inferior
            if(z == resolution - 1) worldZ = tileStartZ + tileWorldSize; // borda superior
            
            // Converter para float apenas no final
            glm::vec3 position(static_cast<float>(worldX), 0.0f, static_cast<float>(worldZ));
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            
            // UVs baseados na posição world
            float u = static_cast<float>(worldX) * 0.005f; // escala menor para mais detalhes
            float v = static_cast<float>(worldZ) * 0.005f;
            
            tile.vertices.push_back({position, normal, {u, v}});
        }
    }
    
    // Gerar índices
    for(int z = 0; z < resolution - 1; ++z) {
        for(int x = 0; x < resolution - 1; ++x) {
            int topLeft = z * resolution + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * resolution + x;
            int bottomRight = bottomLeft + 1;
            
            // Winding order consistente
            tile.indices.push_back(topLeft);
            tile.indices.push_back(bottomLeft);
            tile.indices.push_back(topRight);
            
            tile.indices.push_back(topRight);
            tile.indices.push_back(bottomLeft);
            tile.indices.push_back(bottomRight);
        }
    }
    
    // Criar buffers GPU
    tile.vb = _device.CreateBuffer({
        RHI::BufferType::Vertex,
        tile.vertices.size() * sizeof(Vertex),
        tile.vertices.data()
    });
    
    tile.ib = _device.CreateBuffer({
        RHI::BufferType::Index,
        tile.indices.size() * sizeof(uint32_t),
        tile.indices.data()
    });
    
    tile.loaded = (tile.vb && tile.ib);
}

void TerrainManager::Update(const glm::vec3& cameraPos) {
    // Sistema profissional: tiles baseados na CÂMERA, não no world origin
    const float tileWorldSize = 128.0f;
    
    // Calcular tile central baseado na posição da câmera (projeção XZ)
    int centerTileX = static_cast<int>(std::floor(cameraPos.x / tileWorldSize));
    int centerTileZ = static_cast<int>(std::floor(cameraPos.z / tileWorldSize));
    
    // DEBUG: Log da posição da câmera e tile central (menos frequente)
    static int debugCounter = 0;
    static glm::ivec2 lastCenterTile(-999, -999);
    glm::ivec2 currentCenterTile(centerTileX, centerTileZ);
    
    // Log apenas quando mudar de tile central
    if(currentCenterTile != lastCenterTile) {
        Drift::Core::Log("[TerrainManager] Câmera em tile: (" + std::to_string(centerTileX) + "," + 
                         std::to_string(centerTileZ) + ") - Tiles antes: " + std::to_string(tiles.size()));
        lastCenterTile = currentCenterTile;
    }
    
    // CORRIGIDO: Raio fixo e menor para evitar carregar muitos tiles
    int loadRadius = 3; // Só carrega 7x7 = 49 tiles máximo
    
    // Primeiro: Remover tiles muito distantes da câmera
    float maxLoadDistance = static_cast<float>(loadRadius) + 1.5f;
    
    auto it = tiles.begin();
    while(it != tiles.end()) {
        glm::vec2 tileCenter = glm::vec2(it->first);
        glm::vec2 cameraCenter = glm::vec2(centerTileX, centerTileZ);
        float distance = glm::distance(tileCenter, cameraCenter);
        
        if(distance > maxLoadDistance) {
            it = tiles.erase(it);
        } else {
            ++it;
        }
    }
    
    // Segundo: Criar apenas tiles próximos à câmera
    for(int dz = -loadRadius; dz <= loadRadius; ++dz) {
        for(int dx = -loadRadius; dx <= loadRadius; ++dx) {
            glm::ivec2 tileCoord(centerTileX + dx, centerTileZ + dz);
            
            if(tiles.find(tileCoord) == tiles.end()) {
                TerrainTile newTile;
                newTile.tileCoord = tileCoord;
                GenerateTileMesh(newTile);
                tiles[tileCoord] = std::move(newTile);
            }
        }
    }
    
    // DEBUG: Log após atualização
    if(currentCenterTile != lastCenterTile) {
        Drift::Core::Log("[TerrainManager] Tiles depois: " + std::to_string(tiles.size()));
    }
}

//-----------------------------------------------------------------------------
// TerrainPass
//-----------------------------------------------------------------------------

TerrainPass::TerrainPass(RHI::IDevice& device,
                         RHI::IContext& context,
                         const std::wstring& texturePath)
  : _device(device)
  , _context(context)
  , _tileManager(std::make_unique<TerrainManager>(device, context, 128, 4)) // Sistema profissional: tiles 128x128, raio 4
{
    // pipeline sólido
    RHI::PipelineDesc pd;
    pd.vsFile = "shaders/TerrainVS.hlsl";
    pd.psFile = "shaders/TerrainPS.hlsl";
    pd.inputLayout = {
      {"POSITION",0,0,"R32G32B32_FLOAT"},
      {"NORMAL",0,12,"R32G32B32_FLOAT"},
      {"TEXCOORD",0,24,"R32G32_FLOAT"}
    };
    pd.rasterizer.cullMode = RHI::PipelineDesc::RasterizerDesc::CullMode::Back;
    pd.rasterizer.wireframe = false;
    _pipeline = _device.CreatePipeline(pd);

    // wireframe
    auto pw = pd; pw.rasterizer.wireframe = true;
    pw.defines.push_back({"WIREFRAME","1"});
    _pipelineWire = _device.CreatePipeline(pw);

    // debug normais
    RHI::PipelineDesc pd2 = pd;
    pd2.gsFile  = "shaders/NormalLineGS.hlsl";
    pd2.gsEntry = "GS";
    pd2.psFile  = "shaders/LinePS.hlsl";
    pd2.rasterizer.cullMode = RHI::PipelineDesc::RasterizerDesc::CullMode::None;
    _pipelineDebug = _device.CreatePipeline(pd2);

    _cb   = _device.CreateBuffer({RHI::BufferType::Constant,sizeof(CBFrame),nullptr});
    _tex  = _device.CreateTexture({texturePath});
    _samp = _device.CreateSampler({});

    _camera.SetFovY(glm::radians(45.0f));
    _camera.SetAspect(1.0f);
    _camera.SetNearFar(0.1f,100000.0f);
    _camera.SetPosition({64, 20, 64}); // CORRIGIDO: posição inicial no centro do tile (0,0) mais próxima do chão
    _camera.SetTarget({64, 0, 128});   // CORRIGIDO: olhando para frente no terreno
}

void TerrainPass::SetAspect(float a) {
    _camera.SetAspect(a);
}

void TerrainPass::Update(float dt, GLFWwindow* window)
{
    // Toggle de captura de mouse com TAB
    bool tabPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tabPressed && !_prevTab) {
        _mouseCaptured = !_mouseCaptured;
        if (_mouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            _firstMouse = true; // reset para evitar jump
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    _prevTab = tabPressed;

    // Só processa mouse se capturado
    if (_mouseCaptured) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (_firstMouse) {
            _lastX = xpos; _lastY = ypos; _firstMouse = false;
        }
        float xoff = float(_lastX - xpos) * 0.1f; // CORRIGIDO: invertido para comportamento FPS padrão
        float yoff = float(_lastY - ypos) * 0.1f;
        _lastX = xpos; _lastY = ypos;
        _yaw   += xoff; _pitch += yoff;
        _pitch = glm::clamp(_pitch, -89.0f, 89.0f);
    }

    glm::vec3 front{
        cos(glm::radians(_yaw)) * cos(glm::radians(_pitch)),
        sin(glm::radians(_pitch)),
        sin(glm::radians(_yaw)) * cos(glm::radians(_pitch))
    };
    front = glm::normalize(front);
    glm::vec3 right = glm::normalize(glm::cross(front, {0,1,0}));
    glm::vec3 up    = glm::normalize(glm::cross(right, front));
    float speed = 100.0f * dt * (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS?3.0f:1.0f);
    glm::vec3 pos = _camera.GetPosition();
    if (glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS) pos += front * speed;
    if (glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS) pos -= front * speed;
    if (glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS) pos += right * speed;
    if (glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS) pos -= right * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE)==GLFW_PRESS) pos += up * speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS) pos -= up * speed;
    _camera.SetPosition(pos);
    _camera.SetTarget(pos + front);

    bool f1 = glfwGetKey(window, GLFW_KEY_F1)==GLFW_PRESS;
    bool f2 = glfwGetKey(window, GLFW_KEY_F2)==GLFW_PRESS;
    if (f1 && !_prevF1) _showWireframe   = !_showWireframe;
    if (f2 && !_prevF2) _showNormalLines = !_showNormalLines;
    _prevF1 = f1;
    _prevF2 = f2;
}

void TerrainPass::Execute() {
    _context.Clear(0.1f,0.1f,0.1f,1.0f);

    // só precisa da posição
    _tileManager->Update(_camera.GetPosition());

    CBFrame cbf{_camera.GetViewProjForHLSL()};
    RHI::UpdateConstantBuffer(_cb.get(), cbf);
    _context.VSSetConstantBuffer(0,_cb->GetBackendHandle());
    _context.GSSetConstantBuffer(0,_cb->GetBackendHandle());

    // FRUSTUM CULLING PROFISSIONAL
    // Calcular frustum da câmera para renderizar apenas tiles visíveis
    glm::mat4 viewProj = _camera.GetViewProjForHLSL();
    
    // DEBUG: Contar tiles carregados vs renderizados
    static int frameCount = 0;
    int tilesLoaded = 0;
    int tilesRendered = 0;

    // desenha cada tile COM FRUSTUM CULLING BASEADO NA CÂMERA
    _tileManager->ForEachTile([&](auto const& coord, TerrainTile& t){
        if(!t.loaded) return;
        tilesLoaded++;
        
        // FRUSTUM CULLING: Baseado na posição e direção da câmera
        const float tileSize = 128.0f;
        glm::vec3 cameraPos = _camera.GetPosition();
        glm::vec3 cameraTarget = _camera.GetTarget();
        glm::vec3 cameraDir = glm::normalize(cameraTarget - cameraPos);
        
        // Centro do tile no world space
        glm::vec3 tileCenter(
            coord.x * tileSize + tileSize * 0.5f,
            0.0f, // terreno plano
            coord.y * tileSize + tileSize * 0.5f
        );
        
        // Vetor da câmera para o tile
        glm::vec3 toTile = tileCenter - cameraPos;
        float distanceToTile = glm::length(toTile);
        
        // Culling por distância: tiles muito distantes
        if(distanceToTile > 400.0f) return; // Máximo 400 unidades (~3 tiles)
        
        // Culling por direção: tiles fora do FOV
        if(distanceToTile > 0.1f) { // Evitar divisão por zero
            glm::vec3 dirToTile = toTile / distanceToTile;
            float dot = glm::dot(cameraDir, dirToTile);
            
            // FOV culling: só renderizar tiles na direção da câmera
            if(dot < 0.2f) return; // ~78 graus de FOV (mais restritivo)
        }
        
        tilesRendered++;
        
        // sólido
        _pipeline->Apply(_context);
        _context.PSSetTexture(0,_tex.get());
        _context.PSSetSampler(0,_samp.get());
        _context.IASetVertexBuffer(t.vb->GetBackendHandle(),sizeof(Vertex),0);
        _context.IASetIndexBuffer (t.ib->GetBackendHandle(),RHI::Format::R32_UINT,0);
        _context.SetDepthTestEnabled(true);
        _context.DrawIndexed(t.indices.size(),0,0);

        // wire
        if(_showWireframe){
          _pipelineWire->Apply(_context);
          _context.SetDepthTestEnabled(false);
          _context.DrawIndexed(t.indices.size(),0,0);
        }

        // normais
        if(_showNormalLines){
          _pipelineDebug->Apply(_context);
          _context.SetDepthTestEnabled(false);
          _context.IASetPrimitiveTopology(RHI::PrimitiveTopology::PointList);
          _context.Draw(static_cast<UINT>(t.vertices.size()),0);
        }
    });
    
    // DEBUG: Log tiles carregados vs renderizados (menos frequente)
    if(frameCount++ % 300 == 0) { // A cada 5 segundos
        Drift::Core::Log("[Render] Carregados: " + std::to_string(tilesLoaded) + 
                         " | Renderizados: " + std::to_string(tilesRendered) + 
                         " | Economia: " + std::to_string(tilesLoaded - tilesRendered));
    }
}