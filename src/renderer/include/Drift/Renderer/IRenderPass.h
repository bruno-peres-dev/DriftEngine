// src/renderer/include/Drift/Renderer/IRenderPass.h
#pragma once

#include "Drift/RHI/Context.h"

// Forward declaration
namespace Drift::Engine::Camera {
    class ICamera;
}

namespace Drift::Renderer {

    /// Interface genérica para um passo de renderização
    class IRenderPass {
    public:
        virtual ~IRenderPass() = default;
        
        // Nova interface: recebe contexto RHI e câmera como parâmetros
        virtual void Execute(RHI::IContext& context, const Engine::Camera::ICamera& camera) = 0;
        
        // Métodos de configuração opcionais
        virtual void SetEnabled(bool enabled) { _enabled = enabled; }
        virtual bool IsEnabled() const { return _enabled; }
        
        virtual void SetName(const std::string& name) { _name = name; }
        virtual const std::string& GetName() const { return _name; }
        
    protected:
        bool _enabled = true;
        std::string _name = "UnnamedRenderPass";
    };

} // namespace Drift::Renderer
