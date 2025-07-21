#include "Drift/RHI/DX11/SwapChainDX11.h"
using namespace Drift::RHI::DX11;

SwapChainDX11::SwapChainDX11(IDXGISwapChain* swapChain)
    : _swapChain(swapChain)
{
}

void SwapChainDX11::Resize(unsigned width, unsigned height) {
    // Apenas redimensiona os buffers
    // (o ContextDX11::Resize() deve ser chamado em seguida para
    // recriar RTV/DSV e ajustar o viewport)
    _swapChain->ResizeBuffers(
        0,               // deixar o count inalterado
        width, height,   // novas dimensões
        DXGI_FORMAT_UNKNOWN,
        0                // flags
    );
}
