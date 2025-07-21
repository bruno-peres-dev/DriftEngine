// src/renderer/include/Drift/Renderer/TerrainPass.h
#pragma once

#include "Drift/Renderer/IRenderPass.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/Types.h"
#include "Drift/Engine/Camera/ICamera.h"

#include <glm/glm.hpp>
#include <memory>
#include <cstdint>
#include "Drift/Renderer/CBFrame.h"

namespace Drift::Renderer {

    class TerrainPass : public IRenderPass {
    public:
        TerrainPass(Drift::RHI::IDevice& device,
            const std::wstring& texturePath,
            int rows = 100,
            int cols = 100,
            float uvScale = 1.0f,
            bool sphere = false);
        
        ~TerrainPass() override = default;

        // Nova interface sem acoplamento
        void Execute(RHI::IContext& context, const Engine::Camera::ICamera& camera) override;

    private:
        void BuildGrid(int rows, int cols);

        struct Vertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
        };

        Drift::RHI::IDevice& _device;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipeline;
        std::shared_ptr<Drift::RHI::IPipelineState> _pipelineWireframe;
        std::shared_ptr<Drift::RHI::IBuffer>        _vb;
        std::shared_ptr<Drift::RHI::IBuffer>        _ib;
        std::shared_ptr<Drift::RHI::IBuffer>        _cb;
        std::shared_ptr<Drift::RHI::ITexture>       _tex;
        std::shared_ptr<Drift::RHI::ISampler>       _samp;
        
        uint32_t                                    _indexCount;
        Drift::RHI::Format                          _indexFormat;
        
        // Configurações de renderização
        float _uvScale = 1.0f;
        bool _sphere = false;
        bool _showWireframe = false;
        
        std::vector<Vertex> _verts;
        std::shared_ptr<Drift::RHI::IRingBuffer> _ringBuffer;
    };

} // namespace Drift::Renderer