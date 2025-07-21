#pragma once

#include <glm/glm.hpp>

namespace Drift::Engine::Camera {

    // Interface abstrata para câmeras
    class ICamera {
    public:
        virtual ~ICamera() = default;
        
        // Matrizes de transformação
        virtual const glm::mat4& GetViewMatrix() const = 0;
        virtual const glm::mat4& GetProjectionMatrix() const = 0;
        virtual const glm::mat4& GetViewProjectionMatrix() const = 0;
        
        // Compatibilidade com HLSL (transposta para column-major)
        virtual glm::mat4 GetViewProjectionMatrixForHLSL() const {
            return glm::transpose(GetViewProjectionMatrix());
        }
        
        // Posicionamento e orientação
        virtual glm::vec3 GetPosition() const = 0;
        virtual glm::vec3 GetForward() const = 0;
        virtual glm::vec3 GetRight() const = 0;
        virtual glm::vec3 GetUp() const = 0;
        
        // Configurações de projeção
        virtual void SetAspectRatio(float aspect) = 0;
        virtual float GetAspectRatio() const = 0;
        
        virtual void SetNearPlane(float nearPlane) = 0;
        virtual float GetNearPlane() const = 0;
        
        virtual void SetFarPlane(float farPlane) = 0;
        virtual float GetFarPlane() const = 0;
        
        // Ray casting (útil para mouse picking)
        struct Ray {
            glm::vec3 origin;
            glm::vec3 direction;
        };
        
        virtual Ray GetRayFromScreenPoint(const glm::vec2& screenPoint, 
                                         const glm::vec2& screenSize) const = 0;
        
        // Frustum planes (para culling)
        struct Frustum {
            glm::vec4 planes[6]; // left, right, bottom, top, near, far
        };
        
        virtual Frustum GetFrustum() const = 0;
        
    protected:
        // Helper para extrair frustum da view-projection matrix
        static Frustum ExtractFrustumFromMatrix(const glm::mat4& viewProj) {
            Frustum frustum;
            
            // Left plane
            frustum.planes[0] = glm::vec4(
                viewProj[0][3] + viewProj[0][0],
                viewProj[1][3] + viewProj[1][0],
                viewProj[2][3] + viewProj[2][0],
                viewProj[3][3] + viewProj[3][0]
            );
            
            // Right plane
            frustum.planes[1] = glm::vec4(
                viewProj[0][3] - viewProj[0][0],
                viewProj[1][3] - viewProj[1][0],
                viewProj[2][3] - viewProj[2][0],
                viewProj[3][3] - viewProj[3][0]
            );
            
            // Bottom plane
            frustum.planes[2] = glm::vec4(
                viewProj[0][3] + viewProj[0][1],
                viewProj[1][3] + viewProj[1][1],
                viewProj[2][3] + viewProj[2][1],
                viewProj[3][3] + viewProj[3][1]
            );
            
            // Top plane
            frustum.planes[3] = glm::vec4(
                viewProj[0][3] - viewProj[0][1],
                viewProj[1][3] - viewProj[1][1],
                viewProj[2][3] - viewProj[2][1],
                viewProj[3][3] - viewProj[3][1]
            );
            
            // Near plane
            frustum.planes[4] = glm::vec4(
                viewProj[0][3] + viewProj[0][2],
                viewProj[1][3] + viewProj[1][2],
                viewProj[2][3] + viewProj[2][2],
                viewProj[3][3] + viewProj[3][2]
            );
            
            // Far plane
            frustum.planes[5] = glm::vec4(
                viewProj[0][3] - viewProj[0][2],
                viewProj[1][3] - viewProj[1][2],
                viewProj[2][3] - viewProj[2][2],
                viewProj[3][3] - viewProj[3][2]
            );
            
            // Normalize all planes
            for (int i = 0; i < 6; ++i) {
                float length = glm::length(glm::vec3(frustum.planes[i]));
                if (length > 0.0f) {
                    frustum.planes[i] /= length;
                }
            }
            
            return frustum;
        }
    };

} // namespace Drift::Engine::Camera 