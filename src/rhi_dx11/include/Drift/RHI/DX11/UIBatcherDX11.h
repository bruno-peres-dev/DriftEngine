#pragma once
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/Context.h"
#include "Drift/Core/Color.h"
#include <vector>
#include <memory>

// Forward declaration para o sistema de texto
namespace Drift::UI { class UIBatcherTextRenderer; }

namespace Drift::RHI { class IPipelineState; }

namespace Drift::RHI::DX11 {

// Estrutura para representar um retângulo de clipping
struct ScissorRect {
    float x, y, width, height;
    
    ScissorRect() : x(0), y(0), width(0), height(0) {}
    ScissorRect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    bool IsValid() const { return width > 0 && height > 0; }
};

// Implementação DX11 de IUIBatcher para batching de primitivas 2D
class UIBatcherDX11 : public Drift::RHI::IUIBatcher {
public:
    UIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, Drift::RHI::IContext* ctx);
    void Begin() override;
    void AddRect(float x, float y, float w, float h, Drift::Color color) override;
    void AddQuad(float x0, float y0, float x1, float y1,
                 float x2, float y2, float x3, float y3,
                 Drift::Color color) override;
    void AddText(float x, float y, const char* text, Drift::Color color) override;
    void End() override;

    void SetScreenSize(float w, float h) override;
    
    // Métodos para gerenciar scissor rectangles
    void PushScissorRect(float x, float y, float w, float h) override;
    void PopScissorRect() override;
    void ClearScissorRects() override;
    
private:
    struct Vertex {
        float x, y;
        Drift::Color color;
    };

    // Helper
    void EnsurePipeline();
    bool IsRectVisible(const ScissorRect& rect) const;
    ScissorRect GetCurrentScissorRect() const;
    ScissorRect ClipRectToScissor(const ScissorRect& rect, const ScissorRect& scissor) const;

    float _screenW{1280.0f};
    float _screenH{720.0f};
    std::shared_ptr<Drift::RHI::IPipelineState> _pipeline;
    std::vector<Vertex> _vertices;
    std::vector<unsigned> _indices;
    std::shared_ptr<IRingBuffer> _ringBuffer;
    Drift::RHI::IContext* _ctx;
    
    // Stack de scissor rectangles
    std::vector<ScissorRect> _scissorStack;
    
    // Sistema de renderização de texto
    std::unique_ptr<Drift::UI::UIBatcherTextRenderer> _textRenderer;
};

// Cria um UIBatcherDX11
std::unique_ptr<Drift::RHI::IUIBatcher> CreateUIBatcherDX11(std::shared_ptr<IRingBuffer> ringBuffer, Drift::RHI::IContext* ctx);

} // namespace Drift::RHI::DX11 