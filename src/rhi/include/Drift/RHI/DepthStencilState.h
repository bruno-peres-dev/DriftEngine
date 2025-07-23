#pragma once

#include <memory>
#include <cstdint>
#include <cstring>
#include <functional>

namespace Drift::RHI {

    enum class ComparisonFunc {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class StencilOp {
        Keep,
        Zero,
        Replace,
        IncrementSaturate,
        DecrementSaturate,
        Invert,
        Increment,
        Decrement
    };

    struct DepthStencilDesc {
        bool depthEnable = true;
        bool depthWrite = true;
        ComparisonFunc depthFunc = ComparisonFunc::Less;
        bool stencilEnable = false;
        uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;
        StencilOp frontStencilFailOp = StencilOp::Keep;
        StencilOp frontStencilDepthFailOp = StencilOp::Keep;
        StencilOp frontStencilPassOp = StencilOp::Keep;
        ComparisonFunc frontStencilFunc = ComparisonFunc::Always;
        bool separateBackFace = false;
        StencilOp backStencilFailOp = StencilOp::Keep;
        StencilOp backStencilDepthFailOp = StencilOp::Keep;
        StencilOp backStencilPassOp = StencilOp::Keep;
        ComparisonFunc backStencilFunc = ComparisonFunc::Always;
        uint32_t stencilRef = 0;
        
        bool operator==(const DepthStencilDesc& o) const {
            return std::memcmp(this, &o, sizeof(DepthStencilDesc)) == 0;
        }
    };

    class DepthStencilState {
    public:
        virtual ~DepthStencilState() = default;
        virtual void Apply(void* context) = 0;
        virtual const DepthStencilDesc& GetDesc() const = 0;
        virtual void* GetBackendHandle() const = 0;
    };

    std::shared_ptr<DepthStencilState> CreateDepthStencilState(const DepthStencilDesc& desc);
}

// Especializações de hash e equal_to para uso em unordered_map
namespace std {
    template<>
    struct equal_to<Drift::RHI::DepthStencilDesc> {
        bool operator()(const Drift::RHI::DepthStencilDesc& a, const Drift::RHI::DepthStencilDesc& b) const noexcept {
            return std::memcmp(&a, &b, sizeof(Drift::RHI::DepthStencilDesc)) == 0;
        }
    };
    template<>
    struct hash<Drift::RHI::DepthStencilDesc> {
        size_t operator()(const Drift::RHI::DepthStencilDesc& desc) const noexcept {
            const std::uint64_t* p = reinterpret_cast<const std::uint64_t*>(&desc);
            size_t h = 0;
            for (size_t i = 0; i < sizeof(Drift::RHI::DepthStencilDesc) / sizeof(std::uint64_t); ++i) {
                h ^= std::hash<std::uint64_t>{}(p[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };
} 