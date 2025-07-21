// src/renderer/src/TerrainPass.cpp

#include "Drift/Renderer/TerrainPass.h"
#include "Drift/RHI/Types.h"
#include "Drift/Core/Log.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Drift/Math/Math.h"
#include <iostream>
#include "Drift/RHI/DX11/RingBufferDX11.h"

using namespace Drift::Renderer;
using namespace Drift::RHI;

TerrainPass::TerrainPass(Drift::RHI::IDevice& device,
                         const std::wstring& texturePath,
                         int rows,
                         int cols,
                         float uvScale,
                         bool sphere)
    : _device(device)
    , _uvScale(uvScale)
    , _sphere(sphere)
{
    SetName("TerrainPass");
    
    PipelineDesc pd;
    pd.vsFile = "shaders/TerrainVS.hlsl";
    pd.psFile = "shaders/TerrainPS.hlsl";
    pd.inputLayout = {
        { "POSITION", 0, 0,  "R32G32B32_FLOAT" },
        { "NORMAL",   0, 12, "R32G32B32_FLOAT" },
        { "TEXCOORD", 0, 24, "R32G32_FLOAT" }
    };
    pd.rasterizer.cullMode = Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::Back;
    pd.rasterizer.wireframe = false;
    
    _pipeline = _device.CreatePipeline(pd);
    
    // Pipeline wireframe
    PipelineDesc pdWire = pd;
    pdWire.rasterizer.wireframe = true;
    pdWire.defines.push_back({ "WIREFRAME", "1" });
    _pipelineWireframe = _device.CreatePipeline(pdWire);

    BuildGrid(rows, cols);

    BufferDesc cbd{ BufferType::Constant, sizeof(CBFrame), nullptr };
    _cb = _device.CreateBuffer(cbd);

    TextureDesc td{}; 
    td.path = texturePath;
    _tex = _device.CreateTexture(td);

    SamplerDesc sd{};
    _samp = _device.CreateSampler(sd);

    // RingBuffer será criado dinamicamente no primeiro Execute() quando temos contexto
    _ringBuffer = nullptr;
    
    Drift::Core::Log("[TerrainPass] Inicializado com sucesso");
}

void TerrainPass::BuildGrid(int rows, int cols)
{
    const float scale  = _sphere ? 0.0f   : 1000.0f;
    const float radius = _sphere ? 100.0f : 0.0f;
    const float dx     = scale / cols;
    const float dz     = scale / rows;

    std::vector<Vertex> verts;
    std::vector<uint32_t> idx;

    if (_sphere) {
        for (int y = 0; y <= rows; ++y) {
            float v   = float(y) / float(rows);
            float phi = v * Math::PI;
            for (int x = 0; x <= cols; ++x) {
                float u     = float(x) / float(cols);
                float theta = u * 2.0f * Math::PI;
                glm::vec3 pos = {
                    radius * sin(phi) * cos(theta),
                    radius * cos(phi),
                    radius * sin(phi) * sin(theta)
                };
                glm::vec3 normal = glm::normalize(pos);
                verts.push_back({ pos, normal, { u * _uvScale, v * _uvScale } });
            }
        }
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                int i0 = y * (cols + 1) + x;
                int i1 = i0 + 1;
                int i2 = i0 + (cols + 1);
                int i3 = i2 + 1;

                idx.push_back(uint32_t(i0));
                idx.push_back(uint32_t(i1));
                idx.push_back(uint32_t(i2));
                idx.push_back(uint32_t(i1));
                idx.push_back(uint32_t(i3));
                idx.push_back(uint32_t(i2));
            }
        }
    } else {
        for (int z = 0; z <= rows; ++z) {
            for (int x = 0; x <= cols; ++x) {
                glm::vec3 pos    = { x * dx, 0.0f, z * dz };
                glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
                float u = float(x) / float(cols);
                float v = float(z) / float(rows);
                verts.push_back({ pos, normal, { u * _uvScale, v * _uvScale } });
            }
        }
        for (int z = 0; z < rows; ++z) {
            for (int x = 0; x < cols; ++x) {
                int i0 = z * (cols + 1) + x;
                int i1 = i0 + 1;
                int i2 = i0 + (cols + 1);
                int i3 = i2 + 1;
                idx.push_back(uint32_t(i0));
                idx.push_back(uint32_t(i2));
                idx.push_back(uint32_t(i1));
                idx.push_back(uint32_t(i1));
                idx.push_back(uint32_t(i2));
                idx.push_back(uint32_t(i3));
            }
        }
    }

    _indexCount  = uint32_t(idx.size());
    _indexFormat = Format::R32_UINT;
    _vb = _device.CreateBuffer({ BufferType::Vertex, verts.size() * sizeof(Vertex), verts.data() });
    _ib = _device.CreateBuffer({ BufferType::Index, idx.size() * sizeof(uint32_t), idx.data() });
    
    _verts = verts;
    
    Drift::Core::Log("[TerrainPass] Grid construído: " + std::to_string(_indexCount) + " índices");
}

void TerrainPass::Execute(RHI::IContext& context, const Engine::Camera::ICamera& camera)
{
    if (!IsEnabled()) return;
    
    // Inicializa RingBuffer na primeira execução (lazy initialization)
    if (!_ringBuffer) {
        _ringBuffer = Drift::RHI::DX11::CreateRingBufferDX11(
            static_cast<ID3D11Device*>(_device.GetNativeDevice()),
            static_cast<ID3D11DeviceContext*>(context.GetNativeContext()),
            4 * 1024 * 1024 // 4MB
        );
        Drift::Core::Log("[TerrainPass] RingBuffer inicializado com sucesso");
    }
    
    // Atualiza constant buffer com matriz da câmera
    CBFrame cbf{ camera.GetViewProjectionMatrixForHLSL() };
    context.UpdateConstantBuffer(_cb.get(), &cbf, sizeof(CBFrame));
    context.VSSetConstantBuffer(0, _cb->GetBackendHandle());

    // Seleciona pipeline baseado no modo wireframe
    auto& pipeline = _showWireframe ? _pipelineWireframe : _pipeline;
    
    if (pipeline && !_verts.empty() && _indexCount > 0) {
        pipeline->Apply(context);
        
                 // Upload dinâmico via RingBuffer (preferível) ou buffers estáticos (fallback)
         if (_ringBuffer) {
             _ringBuffer->NextFrame();
             size_t vtxSize = _verts.size() * sizeof(Vertex);
             size_t vtxOffset = 0;
             void* vtxPtr = _ringBuffer->Allocate(vtxSize, 16, vtxOffset);
             memcpy(vtxPtr, _verts.data(), vtxSize);

             IBuffer* vtxBuf = _ringBuffer->GetBuffer();
             IBuffer* idxBuf = _ib.get(); // buffer de índices estático
             context.IASetVertexBuffer(vtxBuf->GetBackendHandle(), sizeof(Vertex), (UINT)vtxOffset);
             context.IASetIndexBuffer(idxBuf->GetBackendHandle(), _indexFormat, 0);
         } else {
             // Fallback para buffers estáticos
             context.IASetVertexBuffer(_vb->GetBackendHandle(), sizeof(Vertex), 0);
             context.IASetIndexBuffer(_ib->GetBackendHandle(), _indexFormat, 0);
         }
        
        context.IASetPrimitiveTopology(PrimitiveTopology::TriangleList);
        context.SetDepthTestEnabled(true);
        context.PSSetTexture(0, _tex.get());
        context.PSSetSampler(0, _samp.get());
        context.DrawIndexed(_indexCount, 0, 0);
    }
}