#pragma once

#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Shader.h"
#include "Drift/RHI/Context.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Drift::RHI::DX11 {

    /// Implementa��o DX11 de IPipelineState
    class PipelineStateDX11 : public IPipelineState {
    public:
        PipelineStateDX11(ID3D11Device* device, const PipelineDesc& desc);
        ~PipelineStateDX11() override = default;

        void Apply(IContext& ctx) override;

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout>  _inputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> _vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>  _ps;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11BlendState> _blendState; // Novo: blend state avançado
    };

    /// Cria um PipelineStateDX11 e devolve shared_ptr<IPipelineState>
    std::shared_ptr<IPipelineState> CreatePipelineDX11(ID3D11Device* device, const PipelineDesc& desc);

} // namespace Drift::RHI::DX11
