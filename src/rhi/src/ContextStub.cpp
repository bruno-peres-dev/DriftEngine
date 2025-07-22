#include "Drift/RHI/Context.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/Core/Log.h"

namespace Drift::RHI {

    // Implementações vazias apenas para link, sem GPU
    class ContextStub : public IContext {
    public:
        void Clear(float, float, float, float) override { /* nada */ }
        void Present() override { /* nada */ }
        void Resize(unsigned, unsigned) override { /* nada */ }
        
        // Input Assembler
        void IASetVertexBuffer(void*, UINT, UINT) override { /* nada */ }
        void IASetIndexBuffer(void*, Format, UINT) override { /* nada */ }
        void IASetPrimitiveTopology(PrimitiveTopology) override { /* nada */ }
        void DrawIndexed(UINT, UINT, INT) override { /* nada */ }
        void Draw(UINT, UINT) override { /* nada */ }
        void DrawInstanced(UINT, UINT, UINT, UINT) override { /* nada */ }
        void DrawIndexedInstanced(UINT, UINT, UINT, INT, UINT) override { /* nada */ }
        
        // Device access
        BackendHandle GetNativeDevice() const override { return nullptr; }
        BackendHandle GetNativeContext() const override { return nullptr; }
        
        // Resource binding
        void PSSetTexture(UINT, ITexture*) override { /* nada */ }
        void PSSetSampler(UINT, ISampler*) override { /* nada */ }
        void SetDepthTestEnabled(bool) override { /* nada */ }
        void VSSetConstantBuffer(UINT, BackendHandle) override { /* nada */ }
        void PSSetConstantBuffer(UINT, BackendHandle) override { /* nada */ }
        void GSSetConstantBuffer(UINT, BackendHandle) override { /* nada */ }
        
        // Viewport
        void SetViewport(int, int, int, int) override { /* nada */ }
        
        // Constant buffer update
        void UpdateConstantBuffer(IBuffer*, const void*, size_t) override { /* nada */ }
        
        // Render target
        void BindBackBufferRTV() override { /* nada */ }
    };

    class SwapChainStub : public ISwapChain {
    public:
        void Resize(unsigned, unsigned) override { /* nada */ }
    };

    class UIBatcherStub : public IUIBatcher {
    public:
        void Begin() override { 
            Drift::Core::Log("[UIBatcherStub] Begin() called");
        }
        
        void AddRect(float x, float y, float w, float h, unsigned color) override { 
            Drift::Core::Log("[UIBatcherStub] AddRect: pos=(" + std::to_string(x) + "," + std::to_string(y) + 
                             ") size=(" + std::to_string(w) + "," + std::to_string(h) + 
                             ") color=0x" + std::to_string(color));
        }
        
        void AddText(float x, float y, const char* text, unsigned color) override { 
            Drift::Core::Log("[UIBatcherStub] AddText: pos=(" + std::to_string(x) + "," + std::to_string(y) + 
                             ") text='" + std::string(text ? text : "null") + "' color=0x" + std::to_string(color));
        }
        
        void End() override { 
            Drift::Core::Log("[UIBatcherStub] End() called");
        }
        
        void SetScreenSize(float w, float h) override {
            Drift::Core::Log("[UIBatcherStub] SetScreenSize: " + std::to_string(w) + "x" + std::to_string(h));
        }
    };

    // Factory functions opcionais:
    std::unique_ptr<IContext>  CreateContextStub() { return std::make_unique<ContextStub>(); }
    std::unique_ptr<ISwapChain> CreateSwapChainStub() { return std::make_unique<SwapChainStub>(); }
    std::unique_ptr<IUIBatcher> CreateUIBatcherStub() { return std::make_unique<UIBatcherStub>(); }

} // namespace Drift::RHI
