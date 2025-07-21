// src/renderer/include/Drift/Renderer/TerrainPass.h
#pragma once

#include "Drift/Renderer/IRenderPass.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Types.h"
#include "Drift/Math/Camera.h"

#include <glm/glm.hpp>
#include <memory>
#include <cstdint>
#include "Drift/Renderer/CBFrame.h"
#include <GLFW/glfw3.h>

namespace Drift::Renderer {

    class TerrainPass : public IRenderPass {
    public:
        TerrainPass(Drift::RHI::IDevice& device,
            Drift::RHI::IContext& context,
            const std::wstring& texturePath,
            int rows = 100,
            int cols = 100,
            float uvScale = 1.0f,
            bool sphere = false);
        ~TerrainPass() override = default;

        void Execute() override;
        void SetAspect(float aspect) override;
        void Update(float dt, GLFWwindow* window);
        Drift::Math::Camera& GetCamera() { return _camera; }

    private:
        void BuildGrid(int rows, int cols);

        struct Vertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
        };

        Drift::RHI::IDevice& _device;
        Drift::RHI::IContext& _context;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipeline;
        std::shared_ptr<Drift::RHI::IBuffer>        _vb;
        std::shared_ptr<Drift::RHI::IBuffer>        _ib;
        std::shared_ptr<Drift::RHI::IBuffer>        _cb;
        std::shared_ptr<Drift::RHI::ITexture>       _tex;
        std::shared_ptr<Drift::RHI::ISampler>       _samp;
        Drift::Math::Camera                         _camera;
        uint32_t                                    _indexCount;
        Drift::RHI::Format                          _indexFormat;
        // Controle de câmera
        double _lastX = 0.0, _lastY = 0.0;
        float _yaw = -90.0f;
        float _pitch = 0.0f;
        bool _firstMouse = true;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineWireframe;
        bool _showWireframe = false;
        bool _showNormalLines = false;
        float _uvScale = 1.0f;
        bool _sphere = false;
        std::vector<Vertex> _verts;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineLineTest;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineNormals;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineWireframeNormals;
        bool _showNormals = false;
        bool _prevF1 = false;
        bool _prevF2 = false;
        std::shared_ptr<Drift::RHI::IRingBuffer> _ringBuffer;
    };

} // namespace Drift::Renderer
