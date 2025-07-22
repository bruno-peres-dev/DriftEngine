#pragma once

#include "Drift/Engine/Camera/ICameraController.h"
#include "Drift/Engine/Camera/PerspectiveCamera.h"
#include <glm/glm.hpp>
#include "Drift/Core/Log.h"

namespace Drift::Engine::Camera {

    // Controlador de câmera livre (FPS-style)
    class FreeCameraController : public ICameraController {
    public:
        explicit FreeCameraController(std::unique_ptr<ICamera> camera)
            : _camera(std::move(camera))
            , _yaw(-90.0f)    // Olhando para frente (-Z)
            , _pitch(0.0f)    // Olhando horizontalmente
            , _movementSpeed(10.0f)
            , _mouseSensitivity(0.1f)
            , _firstMouse(true)
            , _lastMousePos(0.0f, 0.0f)
        {
            if (!_camera) {
                // Cria uma câmera padrão se não foi fornecida
                _camera = std::make_unique<PerspectiveCamera>();
            }
            
            // Inicializa baseado na posição atual da câmera
            UpdateFromCameraState();
        }
        
        // Construtor de conveniência
        FreeCameraController()
            : FreeCameraController(std::make_unique<PerspectiveCamera>())
        {
        }

        void Update(float deltaTime, const Input::InputFrame& input) override {
            if (!_enabled || !_camera) return;
            
            // Get current camera as PerspectiveCamera for modification
            auto* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
            if (!perspCamera) return; // Só funciona com PerspectiveCamera
            
            // === MOUSE LOOK ===
            HandleMouseLook(input);
            
            // === MOVEMENT ===
            HandleMovement(deltaTime, input, *perspCamera);
            
            // === SPEED CONTROL ===
            HandleSpeedControl(input);
        }

        ICamera* GetCamera() override { return _camera.get(); }
        const ICamera* GetCamera() const override { return _camera.get(); }
        
        // Configurações específicas
        void SetMovementSpeed(float speed) { _movementSpeed = speed; }
        float GetMovementSpeed() const { return _movementSpeed; }
        
        void SetMouseSensitivity(float sensitivity) { _mouseSensitivity = sensitivity; }
        float GetMouseSensitivity() const { return _mouseSensitivity; }
        
        void SetSpeedMultiplier(float multiplier) { _speedMultiplier = multiplier; }
        float GetSpeedMultiplier() const { return _speedMultiplier; }
        
        // Controle manual da orientação
        void SetYaw(float yaw) { 
            _yaw = yaw;
            UpdateCameraFromAngles();
        }
        
        void SetPitch(float pitch) { 
            _pitch = glm::clamp(pitch, -89.0f, 89.0f);
            UpdateCameraFromAngles();
        }
        
        float GetYaw() const { return _yaw; }
        float GetPitch() const { return _pitch; }
        
    private:
        std::unique_ptr<ICamera> _camera;
        
        // Ângulos de rotação
        float _yaw;
        float _pitch;
        
        // Configurações de movimento
        float _movementSpeed;
        float _mouseSensitivity;
        float _speedMultiplier = 1.0f;
        
        // Estado do mouse
        bool _firstMouse;
        glm::vec2 _lastMousePos;
        
        void HandleMouseLook(const Input::InputFrame& input) {
            glm::vec2 currentMousePos = input.mousePosition;
            
            if (_firstMouse) {
                _lastMousePos = currentMousePos;
                _firstMouse = false;
                return;
            }
            
            // Calcula delta do mouse
            glm::vec2 mouseDelta = currentMousePos - _lastMousePos;
            _lastMousePos = currentMousePos;
            
            // Aplica sensitividade
            mouseDelta *= _mouseSensitivity;
            
            // Atualiza ângulos
            _yaw += mouseDelta.x;
            _pitch -= mouseDelta.y; // Inverte Y para comportamento natural
            
            // Limita pitch
            _pitch = glm::clamp(_pitch, -89.0f, 89.0f);
            
            UpdateCameraFromAngles();
        }
        
        void HandleMovement(float deltaTime, const Input::InputFrame& input, 
                           PerspectiveCamera& camera) {

            float currentSpeed = _movementSpeed * _speedMultiplier * deltaTime;
            
            glm::vec3 position = camera.GetPosition();
            glm::vec3 forward = camera.GetForward();
            glm::vec3 right = camera.GetRight();
            glm::vec3 up = camera.GetUp();
            
            // Movimento WASD
            if (input.IsKeyDown(Input::Key::W)) {
                position += forward * currentSpeed;
            }
            if (input.IsKeyDown(Input::Key::S)) {
                position -= forward * currentSpeed;
            }
            if (input.IsKeyDown(Input::Key::A)) {
                position -= right * currentSpeed;
            }
            if (input.IsKeyDown(Input::Key::D)) {
                position += right * currentSpeed;
            }
            
            // Movimento vertical (Space/Ctrl)
            if (input.IsKeyDown(Input::Key::Space)) {
                position += up * currentSpeed;
            }
            if (input.IsKeyDown(Input::Key::LeftCtrl)) {
                position -= up * currentSpeed;
            }
            
            camera.SetPosition(position);
        }
        
        void HandleSpeedControl(const Input::InputFrame& input) {
            // Shift para movimento rápido
            if (input.IsKeyDown(Input::Key::LeftShift)) {
                _speedMultiplier = 3.0f;
            } else {
                _speedMultiplier = 1.0f;
            }
        }
        
        void UpdateCameraFromAngles() {
            auto* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
            if (!perspCamera) return;
            
            // Calcula direção forward baseada nos ângulos
            glm::vec3 forward;
            forward.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
            forward.y = sin(glm::radians(_pitch));
            forward.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
            forward = glm::normalize(forward);
            
            // Atualiza target da câmera
            glm::vec3 position = perspCamera->GetPosition();
            perspCamera->SetTarget(position + forward);
        }
        
        void UpdateFromCameraState() {
            auto* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
            if (!perspCamera) return;
            
            // Calcula yaw e pitch baseado na direção atual
            glm::vec3 forward = perspCamera->GetForward();
            
            _yaw = glm::degrees(atan2(forward.z, forward.x));
            _pitch = glm::degrees(asin(forward.y));
            
            // Garante que pitch está no range válido
            _pitch = glm::clamp(_pitch, -89.0f, 89.0f);
        }
    };

} // namespace Drift::Engine::Camera 