#pragma once

#include "Drift/RHI/Context.h"  // ISwapChain
#include <wrl/client.h>
#include <dxgi.h>

namespace Drift::RHI::DX11 {

    // Implementação DX11 de ISwapChain; ResizeBuffers apenas, ContextDX11 cuida de RTV/DSV
    class SwapChainDX11 : public Drift::RHI::ISwapChain {
    public:
        explicit SwapChainDX11(IDXGISwapChain* swapChain);
        ~SwapChainDX11() override = default;

        // Redimensiona os buffers de apresentação; após isso, chame ContextDX11::Resize()
        void Resize(unsigned width, unsigned height) override;

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
    };

} // namespace Drift::RHI::DX11
