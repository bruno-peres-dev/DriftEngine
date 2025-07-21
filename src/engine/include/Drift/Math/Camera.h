// src/engine/include/Drift/Math/Camera.h
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Drift::Math {

class Camera {
public:
    Camera() = default;

    // Setters
    void SetPosition(const glm::vec3& pos) { _pos = pos; _dirty = true; }
    void SetTarget(const glm::vec3& tgt)   { _tgt = tgt; _dirty = true; }
    void SetUp(const glm::vec3& up)        { _up = up;   _dirty = true; }
    void SetFovY(float fovy)               { _fovY = fovy; _dirty = true; }
    void SetAspect(float aspect)           { _aspect = aspect; _dirty = true; }
    void SetNearFar(float zn, float zf)    { _zn = zn; _zf = zf; _dirty = true; }

    // Getters
    const glm::vec3& GetPosition() const { return _pos; }
    const glm::vec3& GetTarget()   const { return _tgt; }
    const glm::vec3& GetUp()       const { return _up;  }
    float GetFovY()   const { return _fovY; }
    float GetAspect() const { return _aspect; }
    float GetNear()   const { return _zn; }
    float GetFar()    const { return _zf; }

    // Matrizes
    const glm::mat4& GetView() {
        UpdateIfDirty();
        return _view;
    }
    const glm::mat4& GetProj() {
        UpdateIfDirty();
        return _proj;
    }
    const glm::mat4& GetViewProj() {
        UpdateIfDirty();
        return _vp;
    }
    // Para HLSL: viewProj transposta (column-major, compatível com mul(float4, M))
    glm::mat4 GetViewProjForHLSL() {
        return glm::transpose(GetViewProj());
    }
    // Para GLSL: viewProj sem transpor
    glm::mat4 GetViewProjForGLSL() {
        return GetViewProj();
    }

private:
    void UpdateIfDirty() {
        if (_dirty) {
            _view = glm::lookAtLH(_pos, _tgt, _up);
            _proj = glm::perspectiveLH_ZO(_fovY, _aspect, _zn, _zf);
            _vp = _proj * _view;
            _dirty = false;
        }
    }

    glm::vec3 _pos = { 0,0,-5 };
    glm::vec3 _tgt = { 0,0, 0 };
    glm::vec3 _up  = { 0,1, 0 };
    float     _fovY = glm::radians(45.0f);
    float     _aspect = 16.0f / 9.0f;
    float     _zn = 0.1f;
    float     _zf = 100.0f;

    glm::mat4 _view = glm::mat4(1.0f);
    glm::mat4 _proj = glm::mat4(1.0f);
    glm::mat4 _vp   = glm::mat4(1.0f);
    bool      _dirty = true;
};

} // namespace Drift::Math
