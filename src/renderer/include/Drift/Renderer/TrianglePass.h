// src/renderer/include/Drift/Renderer/TrianglePass.h
#pragma once

#include "Drift/Renderer/IRenderPass.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/PipelineState.h"
#include <memory>
#include <glm/glm.hpp>
#include "Drift/Math/Camera.h"
#include "Drift/Renderer/CBFrame.h"
#include <GLFW/glfw3.h>

namespace Drift::Renderer {

    /// Desenha um tringulo com cmera
    class TrianglePass : public IRenderPass {
    public:
        TrianglePass(Drift::RHI::IDevice& device,
            Drift::RHI::IContext& context);
        ~TrianglePass() override = default;

        void Execute() override;
        void SetAspect(float aspect) { _camera.SetAspect(aspect); }
        void Update(float dt, GLFWwindow* window);
        Drift::Math::Camera& GetCamera() { return _camera; }

    private:
        Drift::RHI::IDevice& _device;
        Drift::RHI::IContext& _context;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipeline;
        std::shared_ptr<Drift::RHI::IBuffer>        _vb;
        std::shared_ptr<Drift::RHI::IBuffer>        _cb;      // constant buffer
        Drift::Math::Camera _camera;
        float _yaw = 0.0f;
        float _pitch = 0.0f;
        bool _firstMouse = true;
        double _lastX = 0.0, _lastY = 0.0;
    };

} // namespace Drift::Renderer
