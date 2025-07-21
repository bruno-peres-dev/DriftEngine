#pragma once
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/Context.h"
#include <vector>
#include <memory>

namespace Drift::RHI::DX11 {

// Implementação DX11 de IUIBatcher para batching de primitivas 2D
class UIBatcherDX11 : public Drift::RHI::IUIBatcher {
public:
    UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, Drift::RHI::IContext* ctx);
    void Begin() override;
    void AddRect(float x, float y, float w, float h, unsigned color) override;
    void AddText(float x, float y, const char* text, unsigned color) override; // Stub
    void End() override;
private:
    struct Vertex {
        float x, y;
        unsigned color;
    };
    std::vector<Vertex> _vertices;
    std::vector<unsigned> _indices;
    std::shared_ptr<IRingBuffer> _ringBuffer;
    Drift::RHI::IContext* _ctx;
};

// Cria um UIBatcherDX11
std::unique_ptr<Drift::RHI::IUIBatcher> CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, Drift::RHI::IContext* ctx);

} // namespace Drift::RHI::DX11 