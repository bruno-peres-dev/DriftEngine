#include "Drift/RHI/DX11/SwapChainDX11.h"
using namespace Drift::RHI::DX11;

// Construtor: armazena ponteiro do swapchain
SwapChainDX11::SwapChainDX11(IDXGISwapChain* swapChain)
    : _swapChain(swapChain)
{
}

// Redimensiona os buffers do swapchain (ContextDX11::Resize deve ser chamado em seguida)
void SwapChainDX11::Resize(unsigned width, unsigned height) {
    _swapChain->ResizeBuffers(
        0,               // mantém o número de buffers
        width, height,   // novas dimensões
        DXGI_FORMAT_UNKNOWN,
        0                // flags
    );
}
