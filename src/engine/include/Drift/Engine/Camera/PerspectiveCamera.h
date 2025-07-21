#pragma once

#include "Drift/Engine/Camera/ICamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Drift::Engine::Camera {

    class PerspectiveCamera : public ICamera {
    public:
        PerspectiveCamera(
            const glm::vec3& position = {0.0f, 0.0f, -5.0f},
            const glm::vec3& target = {0.0f, 0.0f, 0.0f},
            const glm::vec3& up = {0.0f, 1.0f, 0.0f},
            float fovY = glm::radians(45.0f),
            float aspect = 16.0f / 9.0f,
            float nearPlane = 0.1f,
            float farPlane = 1000.0f
        )
            : _position(position)
            , _target(target)
            , _worldUp(up)
            , _fovY(fovY)
            , _aspect(aspect)
            , _nearPlane(nearPlane)
            , _farPlane(farPlane)
            , _viewMatrixDirty(true)
            , _projectionMatrixDirty(true)
            , _viewProjectionMatrixDirty(true)
        {
            UpdateCameraVectors();
        }
        
        // Implementação da interface ICamera
        const glm::mat4& GetViewMatrix() const override {
            if (_viewMatrixDirty) {
                UpdateViewMatrix();
            }
            return _viewMatrix;
        }
        
        const glm::mat4& GetProjectionMatrix() const override {
            if (_projectionMatrixDirty) {
                UpdateProjectionMatrix();
            }
            return _projectionMatrix;
        }
        
        const glm::mat4& GetViewProjectionMatrix() const override {
            if (_viewProjectionMatrixDirty || _viewMatrixDirty || _projectionMatrixDirty) {
                UpdateViewProjectionMatrix();
            }
            return _viewProjectionMatrix;
        }
        
        glm::vec3 GetPosition() const override { return _position; }
        glm::vec3 GetForward() const override { return _forward; }
        glm::vec3 GetRight() const override { return _right; }
        glm::vec3 GetUp() const override { return _up; }
        
        void SetAspectRatio(float aspect) override {
            if (_aspect != aspect) {
                _aspect = aspect;
                _projectionMatrixDirty = true;
                _viewProjectionMatrixDirty = true;
            }
        }
        
        float GetAspectRatio() const override { return _aspect; }
        
        void SetNearPlane(float nearPlane) override {
            if (_nearPlane != nearPlane) {
                _nearPlane = nearPlane;
                _projectionMatrixDirty = true;
                _viewProjectionMatrixDirty = true;
            }
        }
        
        float GetNearPlane() const override { return _nearPlane; }
        
        void SetFarPlane(float farPlane) override {
            if (_farPlane != farPlane) {
                _farPlane = farPlane;
                _projectionMatrixDirty = true;
                _viewProjectionMatrixDirty = true;
            }
        }
        
        float GetFarPlane() const override { return _farPlane; }
        
        Ray GetRayFromScreenPoint(const glm::vec2& screenPoint, 
                                 const glm::vec2& screenSize) const override {
            // Converte ponto da tela para NDC (-1 a 1)
            glm::vec2 ndc = {
                (screenPoint.x / screenSize.x) * 2.0f - 1.0f,
                1.0f - (screenPoint.y / screenSize.y) * 2.0f  // Inverte Y
            };
            
            // Ponto no near plane em clip space
            glm::vec4 nearPoint = glm::vec4(ndc.x, ndc.y, -1.0f, 1.0f);
            // Ponto no far plane em clip space
            glm::vec4 farPoint = glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);
            
            // Inverse view-projection matrix
            glm::mat4 invViewProj = glm::inverse(GetViewProjectionMatrix());
            
            // Converte para world space
            nearPoint = invViewProj * nearPoint;
            farPoint = invViewProj * farPoint;
            
            // Perspectiva divide
            if (nearPoint.w != 0.0f) nearPoint /= nearPoint.w;
            if (farPoint.w != 0.0f) farPoint /= farPoint.w;
            
            Ray ray;
            ray.origin = glm::vec3(nearPoint);
            ray.direction = glm::normalize(glm::vec3(farPoint) - glm::vec3(nearPoint));
            
            return ray;
        }
        
        Frustum GetFrustum() const override {
            return ExtractFrustumFromMatrix(GetViewProjectionMatrix());
        }
        
        // Métodos específicos da PerspectiveCamera
        void SetPosition(const glm::vec3& position) {
            if (_position != position) {
                _position = position;
                _viewMatrixDirty = true;
                _viewProjectionMatrixDirty = true;
            }
        }
        
        void SetTarget(const glm::vec3& target) {
            if (_target != target) {
                _target = target;
                UpdateCameraVectors();
                _viewMatrixDirty = true;
                _viewProjectionMatrixDirty = true;
            }
        }
        
        void SetWorldUp(const glm::vec3& up) {
            if (_worldUp != up) {
                _worldUp = up;
                UpdateCameraVectors();
                _viewMatrixDirty = true;
                _viewProjectionMatrixDirty = true;
            }
        }
        
        void SetFovY(float fovY) {
            if (_fovY != fovY) {
                _fovY = fovY;
                _projectionMatrixDirty = true;
                _viewProjectionMatrixDirty = true;
            }
        }
        
        float GetFovY() const { return _fovY; }
        
        const glm::vec3& GetTarget() const { return _target; }
        const glm::vec3& GetWorldUp() const { return _worldUp; }
        
    private:
        // Posicionamento
        glm::vec3 _position;
        glm::vec3 _target;
        glm::vec3 _worldUp;
        
        // Vetores da câmera (calculados)
        glm::vec3 _forward;
        glm::vec3 _right;
        glm::vec3 _up;
        
        // Parâmetros de projeção
        float _fovY;
        float _aspect;
        float _nearPlane;
        float _farPlane;
        
        // Matrizes (cached)
        mutable glm::mat4 _viewMatrix;
        mutable glm::mat4 _projectionMatrix;
        mutable glm::mat4 _viewProjectionMatrix;
        
        // Dirty flags para lazy evaluation
        mutable bool _viewMatrixDirty;
        mutable bool _projectionMatrixDirty;
        mutable bool _viewProjectionMatrixDirty;
        
        void UpdateCameraVectors() {
            _forward = glm::normalize(_target - _position);
            _right = glm::normalize(glm::cross(_forward, _worldUp));
            _up = glm::normalize(glm::cross(_right, _forward));
        }
        
        void UpdateViewMatrix() const {
            _viewMatrix = glm::lookAtLH(_position, _target, _worldUp);
            _viewMatrixDirty = false;
        }
        
        void UpdateProjectionMatrix() const {
            _projectionMatrix = glm::perspectiveLH_ZO(_fovY, _aspect, _nearPlane, _farPlane);
            _projectionMatrixDirty = false;
        }
        
        void UpdateViewProjectionMatrix() const {
            // Garante que view e projection estão atualizadas
            if (_viewMatrixDirty) UpdateViewMatrix();
            if (_projectionMatrixDirty) UpdateProjectionMatrix();
            
            _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
            _viewProjectionMatrixDirty = false;
        }
    };

} // namespace Drift::Engine::Camera 