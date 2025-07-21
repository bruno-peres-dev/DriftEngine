// src/renderer/include/Drift/Renderer/IRenderPass.h
#pragma once

namespace Drift::Renderer {

    /// Interface genérica para um passo de renderização
    class IRenderPass {
    public:
        virtual ~IRenderPass() = default;
        virtual void Execute() = 0;

        /// Ajusta o aspect ratio (para responder a resize)
        virtual void SetAspect(float /*aspect*/) {}
    };

} // namespace Drift::Renderer
