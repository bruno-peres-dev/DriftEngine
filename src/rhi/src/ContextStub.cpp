#include "Drift/RHI/Context.h"

namespace Drift::RHI {

    // Implementações vazias apenas para link, sem GPU
    class ContextStub : public IContext {
    public:
        void Clear(float, float, float, float) override { /* nada */ }
        void Present() override { /* nada */ }
    };

    class SwapChainStub : public ISwapChain {
    public:
        void Resize(unsigned, unsigned) override { /* nada */ }
    };

    // Factory functions opcionais:
    std::unique_ptr<IContext>  CreateContextStub() { return std::make_unique<ContextStub>(); }
    std::unique_ptr<ISwapChain> CreateSwapChainStub() { return std::make_unique<SwapChainStub>(); }

} // namespace Drift::RHI
