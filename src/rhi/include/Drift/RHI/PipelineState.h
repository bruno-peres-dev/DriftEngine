// src/rhi/include/Drift/RHI/PipelineState.h
#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Drift::RHI {

struct InputElementDesc {
    std::string semanticName;
    unsigned semanticIndex;
    unsigned offset;
    std::string format; // Ex: "R32G32B32_FLOAT"
    bool operator==(const InputElementDesc& o) const {
        return semanticName == o.semanticName &&
               semanticIndex == o.semanticIndex &&
               offset == o.offset &&
               format == o.format;
    }
};

struct PipelineDesc {
    std::string vsFile;
    std::string psFile;
    std::string gsFile;
    std::string csFile;
    std::vector<InputElementDesc> inputLayout;
    std::vector<std::pair<std::string, std::string>> defines;
    struct BlendDesc {
        bool enable = false;
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
        CullMode cullMode = CullMode::Back;
        bool wireframe = false;
        bool operator==(const RasterizerDesc& o) const {
            return cullMode == o.cullMode && wireframe == o.wireframe;
        }
    } rasterizer;
    struct DepthStencilDesc {
        bool depthEnable = true;
        bool depthWrite = true;
        bool operator==(const DepthStencilDesc& o) const {
            return depthEnable == o.depthEnable && depthWrite == o.depthWrite;
        }
    } depthStencil;
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

class IPipelineState {
public:
    virtual ~IPipelineState() = default;
    virtual void Apply(class IContext& ctx) = 0;
};

} // namespace Drift::RHI

// specialize hash for PipelineDesc
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
