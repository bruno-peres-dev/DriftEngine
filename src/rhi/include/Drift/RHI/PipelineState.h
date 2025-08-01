// src/rhi/include/Drift/RHI/PipelineState.h
#pragma once

#include "Drift/RHI/Format.h"
#include "Drift/RHI/DepthStencilState.h"
#include "Drift/RHI/Resource.h"
#include <memory>
#include <string>
#include <vector>

namespace Drift::RHI {

// Usa InputElementDesc do Format.h (já definido lá)

// Descrição completa de um pipeline gráfico
struct PipelineDesc {
    std::string vsFile; // Vertex shader
    std::string psFile; // Pixel shader
    std::string gsFile; // Geometry shader (opcional)
    std::string csFile; // Compute shader (opcional)
    std::vector<InputElementDesc> inputLayout; // Layout dos vértices
    std::vector<std::pair<std::string, std::string>> defines; // Macros de compilação
    std::string vsEntry = "VSMain";
    std::string psEntry = "PSMain";
    std::string gsEntry = "GS"; // Novo: entrypoint do geometry shader
    struct BlendDesc {
        bool enable = false; // Ativa blending
        enum class BlendFactor {
            Zero, One, SrcColor, InvSrcColor, SrcAlpha, InvSrcAlpha, DestAlpha, InvDestAlpha, DestColor, InvDestColor, SrcAlphaSaturate
        } srcColor = BlendFactor::One, dstColor = BlendFactor::Zero,
          srcAlpha = BlendFactor::One, dstAlpha = BlendFactor::Zero;
        enum class BlendOp {
            Add, Subtract, RevSubtract, Min, Max
        } colorOp = BlendOp::Add, alphaOp = BlendOp::Add;
        bool alphaToCoverage = false;
        bool blendFactorSeparate = false;
        bool operator==(const BlendDesc& o) const {
            return enable == o.enable &&
                   srcColor == o.srcColor && dstColor == o.dstColor &&
                   srcAlpha == o.srcAlpha && dstAlpha == o.dstAlpha &&
                   colorOp == o.colorOp && alphaOp == o.alphaOp &&
                   alphaToCoverage == o.alphaToCoverage &&
                   blendFactorSeparate == o.blendFactorSeparate;
        }
    };
    BlendDesc blend;
    struct RasterizerDesc {
        enum class CullMode { None, Back, Front };
        CullMode cullMode = CullMode::Back; // Modo de culling
        bool wireframe = false;             // Renderização wireframe
        bool operator==(const RasterizerDesc& o) const {
            return cullMode == o.cullMode && wireframe == o.wireframe;
        }
    } rasterizer;
    // Usa o DepthStencilDesc unificado do DepthStencilState.h
    DepthStencilDesc depthStencil;
    bool operator==(const PipelineDesc& o) const {
        return vsFile == o.vsFile &&
               psFile == o.psFile &&
               gsFile == o.gsFile &&
               csFile == o.csFile &&
               inputLayout == o.inputLayout &&
               defines == o.defines &&
               blend == o.blend &&
               rasterizer == o.rasterizer &&
               depthStencil == o.depthStencil;
    }
};

// Interface para pipeline state (encapsula configuração de renderização)
class IPipelineState : public IResource {
public:
    virtual ~IPipelineState() = default;
    virtual void Apply(class IContext& ctx) = 0;
};

} // namespace Drift::RHI

// Especialização de hash para PipelineDesc
#include <functional>
namespace std {
    template<>
    struct hash<Drift::RHI::PipelineDesc> {
        size_t operator()(Drift::RHI::PipelineDesc const& p) const noexcept {
            auto h1 = hash<string>{}(p.vsFile);
            auto h2 = hash<string>{}(p.psFile);
            return h1 ^ (h2 << 1);
        }
    };
}
