// src/renderer/include/Drift/Renderer/TerrainPass.h
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Drift/Renderer/IRenderPass.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Types.h"
#include "Drift/Math/Camera.h"
#include "Drift/Renderer/CBFrame.h"

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <GLFW/glfw3.h>
#include <map>
#include <memory>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <array>

// operador < para glm::ivec2 em std::map
namespace std {
    template<>
    struct less<glm::ivec2> {
        bool operator()(glm::ivec2 const& a, glm::ivec2 const& b) const {
            return tie(a.x, a.y) < tie(b.x, b.y);
        }
    };
    
    // Hash para glm::ivec2 em unordered_map
    template<>
    struct hash<glm::ivec2> {
        size_t operator()(const glm::ivec2& v) const {
            return hash<int>()(v.x) ^ (hash<int>()(v.y) << 1);
        }
    };
}

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

namespace Drift::Renderer {

    // AAA Industry Standard: LOD System
    enum class TerrainLOD : uint8_t {
        LOD0 = 0, // Highest detail - 65x65 vertices
        LOD1 = 1, // High detail - 33x33 vertices  
        LOD2 = 2, // Medium detail - 17x17 vertices
        LOD3 = 3, // Low detail - 9x9 vertices
        COUNT
    };

    // AAA Industry Standard: Tile State Management
    enum class TileState : uint8_t {
        Unloaded,
        Loading,
        Loaded,
        Rendering,
        Unloading
    };

    // AAA Industry Standard: Frustum Plane Structure
    struct FrustumPlane {
        glm::vec3 normal;
        float distance;
        
        float DistanceToPoint(const glm::vec3& point) const {
            return glm::dot(normal, point) + distance;
        }
    };

    // AAA Industry Standard: View Frustum
    struct ViewFrustum {
        std::array<FrustumPlane, 6> planes; // left, right, bottom, top, near, far
        
        bool IsBoxInFrustum(const glm::vec3& center, const glm::vec3& halfExtents) const;
        void ExtractFromMatrix(const glm::mat4& viewProjMatrix);
    };

    // AAA Industry Standard: Enhanced Terrain Tile
    struct TerrainTile {
        glm::ivec2 tileCoord;
        TerrainLOD currentLOD;
        TileState state;
        
        // Multi-LOD vertex data
        std::array<std::vector<Vertex>, static_cast<size_t>(TerrainLOD::COUNT)> lodVertices;
        std::array<std::vector<uint32_t>, static_cast<size_t>(TerrainLOD::COUNT)> lodIndices;
        
        // GPU buffers for each LOD
        std::array<std::shared_ptr<Drift::RHI::IBuffer>, static_cast<size_t>(TerrainLOD::COUNT)> lodVertexBuffers;
        std::array<std::shared_ptr<Drift::RHI::IBuffer>, static_cast<size_t>(TerrainLOD::COUNT)> lodIndexBuffers;
        
        // AAA Industry Standard: Tile bounds for frustum culling
        glm::vec3 boundingBoxMin;
        glm::vec3 boundingBoxMax;
        glm::vec3 boundingBoxCenter;
        glm::vec3 boundingBoxHalfExtents;
        
        // Distance from camera for LOD selection
        float distanceFromCamera = 0.0f;
        
        // Seamless stitching data
        bool needsStitching = false;
        std::array<TerrainLOD, 4> neighborLODs; // North, East, South, West
        
        bool IsLoaded() const { return state == TileState::Loaded || state == TileState::Rendering; }
        void UpdateBoundingBox(float tileSize);
        TerrainLOD SelectLOD(float cameraDistance) const;
    };

    // AAA Industry Standard: Shared Border Vertex Cache
    class BorderVertexCache {
    public:
        struct BorderKey {
            glm::ivec2 tileCoord;
            uint8_t edge; // 0=North, 1=East, 2=South, 3=West
            uint32_t vertexIndex;
            
            bool operator==(const BorderKey& other) const {
                return tileCoord.x == other.tileCoord.x && 
                       tileCoord.y == other.tileCoord.y && 
                       edge == other.edge && 
                       vertexIndex == other.vertexIndex;
            }
        };
        
        struct BorderKeyHash {
            size_t operator()(const BorderKey& key) const {
                return std::hash<int>()(key.tileCoord.x) ^ 
                       (std::hash<int>()(key.tileCoord.y) << 1) ^
                       (std::hash<uint8_t>()(key.edge) << 2) ^
                       (std::hash<uint32_t>()(key.vertexIndex) << 3);
            }
        };
        
        void CacheVertex(const BorderKey& key, const Vertex& vertex);
        bool GetCachedVertex(const BorderKey& key, Vertex& outVertex) const;
        void Clear();
        
    private:
        std::unordered_map<BorderKey, Vertex, BorderKeyHash> cache;
    };

    // AAA Industry Standard: Enhanced Terrain Manager
    class TerrainManager {
    public:
        TerrainManager(Drift::RHI::IDevice& device,
                       Drift::RHI::IContext& context,
                       int tileSize = 128,
                       int visibleRadius = 5);

        void Update(const glm::vec3& cameraPos, const ViewFrustum& frustum);
        
        template<typename F> 
        void ForEachVisibleTile(F&& f) {
            for (auto& kv : tiles) {
                if (kv.second.IsLoaded() && IsVisibleInFrustum(kv.second)) {
                    f(kv.first, kv.second);
                }
            }
        }

        // AAA Industry Standard: Performance metrics
        struct PerformanceStats {
            uint32_t tilesLoaded = 0;
            uint32_t tilesRendered = 0;
            uint32_t tilesLOD0 = 0, tilesLOD1 = 0, tilesLOD2 = 0, tilesLOD3 = 0;
            uint32_t verticesRendered = 0;
            uint32_t trianglesRendered = 0;
        };
        
        const PerformanceStats& GetStats() const { return stats; }

    private:
        void GenerateTileMesh(TerrainTile& tile);
        void GenerateLODMesh(TerrainTile& tile, TerrainLOD lod);
        void UnloadFarTiles(const glm::ivec2& camTile);
        void UpdateTileLODs(const glm::vec3& cameraPos);
        void StitchTileBorders(TerrainTile& tile);
        bool IsVisibleInFrustum(const TerrainTile& tile) const;
        
        // AAA Industry Standard: Precise vertex generation
        Vertex GenerateVertex(double worldX, double worldZ, TerrainLOD lod) const;
        uint32_t GetLODResolution(TerrainLOD lod) const;
        
        std::unordered_map<glm::ivec2, TerrainTile> tiles;
        BorderVertexCache vertexCache;
        ViewFrustum currentFrustum;
        
        int tileSize, visibleRadius;
        float tileWorldSize;
        
        Drift::RHI::IDevice& _device;
        Drift::RHI::IContext& _context;
        
        PerformanceStats stats;
    };

    class TerrainPass : public IRenderPass {
    public:
        TerrainPass(Drift::RHI::IDevice& device,
                    Drift::RHI::IContext& context,
                    const std::wstring& texturePath);
        ~TerrainPass() override = default;

        void Execute() override;
        void SetAspect(float aspect) override;
        void Update(float dt, GLFWwindow* window);

    private:
        std::unique_ptr<TerrainManager>         _tileManager;
        Drift::RHI::IDevice&                   _device;
        Drift::RHI::IContext&                  _context;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipeline;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineWire;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineLOD;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineDebug;
        std::shared_ptr<Drift::RHI::IBuffer>         _cb;
        std::shared_ptr<Drift::RHI::ITexture>        _tex;
        std::shared_ptr<Drift::RHI::ISampler>        _samp;
        Drift::Math::Camera                         _camera;

        // estado de input
        double _lastX = 0.0, _lastY = 0.0;
        bool   _firstMouse = true;
        float  _yaw = -90.0f, _pitch = 0.0f;
        bool   _showWireframe = false, _showNormalLines = false;
        bool   _prevF1 = false, _prevF2 = false;
        bool   _mouseCaptured = false, _prevTab = false;
        
        // AAA Industry Standard: Debug visualization
        bool   _showLODColors = false, _prevF3 = false;
        bool   _showStats = false, _prevF4 = false;
    };

} // namespace Drift::Renderer
