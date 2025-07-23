#pragma once

#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Shader.h"
#include "Drift/RHI/Context.h"
#include "Drift/RHI/DepthStencilState.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Drift::RHI::DX11 {

    // Implementação DX11 de IPipelineState
    class PipelineStateDX11 : public IPipelineState {
    public:
        PipelineStateDX11(ID3D11Device* device, const PipelineDesc& desc);
        ~PipelineStateDX11() override = default;

        void Apply(IContext& ctx) override;
        
        // Métodos da interface IResource
        void* GetBackendHandle() const override;
        size_t GetMemoryUsage() const override;

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout>  _inputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> _vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>  _ps;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> _gs; // Geometry Shader
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11BlendState> _blendState; // Blend state avançado
        std::shared_ptr<Drift::RHI::DepthStencilState> _depthStencilState; // Depth stencil state unificado
    };

    // Cria um PipelineStateDX11 e retorna shared_ptr<IPipelineState>
    std::shared_ptr<IPipelineState> CreatePipelineDX11(ID3D11Device* device, const PipelineDesc& desc);

} // namespace Drift::RHI::DX11
