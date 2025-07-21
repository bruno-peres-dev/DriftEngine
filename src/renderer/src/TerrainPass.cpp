// src/renderer/src/TerrainPass.cpp

#include "Drift/Renderer/TerrainPass.h"
#include "Drift/RHI/Types.h"
#include "Drift/Core/Log.h"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Drift/Math/Math.h"
#include <iostream>
#include "Drift/RHI/DX11/RingBufferDX11.h"

using namespace Drift::Renderer;
using namespace Drift::RHI;

TerrainPass::TerrainPass(Drift::RHI::IDevice& device,
                         Drift::RHI::IContext& context,
                         const std::wstring& texturePath,
                         int rows,
                         int cols,
                         float uvScale,
                         bool sphere)
    : _device(device)
    , _context(context)
    , _uvScale(uvScale)
    , _sphere(sphere)
{
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
    std::cerr << "[TerrainPass] Criando pipeline: cullMode=" << static_cast<int>(pd.rasterizer.cullMode) << std::endl;
    _pipeline = _device.CreatePipeline(pd); // Nenhum define
    std::cerr << "[TerrainPass] _pipeline: " << (_pipeline ? "ok" : "null") << std::endl;

    PipelineDesc pdWire = pd;
    pdWire.rasterizer.cullMode = Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::Back;
    pdWire.rasterizer.wireframe = true;
    std::cerr << "[TerrainPass] Criando pipelineWireframe: cullMode=None" << std::endl;
    pdWire.defines.push_back({ "WIREFRAME", "1" });
    _pipelineWireframe = _device.CreatePipeline(pdWire); // Só wireframe
    std::cerr << "[TerrainPass] _pipelineWireframe: " << (_pipelineWireframe ? "ok" : "null") << std::endl;

    PipelineDesc pdLine = pd;
    pdLine.psFile = "shaders/LinePS.hlsl";
    pdLine.rasterizer.cullMode = Drift::RHI::PipelineDesc::RasterizerDesc::CullMode::Back;
    pdLine.rasterizer.wireframe = false;
    std::cerr << "[TerrainPass] Criando pipelineLineTest: cullMode=None" << std::endl;
    _pipelineLineTest = _device.CreatePipeline(pdLine);
    std::cerr << "[TerrainPass] _pipelineLineTest: " << (_pipelineLineTest ? "ok" : "null") << std::endl;

    BuildGrid(rows, cols);

    BufferDesc cbd{ BufferType::Constant, sizeof(CBFrame), nullptr };
    _cb = _device.CreateBuffer(cbd);
    std::cerr << "[TerrainPass] _cb: " << (_cb ? "ok" : "null") << std::endl;

    TextureDesc td{}; td.path = texturePath;
    _tex = _device.CreateTexture(td);
    std::cerr << "[TerrainPass] _tex: " << (_tex ? "ok" : "null") << std::endl;

    SamplerDesc sd{};
    _samp = _device.CreateSampler(sd);
    std::cerr << "[TerrainPass] _samp: " << (_samp ? "ok" : "null") << std::endl;

    _camera.SetFovY(glm::radians(45.0f));
    _camera.SetAspect(1.0f);
    _camera.SetPosition({ 500.0f, 10.0f, 800.0f });
    _camera.SetTarget({ 500.0f, 0.0f, 500.0f });
    _camera.SetNearFar(0.1f, 100000.0f);

    _ringBuffer = Drift::RHI::DX11::CreateRingBufferDX11(
        static_cast<ID3D11Device*>(_device.GetNativeDevice()),
        static_cast<ID3D11DeviceContext*>(_context.GetNativeContext()),
        4 * 1024 * 1024 // 4MB para testes
    );
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
    _ib = _device.CreateBuffer({ BufferType::Index,  idx.size()   * sizeof(uint32_t), idx.data()   });
    std::cerr << "[TerrainPass] _vb: " << (_vb ? "ok" : "null") << ", _ib: " << (_ib ? "ok" : "null") << ", _indexCount: " << _indexCount << std::endl;
    _verts = verts;
}

void TerrainPass::SetAspect(float aspect)
{
    _camera.SetAspect(aspect);
}

void TerrainPass::Update(float dt, GLFWwindow* window)
{
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

void TerrainPass::Execute()
{
    _ringBuffer->NextFrame();
    _context.Clear(0.0f, 0.0f, 0.0f, 1.0f); // Este método já limpa cor e depth-stencil no DX11

    CBFrame cbf{ _camera.GetViewProjForHLSL() };
    Drift::RHI::UpdateConstantBuffer(_cb.get(), cbf);
    _context.VSSetConstantBuffer(0, _cb->GetBackendHandle());

    // 1. Sempre desenha o sólido (textura)
    if (_pipeline && !_verts.empty() && _indexCount > 0) {
        _pipeline->Apply(_context);
        // Upload dinâmico para o ring buffer (apenas vértices)
        size_t vtxSize = _verts.size() * sizeof(Vertex);
        size_t vtxOffset = 0;
        void* vtxPtr = _ringBuffer->Allocate(vtxSize, 16, vtxOffset);
        memcpy(vtxPtr, _verts.data(), vtxSize);

        IBuffer* vtxBuf = _ringBuffer->GetBuffer();
        IBuffer* idxBuf = _ib.get(); // buffer de índices estático
        _context.IASetVertexBuffer(vtxBuf->GetBackendHandle(), sizeof(Vertex), (UINT)vtxOffset);
        _context.IASetIndexBuffer(idxBuf->GetBackendHandle(), _indexFormat, 0);
        _context.IASetPrimitiveTopology(PrimitiveTopology::TriangleList);
        _context.SetDepthTestEnabled(true);
        _context.PSSetTexture(0, _tex.get());
        _context.PSSetSampler(0, _samp.get());
        _context.DrawIndexed(_indexCount, 0, 0);
    }
    
    // Adicionar toggle para linhas das normais com F2
    if (_showNormalLines && !_verts.empty() && _pipelineLineTest) {
        const float normalScale = 5.0f;
        std::vector<Vertex> normalLineVerts;
        normalLineVerts.reserve(_verts.size() * 2);
        for (const auto& v : _verts) {
            normalLineVerts.push_back({ v.pos, v.normal, v.uv });
            normalLineVerts.push_back({ v.pos + v.normal * normalScale, v.normal, v.uv });
        }
        size_t normalLineVBSize = normalLineVerts.size() * sizeof(Vertex);
        size_t normalLineVBOffset = 0;
        void* normalLineVBPtr = _ringBuffer->Allocate(normalLineVBSize, 16, normalLineVBOffset);
        memcpy(normalLineVBPtr, normalLineVerts.data(), normalLineVBSize);
        IBuffer* normalLineVB = _ringBuffer->GetBuffer();
        if (normalLineVB) {
            _pipelineLineTest->Apply(_context);
            _context.IASetVertexBuffer(normalLineVB->GetBackendHandle(), sizeof(Vertex), (UINT)normalLineVBOffset);
            _context.IASetPrimitiveTopology(PrimitiveTopology::LineList);
            _context.SetDepthTestEnabled(false);
            _context.Draw(static_cast<UINT>(normalLineVerts.size()), 0);
        }
    }
    // 2. Se wireframe, desenha por cima
    if (_showWireframe && _pipelineWireframe && _vb && _ib && _indexCount > 0) {
        _pipelineWireframe->Apply(_context);
        _context.IASetVertexBuffer(_vb->GetBackendHandle(), sizeof(Vertex), 0);
        _context.IASetIndexBuffer(_ib->GetBackendHandle(), _indexFormat, 0);
        _context.IASetPrimitiveTopology(PrimitiveTopology::TriangleList);
        _context.SetDepthTestEnabled(false);
        _context.PSSetTexture(0, _tex.get());
        _context.PSSetSampler(0, _samp.get());
        _context.DrawIndexed(_indexCount, 0, 0);
    }
}
