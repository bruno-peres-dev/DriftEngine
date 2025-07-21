// src/renderer/include/Drift/Renderer/IRenderPass.h
#pragma once

namespace Drift::Renderer {

    /// Interface gen�rica para um passo de renderiza��o
    class IRenderPass {
    public:
        virtual ~IRenderPass() = default;
        virtual void Execute() = 0;

        /// Ajusta o aspect ratio (para responder a resize)
        virtual void SetAspect(float /*aspect*/) {}
    };

} // namespace Drift::Renderer
