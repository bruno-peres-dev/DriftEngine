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
#include <set>
#include <algorithm>

namespace RHI = Drift::RHI;
using namespace Drift::Renderer;

//-----------------------------------------------------------------------------
// AAA Industry Standard: View Frustum Implementation
//-----------------------------------------------------------------------------

bool ViewFrustum::IsBoxInFrustum(const glm::vec3& center, const glm::vec3& halfExtents) const {
    for (const auto& plane : planes) {
        // Get the positive vertex (farthest from plane)
        glm::vec3 positiveVertex = center;
        if (plane.normal.x >= 0) positiveVertex.x += halfExtents.x;
        else positiveVertex.x -= halfExtents.x;
        if (plane.normal.y >= 0) positiveVertex.y += halfExtents.y;
        else positiveVertex.y -= halfExtents.y;
        if (plane.normal.z >= 0) positiveVertex.z += halfExtents.z;
        else positiveVertex.z -= halfExtents.z;
        
        // If positive vertex is outside plane, box is completely outside
        if (plane.DistanceToPoint(positiveVertex) < 0) {
            return false;
        }
    }
    return true;
}

void ViewFrustum::ExtractFromMatrix(const glm::mat4& viewProjMatrix) {
    // Extract frustum planes from view-projection matrix
    // Left plane
    planes[0].normal = glm::vec3(viewProjMatrix[0][3] + viewProjMatrix[0][0],
                                viewProjMatrix[1][3] + viewProjMatrix[1][0],
                                viewProjMatrix[2][3] + viewProjMatrix[2][0]);
    planes[0].distance = viewProjMatrix[3][3] + viewProjMatrix[3][0];
    
    // Right plane
    planes[1].normal = glm::vec3(viewProjMatrix[0][3] - viewProjMatrix[0][0],
                                viewProjMatrix[1][3] - viewProjMatrix[1][0],
                                viewProjMatrix[2][3] - viewProjMatrix[2][0]);
    planes[1].distance = viewProjMatrix[3][3] - viewProjMatrix[3][0];
    
    // Bottom plane
    planes[2].normal = glm::vec3(viewProjMatrix[0][3] + viewProjMatrix[0][1],
                                viewProjMatrix[1][3] + viewProjMatrix[1][1],
                                viewProjMatrix[2][3] + viewProjMatrix[2][1]);
    planes[2].distance = viewProjMatrix[3][3] + viewProjMatrix[3][1];
    
    // Top plane
    planes[3].normal = glm::vec3(viewProjMatrix[0][3] - viewProjMatrix[0][1],
                                viewProjMatrix[1][3] - viewProjMatrix[1][1],
                                viewProjMatrix[2][3] - viewProjMatrix[2][1]);
    planes[3].distance = viewProjMatrix[3][3] - viewProjMatrix[3][1];
    
    // Near plane
    planes[4].normal = glm::vec3(viewProjMatrix[0][3] + viewProjMatrix[0][2],
                                viewProjMatrix[1][3] + viewProjMatrix[1][2],
                                viewProjMatrix[2][3] + viewProjMatrix[2][2]);
    planes[4].distance = viewProjMatrix[3][3] + viewProjMatrix[3][2];
    
    // Far plane
    planes[5].normal = glm::vec3(viewProjMatrix[0][3] - viewProjMatrix[0][2],
                                viewProjMatrix[1][3] - viewProjMatrix[1][2],
                                viewProjMatrix[2][3] - viewProjMatrix[2][2]);
    planes[5].distance = viewProjMatrix[3][3] - viewProjMatrix[3][2];
    
    // Normalize planes
    for (auto& plane : planes) {
        float length = glm::length(plane.normal);
        if (length > 0.0f) {
            plane.normal /= length;
            plane.distance /= length;
        }
    }
}

//-----------------------------------------------------------------------------
// AAA Industry Standard: Terrain Tile Implementation
//-----------------------------------------------------------------------------

void TerrainTile::UpdateBoundingBox(float tileSize) {
    float worldX = static_cast<float>(tileCoord.x) * tileSize;
    float worldZ = static_cast<float>(tileCoord.y) * tileSize;
    
    boundingBoxMin = glm::vec3(worldX, -10.0f, worldZ);
    boundingBoxMax = glm::vec3(worldX + tileSize, 10.0f, worldZ + tileSize);
    boundingBoxCenter = (boundingBoxMin + boundingBoxMax) * 0.5f;
    boundingBoxHalfExtents = (boundingBoxMax - boundingBoxMin) * 0.5f;
}

TerrainLOD TerrainTile::SelectLOD(float cameraDistance) const {
    // AAA Industry Standard: Distance-based LOD selection
    if (cameraDistance < 150.0f) return TerrainLOD::LOD0;      // High detail
    else if (cameraDistance < 300.0f) return TerrainLOD::LOD1; // Medium-high detail
    else if (cameraDistance < 600.0f) return TerrainLOD::LOD2; // Medium detail
    else return TerrainLOD::LOD3;                               // Low detail
}

//-----------------------------------------------------------------------------
// AAA Industry Standard: Border Vertex Cache Implementation
//-----------------------------------------------------------------------------

void BorderVertexCache::CacheVertex(const BorderKey& key, const Vertex& vertex) {
    cache[key] = vertex;
}

bool BorderVertexCache::GetCachedVertex(const BorderKey& key, Vertex& outVertex) const {
    auto it = cache.find(key);
    if (it != cache.end()) {
        outVertex = it->second;
        return true;
    }
    return false;
}

void BorderVertexCache::Clear() {
    cache.clear();
}

//-----------------------------------------------------------------------------
// AAA Industry Standard: Enhanced Terrain Manager
//-----------------------------------------------------------------------------

TerrainManager::TerrainManager(RHI::IDevice& device,
                               RHI::IContext& context,
                               int tileSize,
                               int visibleRadius)
  : _device(device)
  , _context(context)
  , tileSize(tileSize)
  , tileWorldSize(static_cast<float>(tileSize))
  , visibleRadius(visibleRadius)
{}

uint32_t TerrainManager::GetLODResolution(TerrainLOD lod) const {
    switch (lod) {
        case TerrainLOD::LOD0: return 65; // Highest detail
        case TerrainLOD::LOD1: return 33; // High detail
        case TerrainLOD::LOD2: return 17; // Medium detail
        case TerrainLOD::LOD3: return 9;  // Low detail
        default: return 33;
    }
}

Vertex TerrainManager::GenerateVertex(double worldX, double worldZ, TerrainLOD lod) const {
    // AAA Industry Standard: Use double precision for vertex generation to eliminate gaps
    glm::vec3 position(static_cast<float>(worldX), 0.0f, static_cast<float>(worldZ));
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    
    // Scale UV coordinates based on LOD for consistent texturing
    float uvScale = 0.005f;
    switch (lod) {
        case TerrainLOD::LOD0: uvScale = 0.008f; break;
        case TerrainLOD::LOD1: uvScale = 0.006f; break;
        case TerrainLOD::LOD2: uvScale = 0.005f; break;
        case TerrainLOD::LOD3: uvScale = 0.004f; break;
    }
    
    float u = static_cast<float>(worldX) * uvScale;
    float v = static_cast<float>(worldZ) * uvScale;
    
    return {position, normal, {u, v}};
}

void TerrainManager::GenerateLODMesh(TerrainTile& tile, TerrainLOD lod) {
    const uint32_t resolution = GetLODResolution(lod);
    const size_t lodIndex = static_cast<size_t>(lod);
    
    // Clear existing data
    tile.lodVertices[lodIndex].clear();
    tile.lodIndices[lodIndex].clear();
    tile.lodVertices[lodIndex].reserve(resolution * resolution);
    tile.lodIndices[lodIndex].reserve((resolution-1) * (resolution-1) * 6);
    
    // AAA Industry Standard: Use double precision for tile boundaries
    double tileStartX = static_cast<double>(tile.tileCoord.x) * tileWorldSize;
    double tileStartZ = static_cast<double>(tile.tileCoord.y) * tileWorldSize;
    
    // Generate vertices with precise border alignment
    for(uint32_t z = 0; z < resolution; ++z) {
        for(uint32_t x = 0; x < resolution; ++x) {
            // Use double precision for step calculations
            double stepX = static_cast<double>(x) / (resolution - 1);
            double stepZ = static_cast<double>(z) / (resolution - 1);
            
            // Calculate world position with double precision
            double worldX = tileStartX + stepX * tileWorldSize;
            double worldZ = tileStartZ + stepZ * tileWorldSize;
            
            // AAA Industry Standard: Enforce exact border coordinates
            if(x == 0) worldX = tileStartX;
            if(x == resolution - 1) worldX = tileStartX + tileWorldSize;
            if(z == 0) worldZ = tileStartZ;
            if(z == resolution - 1) worldZ = tileStartZ + tileWorldSize;
            
            // Check for cached border vertices first
            bool isBorderVertex = (x == 0 || x == resolution-1 || z == 0 || z == resolution-1);
            Vertex vertex;
            bool useCached = false;
            
            if (isBorderVertex) {
                BorderVertexCache::BorderKey key;
                key.tileCoord = tile.tileCoord;
                key.vertexIndex = z * resolution + x;
                
                // Determine edge
                if (z == 0) key.edge = 2; // South
                else if (z == resolution-1) key.edge = 0; // North
                else if (x == 0) key.edge = 3; // West
                else if (x == resolution-1) key.edge = 1; // East
                
                useCached = vertexCache.GetCachedVertex(key, vertex);
            }
            
            if (!useCached) {
                vertex = GenerateVertex(worldX, worldZ, lod);
                
                // Cache border vertices for adjacent tiles
                if (isBorderVertex) {
                    BorderVertexCache::BorderKey key;
                    key.tileCoord = tile.tileCoord;
                    key.vertexIndex = z * resolution + x;
                    
                    if (z == 0) key.edge = 2; // South
                    else if (z == resolution-1) key.edge = 0; // North
                    else if (x == 0) key.edge = 3; // West
                    else if (x == resolution-1) key.edge = 1; // East
                    
                    vertexCache.CacheVertex(key, vertex);
                }
            }
            
            tile.lodVertices[lodIndex].push_back(vertex);
        }
    }
    
    // Generate indices with consistent winding order
    for(uint32_t z = 0; z < resolution - 1; ++z) {
        for(uint32_t x = 0; x < resolution - 1; ++x) {
            uint32_t topLeft = z * resolution + x;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (z + 1) * resolution + x;
            uint32_t bottomRight = bottomLeft + 1;
            
            // Consistent triangle winding
            tile.lodIndices[lodIndex].push_back(topLeft);
            tile.lodIndices[lodIndex].push_back(bottomLeft);
            tile.lodIndices[lodIndex].push_back(topRight);
            
            tile.lodIndices[lodIndex].push_back(topRight);
            tile.lodIndices[lodIndex].push_back(bottomLeft);
            tile.lodIndices[lodIndex].push_back(bottomRight);
        }
    }
    
    // Create GPU buffers
    tile.lodVertexBuffers[lodIndex] = _device.CreateBuffer({
        RHI::BufferType::Vertex,
        tile.lodVertices[lodIndex].size() * sizeof(Vertex),
        tile.lodVertices[lodIndex].data()
    });
    
    tile.lodIndexBuffers[lodIndex] = _device.CreateBuffer({
        RHI::BufferType::Index,
        tile.lodIndices[lodIndex].size() * sizeof(uint32_t),
        tile.lodIndices[lodIndex].data()
    });
}

void TerrainManager::GenerateTileMesh(TerrainTile& tile) {
    tile.state = TileState::Loading;
    
    // Update bounding box for frustum culling
    tile.UpdateBoundingBox(tileWorldSize);
    
    // Generate all LOD levels
    for (int i = 0; i < static_cast<int>(TerrainLOD::COUNT); ++i) {
        GenerateLODMesh(tile, static_cast<TerrainLOD>(i));
    }
    
    // Set initial LOD
    tile.currentLOD = TerrainLOD::LOD1;
    tile.state = TileState::Loaded;
}

bool TerrainManager::IsVisibleInFrustum(const TerrainTile& tile) const {
    return currentFrustum.IsBoxInFrustum(tile.boundingBoxCenter, tile.boundingBoxHalfExtents);
}

void TerrainManager::UpdateTileLODs(const glm::vec3& cameraPos) {
    for (auto& [coord, tile] : tiles) {
        if (tile.IsLoaded()) {
            // Calculate distance from camera to tile center
            tile.distanceFromCamera = glm::distance(cameraPos, tile.boundingBoxCenter);
            
            // Select appropriate LOD
            TerrainLOD newLOD = tile.SelectLOD(tile.distanceFromCamera);
            if (newLOD != tile.currentLOD) {
                tile.currentLOD = newLOD;
                tile.needsStitching = true;
            }
        }
    }
}

void TerrainManager::Update(const glm::vec3& cameraPos, const ViewFrustum& frustum) {
    currentFrustum = frustum;
    
    // Calculate camera tile coordinate
    int centerTileX = static_cast<int>(std::floor(cameraPos.x / tileWorldSize));
    int centerTileZ = static_cast<int>(std::floor(cameraPos.z / tileWorldSize));
    
    // Clear performance stats
    stats = {};
    
    // Remove tiles outside visible radius
    float maxLoadDistance = static_cast<float>(visibleRadius) + 1.5f;
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
    
    // Load tiles within visible radius
    int loadRadius = std::min(visibleRadius, 4); // Cap at 4 for performance
    for(int dz = -loadRadius; dz <= loadRadius; ++dz) {
        for(int dx = -loadRadius; dx <= loadRadius; ++dx) {
            glm::ivec2 tileCoord(centerTileX + dx, centerTileZ + dz);
            
            if(tiles.find(tileCoord) == tiles.end()) {
                TerrainTile newTile;
                newTile.tileCoord = tileCoord;
                newTile.state = TileState::Unloaded;
                GenerateTileMesh(newTile);
                tiles[tileCoord] = std::move(newTile);
            }
        }
    }
    
    // Update LODs based on camera distance
    UpdateTileLODs(cameraPos);
    
    // Update performance statistics
    for (const auto& [coord, tile] : tiles) {
        if (tile.IsLoaded()) {
            stats.tilesLoaded++;
            if (IsVisibleInFrustum(tile)) {
                stats.tilesRendered++;
                switch (tile.currentLOD) {
                    case TerrainLOD::LOD0: stats.tilesLOD0++; break;
                    case TerrainLOD::LOD1: stats.tilesLOD1++; break;
                    case TerrainLOD::LOD2: stats.tilesLOD2++; break;
                    case TerrainLOD::LOD3: stats.tilesLOD3++; break;
                }
                
                size_t lodIndex = static_cast<size_t>(tile.currentLOD);
                stats.verticesRendered += static_cast<uint32_t>(tile.lodVertices[lodIndex].size());
                stats.trianglesRendered += static_cast<uint32_t>(tile.lodIndices[lodIndex].size() / 3);
            }
        }
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
  , _tileManager(std::make_unique<TerrainManager>(device, context, 128, 4))
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

    // LOD color visualization pipeline
    auto plod = pd;
    plod.defines.push_back({"LOD_COLORS","1"});
    _pipelineLOD = _device.CreatePipeline(plod);

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
    _camera.SetPosition({64, 20, 64});
    _camera.SetTarget({64, 0, 128});
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
            _firstMouse = true;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    _prevTab = tabPressed;

    // Mouse look
    if (_mouseCaptured) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (_firstMouse) {
            _lastX = xpos; _lastY = ypos; _firstMouse = false;
        }
        float xoff = float(_lastX - xpos) * 0.1f;
        float yoff = float(_lastY - ypos) * 0.1f;
        _lastX = xpos; _lastY = ypos;
        _yaw   += xoff; _pitch += yoff;
        _pitch = glm::clamp(_pitch, -89.0f, 89.0f);
    }

    // Movement
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

    // Debug toggles
    bool f1 = glfwGetKey(window, GLFW_KEY_F1)==GLFW_PRESS;
    bool f2 = glfwGetKey(window, GLFW_KEY_F2)==GLFW_PRESS;
    bool f3 = glfwGetKey(window, GLFW_KEY_F3)==GLFW_PRESS;
    bool f4 = glfwGetKey(window, GLFW_KEY_F4)==GLFW_PRESS;
    
    if (f1 && !_prevF1) _showWireframe   = !_showWireframe;
    if (f2 && !_prevF2) _showNormalLines = !_showNormalLines;
    if (f3 && !_prevF3) _showLODColors   = !_showLODColors;
    if (f4 && !_prevF4) _showStats       = !_showStats;
    
    _prevF1 = f1; _prevF2 = f2; _prevF3 = f3; _prevF4 = f4;
}

void TerrainPass::Execute() {
    _context.Clear(0.1f,0.1f,0.1f,1.0f);

    // Extract frustum from camera
    ViewFrustum frustum;
    frustum.ExtractFromMatrix(_camera.GetViewProjForHLSL());
    
    // Update terrain manager with camera position and frustum
    _tileManager->Update(_camera.GetPosition(), frustum);

    CBFrame cbf{_camera.GetViewProjForHLSL()};
    RHI::UpdateConstantBuffer(_cb.get(), cbf);
    _context.VSSetConstantBuffer(0,_cb->GetBackendHandle());
    _context.GSSetConstantBuffer(0,_cb->GetBackendHandle());

    // Render visible tiles with appropriate LOD
    _tileManager->ForEachVisibleTile([&](auto const& coord, TerrainTile& tile){
        if(!tile.IsLoaded()) return;
        
        size_t lodIndex = static_cast<size_t>(tile.currentLOD);
        
        // Check if LOD buffers exist
        if (!tile.lodVertexBuffers[lodIndex] || !tile.lodIndexBuffers[lodIndex]) return;
        
                 // Solid rendering with LOD colors if enabled
         if (_showLODColors) {
             _pipelineLOD->Apply(_context);
         } else {
             _pipeline->Apply(_context);
         }
         _context.PSSetTexture(0,_tex.get());
         _context.PSSetSampler(0,_samp.get());
         _context.IASetVertexBuffer(tile.lodVertexBuffers[lodIndex]->GetBackendHandle(),sizeof(Vertex),0);
         _context.IASetIndexBuffer(tile.lodIndexBuffers[lodIndex]->GetBackendHandle(),RHI::Format::R32_UINT,0);
         _context.SetDepthTestEnabled(true);
         _context.DrawIndexed(static_cast<UINT>(tile.lodIndices[lodIndex].size()),0,0);

        // Wireframe overlay
        if(_showWireframe){
          _pipelineWire->Apply(_context);
          _context.SetDepthTestEnabled(false);
          _context.DrawIndexed(static_cast<UINT>(tile.lodIndices[lodIndex].size()),0,0);
        }

        // Normal lines debug
        if(_showNormalLines){
          _pipelineDebug->Apply(_context);
          _context.SetDepthTestEnabled(false);
          _context.IASetPrimitiveTopology(RHI::PrimitiveTopology::PointList);
          _context.Draw(static_cast<UINT>(tile.lodVertices[lodIndex].size()),0);
        }
    });
    
    // Log performance stats periodically
    static int frameCount = 0;
    if (frameCount++ % 300 == 0 && _showStats) {
        const auto& stats = _tileManager->GetStats();
        Drift::Core::Log("[Terrain Stats] Loaded: " + std::to_string(stats.tilesLoaded) + 
                         " | Rendered: " + std::to_string(stats.tilesRendered) + 
                         " | LOD0: " + std::to_string(stats.tilesLOD0) +
                         " | LOD1: " + std::to_string(stats.tilesLOD1) +
                         " | LOD2: " + std::to_string(stats.tilesLOD2) +
                         " | LOD3: " + std::to_string(stats.tilesLOD3) +
                         " | Triangles: " + std::to_string(stats.trianglesRendered));
    }
}