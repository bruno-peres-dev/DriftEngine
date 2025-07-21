// src/renderer/src/TrianglePass.cpp
#include "Drift/Renderer/TrianglePass.h"
#include "Drift/Core/Log.h"
#include "Drift/RHI/DX11/ContextDX11.h"
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Drift/RHI/Types.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace Drift::Renderer {

    struct Vertex { glm::vec3 pos; };

    TrianglePass::TrianglePass(Drift::RHI::IDevice& device,
        Drift::RHI::IContext& context)
        : _device(device)
        , _context(context)
    {
        Drift::Core::Log("Criando pipeline...");
        Drift::RHI::PipelineDesc pd;
        pd.vsFile = "shaders/TriangleVS.hlsl";
        pd.psFile = "shaders/TrianglePS.hlsl";
        pd.inputLayout = {
            { "POSITION", 0, 0,  "R32G32B32_FLOAT" }
        };
        _pipeline = _device.CreatePipeline(pd);

        // Triângulo centralizado no plano XZ, de frente para a câmera
        float size = 2.0f;
        float aspect = _camera.GetAspect();
        std::vector<Vertex> verts = {
            {{ 0.0f, 0.0f,  size }},                // topo
            {{ size * aspect, 0.0f, -size }},       // canto direito
            {{-size * aspect, 0.0f, -size }},       // canto esquerdo
        };
        for (const auto& v : verts) {
            Drift::Core::Log("Vertex: pos=(" + std::to_string(v.pos.x) + "," + std::to_string(v.pos.y) + "," + std::to_string(v.pos.z) + ")");
        }
        Drift::RHI::BufferDesc vbd{
            Drift::RHI::BufferType::Vertex,
            verts.size() * sizeof(Vertex),
            verts.data()
        };
        _vb = _device.CreateBuffer(vbd);

        // Constant buffer (camera)
        Drift::RHI::BufferDesc cbd{
            Drift::RHI::BufferType::Constant,
            sizeof(CBFrame),
            nullptr
        };
        _cb = _device.CreateBuffer(cbd);

        // Câmera inicial
        _camera.SetFovY(glm::radians(60.0f));
        _camera.SetAspect(1280.0f / 720.0f);
        _camera.SetPosition({ 0.0f, 0.0f, -20.0f });
        _camera.SetTarget({ 0.0f, 0.0f, 0.0f });
        _camera.SetNearFar(0.01f, 10000.0f);
        _yaw = -90.0f;
        _pitch = 0.0f;
        _lastX = 1280.0f / 2.0f;
        _lastY = 720.0f / 2.0f;
        _firstMouse = true;
    }

    void TrianglePass::Update(float dt, GLFWwindow* window) {
        // Mouse look
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (_firstMouse) {
            _lastX = xpos; _lastY = ypos; _firstMouse = false;
        }
        float xoffset = float(xpos - _lastX);
        float yoffset = float(_lastY - ypos); // invertido
        _lastX = xpos; _lastY = ypos;
        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;
        _yaw += xoffset;
        _pitch += yoffset;
        if (_pitch > 89.0f) _pitch = 89.0f;
        if (_pitch < -89.0f) _pitch = -89.0f;

        // WASD move
        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front = glm::normalize(front);
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0,1,0)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));
        float speed = 2.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= 3.0f;
        glm::vec3 pos = _camera.GetPosition();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) _camera.SetPosition(pos + front * speed);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) _camera.SetPosition(pos - front * speed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) _camera.SetPosition(pos - right * speed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) _camera.SetPosition(pos + right * speed);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) _camera.SetPosition(pos + up * speed);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) _camera.SetPosition(pos - up * speed);
        // ESC fecha janela
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
        // Atualiza target da câmera para onde está olhando
        _camera.SetTarget(_camera.GetPosition() + front);
    }

    void TrianglePass::Execute() {
        // Limpa o render target (preto para debug)
        _context.Clear(0.0f, 0.0f, 0.0f, 1.0f);

        // 1) Atualiza constant buffer (view/proj da câmera)
        CBFrame cbf{ _camera.GetViewProjForHLSL() };
        void* ptr = _cb->Map();
        memcpy(ptr, glm::value_ptr(cbf.viewProj), sizeof(cbf.viewProj));
        _cb->Unmap();
        // Bind explícito do constant buffer no slot b0 do vertex shader
        _context.VSSetConstantBuffer(0, _cb->GetBackendHandle());

        // 2) Liga pipeline
        _pipeline->Apply(_context);

        // 3) Bind dos buffers e topology
        _context.IASetVertexBuffer(_vb->GetBackendHandle(), sizeof(Vertex), 0);
        _context.IASetPrimitiveTopology(Drift::RHI::PrimitiveTopology::TriangleList);

        // Log para diagnóstico
        Drift::Core::Log("[TrianglePass] DrawIndexed chamado");

        // 4) Desabilita depth test
        _context.SetDepthTestEnabled(false);

        // 5) Desenha
        auto* nativeCtx = _context.GetNativeContext();
        if (!nativeCtx) {
            Drift::Core::Log("[TrianglePass][ERRO] GetNativeContext() retornou nullptr!");
            return;
        }
        auto* d3dCtx = static_cast<ID3D11DeviceContext*>(nativeCtx);
        d3dCtx->Draw(3, 0);
    }

} // namespace Drift::Renderer
