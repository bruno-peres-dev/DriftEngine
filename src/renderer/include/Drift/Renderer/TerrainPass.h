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

// operador < para glm::ivec2 em std::map
namespace std {
    template<>
    struct less<glm::ivec2> {
        bool operator()(glm::ivec2 const& a, glm::ivec2 const& b) const {
            return tie(a.x, a.y) < tie(b.x, b.y);
        }
    };
}

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

namespace Drift::Renderer {

    struct TerrainTile {
        glm::ivec2 tileCoord;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::shared_ptr<Drift::RHI::IBuffer> vb;
        std::shared_ptr<Drift::RHI::IBuffer> ib;
        bool loaded = false;
    };

    class TerrainManager {
    public:
        TerrainManager(Drift::RHI::IDevice& device,
                       Drift::RHI::IContext& context,
                       int tileSize = 128,
                       int visibleRadius = 5);

        void Update(const glm::vec3& cameraPos);
        template<typename F> void ForEachTile(F&& f) {
            for (auto& kv : tiles) f(kv.first, kv.second);
        }

    private:
        void GenerateTileMesh(TerrainTile& tile);
        void UnloadFarTiles(const glm::ivec2& camTile);

        std::map<glm::ivec2, TerrainTile> tiles;
        int tileSize, visibleRadius;
        Drift::RHI::IDevice& _device;
        Drift::RHI::IContext& _context;
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
    };

} // namespace Drift::Renderer
