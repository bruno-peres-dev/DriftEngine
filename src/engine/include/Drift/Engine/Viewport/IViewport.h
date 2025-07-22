#pragma once

#include "Drift/Engine/Camera/ICameraController.h"
#include "Drift/Engine/Input/InputTypes.h"
#include "Drift/RHI/Context.h"
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "Drift/Renderer/IRenderPass.h"

namespace Drift::Engine::Viewport {

    // Descrição de uma viewport
    struct ViewportDesc {
        std::string name;
        
        // Posição e tamanho na tela (em pixels)
        int x = 0;
        int y = 0;
        int width = 1280;
        int height = 720;
        
        // Configurações
        bool enabled = true;
        bool acceptsInput = true;  // Se aceita input do mouse/teclado
        
        // Cor de fundo
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        
        bool operator==(const ViewportDesc& other) const {
            return name == other.name && 
                   x == other.x && y == other.y &&
                   width == other.width && height == other.height &&
                   enabled == other.enabled && acceptsInput == other.acceptsInput;
        }
    };

    // Interface abstrata para viewports
    class IViewport {
    public:
        virtual ~IViewport() = default;
        
        // Atualização e renderização
        virtual void Update(float deltaTime, const Input::InputFrame& input) = 0;
        virtual void Render(RHI::IContext& context) = 0;
        
        // Controle de tamanho
        virtual void Resize(int width, int height) = 0;
        
        // Acesso aos componentes
        virtual Camera::ICamera* GetCamera() = 0;
        virtual const Camera::ICamera* GetCamera() const = 0;
        
        // Descrição da viewport
        virtual const ViewportDesc& GetDesc() const = 0;
        virtual void SetDesc(const ViewportDesc& desc) = 0;
        
        // Controle de ativação
        virtual void SetEnabled(bool enabled) = 0;
        virtual bool IsEnabled() const = 0;
        
        // Teste de hit (ponto está dentro da viewport?)
        virtual bool IsPointInside(int x, int y) const = 0;
        
        // Conversão de coordenadas
        virtual glm::vec2 ScreenToViewport(const glm::vec2& screenPoint) const = 0;
        virtual glm::vec2 ViewportToScreen(const glm::vec2& viewportPoint) const = 0;
    };

    // Viewport básica com câmera e render passes
    class BasicViewport : public IViewport {
    public:
        BasicViewport(ViewportDesc desc,
                     std::unique_ptr<Camera::ICameraController> cameraController,
                     std::vector<std::shared_ptr<Renderer::IRenderPass>> renderPasses = {})
            : _desc(std::move(desc))
            , _cameraController(std::move(cameraController))
            , _renderPasses(std::move(renderPasses))
        {
            if (!_cameraController) {
                throw std::runtime_error("BasicViewport requer um CameraController válido");
            }
            // Garante que a câmara nasce com o aspect ratio correcto
            if (_cameraController && _cameraController->GetCamera()) {
                float aspect = (_desc.height > 0) ? static_cast<float>(_desc.width) / static_cast<float>(_desc.height) : 1.0f;
                _cameraController->GetCamera()->SetAspectRatio(aspect);
            }
        }
        
        void Update(float deltaTime, const Input::InputFrame& input) override {
            if (!_desc.enabled) return;
            
            // O RenderManager já decide qual viewport recebe o InputFrame real.
            // Portanto, se acceptsInput estiver true, processamos o input diretamente.
            if (_desc.acceptsInput) {
                _cameraController->Update(deltaTime, input);
            }
        }
        
        void Render(RHI::IContext& context) override {
            if (!_desc.enabled) return;
            
            // Define viewport no contexto RHI
            context.SetViewport(_desc.x, _desc.y, _desc.width, _desc.height);
            
            // Clear com a cor da viewport
            context.Clear(_desc.clearColor[0], _desc.clearColor[1], 
                         _desc.clearColor[2], _desc.clearColor[3]);
            
            // Executa render passes
            for (auto& pass : _renderPasses) {
                if (pass) {
                    pass->Execute(context, *GetCamera());
                }
            }
        }
        
        void Resize(int width, int height) override {
            _desc.width = width;
            _desc.height = height;
            
            // Atualiza aspect ratio da câmera
            if (_cameraController && _cameraController->GetCamera()) {
                float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
                _cameraController->GetCamera()->SetAspectRatio(aspect);
            }
        }
        
        Camera::ICamera* GetCamera() override {
            return _cameraController ? _cameraController->GetCamera() : nullptr;
        }
        
        const Camera::ICamera* GetCamera() const override {
            return _cameraController ? _cameraController->GetCamera() : nullptr;
        }
        
        const ViewportDesc& GetDesc() const override { return _desc; }
        
        void SetDesc(const ViewportDesc& desc) override {
            // Detecta alteração de tamanho ANTES de sobrescrever _desc
            bool sizeChanged = (_desc.width != desc.width) || (_desc.height != desc.height);
            _desc = desc;
            if (sizeChanged) {
                Resize(_desc.width, _desc.height);
            }
        }
        
        void SetEnabled(bool enabled) override { _desc.enabled = enabled; }
        bool IsEnabled() const override { return _desc.enabled; }
        
        bool IsPointInside(int x, int y) const override {
            return x >= _desc.x && x < _desc.x + _desc.width &&
                   y >= _desc.y && y < _desc.y + _desc.height;
        }
        
        glm::vec2 ScreenToViewport(const glm::vec2& screenPoint) const override {
            return glm::vec2(
                screenPoint.x - static_cast<float>(_desc.x),
                screenPoint.y - static_cast<float>(_desc.y)
            );
        }
        
        glm::vec2 ViewportToScreen(const glm::vec2& viewportPoint) const override {
            return glm::vec2(
                viewportPoint.x + static_cast<float>(_desc.x),
                viewportPoint.y + static_cast<float>(_desc.y)
            );
        }
        
        // Métodos específicos
        Camera::ICameraController* GetCameraController() { return _cameraController.get(); }
        
        void AddRenderPass(std::shared_ptr<Renderer::IRenderPass> pass) {
            if (pass) {
                _renderPasses.push_back(pass);
            }
        }
        
        void RemoveRenderPass(std::shared_ptr<Renderer::IRenderPass> pass) {
            _renderPasses.erase(
                std::remove(_renderPasses.begin(), _renderPasses.end(), pass),
                _renderPasses.end()
            );
        }
        
        const std::vector<std::shared_ptr<Renderer::IRenderPass>>& GetRenderPasses() const {
            return _renderPasses;
        }
        
    private:
        ViewportDesc _desc;
        std::unique_ptr<Camera::ICameraController> _cameraController;
        std::vector<std::shared_ptr<Renderer::IRenderPass>> _renderPasses;
    };

} // namespace Drift::Engine::Viewport 