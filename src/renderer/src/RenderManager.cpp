#include "Drift/Renderer/RenderManager.h"
#include "Drift/Core/Log.h"
#include <chrono>
#include <algorithm>

namespace Drift::Renderer {

    void RenderManager::Update(float deltaTime, const Engine::Input::InputFrame& input) {
        // Reset statistics for this frame
        _stats.Reset();
        
        // Start timing this frame
        _frameStartTime = std::chrono::high_resolution_clock::now();
        
        // Detecta troca de foco (clique com botão esquerdo)
        if (input.IsMouseButtonPressed(Engine::Input::MouseButton::Left)) {
            for (const std::string& name : _renderOrder) {
                auto it = _viewports.find(name);
                if (it != _viewports.end() && it->second) {
                    if (it->second->IsPointInside(static_cast<int>(input.mousePosition.x),
                                                  static_cast<int>(input.mousePosition.y)) &&
                        it->second->GetDesc().acceptsInput) {
                        _activeViewport = name;
                        break;
                    }
                }
            }
        }

        Engine::Input::InputFrame emptyInput; // tudo Released

        // Update viewports: somente a ativa recebe input real
        for (const std::string& name : _renderOrder) {
            auto it = _viewports.find(name);
            if (it != _viewports.end() && it->second) {
                auto& viewport = it->second;
                if (viewport->IsEnabled()) {
                    const auto& inp = (name == _activeViewport) ? input : emptyInput;
                    viewport->Update(deltaTime, inp);
                }
            }
        }
        
        //Drift::Core::Log("[RenderManager] Updated " + std::to_string(_renderOrder.size()) + " viewports");
    }
    
    void RenderManager::Render(RHI::IContext& context) {
        _stats.viewportsRendered = 0;
        
        // Render all viewports in order
        for (const std::string& name : _renderOrder) {
            auto it = _viewports.find(name);
            if (it != _viewports.end() && it->second) {
                auto& viewport = it->second;
                if (viewport->IsEnabled()) {
                    viewport->Render(context);
                    _stats.viewportsRendered++;
                }
            }
        }
        
        // Update frame timing
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(
            frameEndTime - _frameStartTime);
        _stats.frameTime = frameDuration.count() / 1000.0f; // Convert to milliseconds
        
        //Drift::Core::Log("[RenderManager] Rendered " + std::to_string(_stats.viewportsRendered) + " viewports");
    }
    
    void RenderManager::SetLayout(ViewportLayout layout, int screenWidth, int screenHeight) {
        _currentLayout = layout;
        
        switch (layout) {
            case ViewportLayout::Single:
                ApplySingleLayout(screenWidth, screenHeight);
                break;
            case ViewportLayout::SplitHorizontal:
                ApplySplitHorizontalLayout(screenWidth, screenHeight);
                break;
            case ViewportLayout::SplitVertical:
                ApplySplitVerticalLayout(screenWidth, screenHeight);
                break;
            case ViewportLayout::Quad:
                ApplyQuadLayout(screenWidth, screenHeight);
                break;
            case ViewportLayout::Custom:
                // Layout customizado - não faz nada, deixa as viewports como estão
                break;
        }
        
        Drift::Core::Log("[RenderManager] Applied layout: " + std::to_string(static_cast<int>(layout)));
    }
    
    void RenderManager::SetAllViewportsEnabled(bool enabled) {
        for (auto& [name, viewport] : _viewports) {
            if (viewport) {
                viewport->SetEnabled(enabled);
            }
        }
        
        Drift::Core::Log("[RenderManager] Set all viewports enabled: " + std::string(enabled ? "true" : "false"));
    }
    
    void RenderManager::ResizeAllViewports(int screenWidth, int screenHeight) {
        // Reaplica o layout atual com as novas dimensões
        SetLayout(_currentLayout, screenWidth, screenHeight);
    }
    
    void RenderManager::ApplySingleLayout(int screenWidth, int screenHeight) {
        // Uma viewport ocupa toda a tela
        if (_renderOrder.empty()) return;
        
        auto* viewport = GetViewport(_renderOrder[0]);
        if (viewport) {
            Engine::Viewport::ViewportDesc desc = viewport->GetDesc();
            desc.x = 0;
            desc.y = 0;
            desc.width = screenWidth;
            desc.height = screenHeight;
            viewport->SetDesc(desc);
        }
        
        // Desabilita outras viewports se existirem
        for (size_t i = 1; i < _renderOrder.size(); ++i) {
            auto* vp = GetViewport(_renderOrder[i]);
            if (vp) {
                vp->SetEnabled(false);
            }
        }
    }
    
    void RenderManager::ApplySplitHorizontalLayout(int screenWidth, int screenHeight) {
        // Duas viewports lado a lado
        if (_renderOrder.size() < 2) return;
        
        int halfWidth = screenWidth / 2;
        
        // Viewport da esquerda
        auto* leftViewport = GetViewport(_renderOrder[0]);
        if (leftViewport) {
            Engine::Viewport::ViewportDesc desc = leftViewport->GetDesc();
            desc.x = 0;
            desc.y = 0;
            desc.width = halfWidth;
            desc.height = screenHeight;
            desc.enabled = true;
            leftViewport->SetDesc(desc);
        }
        
        // Viewport da direita
        auto* rightViewport = GetViewport(_renderOrder[1]);
        if (rightViewport) {
            Engine::Viewport::ViewportDesc desc = rightViewport->GetDesc();
            desc.x = halfWidth;
            desc.y = 0;
            desc.width = screenWidth - halfWidth;
            desc.height = screenHeight;
            desc.enabled = true;
            rightViewport->SetDesc(desc);
        }
        
        // Desabilita viewports extras
        for (size_t i = 2; i < _renderOrder.size(); ++i) {
            auto* vp = GetViewport(_renderOrder[i]);
            if (vp) {
                vp->SetEnabled(false);
            }
        }
    }
    
    void RenderManager::ApplySplitVerticalLayout(int screenWidth, int screenHeight) {
        // Duas viewports em cima/baixo
        if (_renderOrder.size() < 2) return;
        
        int halfHeight = screenHeight / 2;
        
        // Viewport de cima
        auto* topViewport = GetViewport(_renderOrder[0]);
        if (topViewport) {
            Engine::Viewport::ViewportDesc desc = topViewport->GetDesc();
            desc.x = 0;
            desc.y = 0;
            desc.width = screenWidth;
            desc.height = halfHeight;
            desc.enabled = true;
            topViewport->SetDesc(desc);
        }
        
        // Viewport de baixo
        auto* bottomViewport = GetViewport(_renderOrder[1]);
        if (bottomViewport) {
            Engine::Viewport::ViewportDesc desc = bottomViewport->GetDesc();
            desc.x = 0;
            desc.y = halfHeight;
            desc.width = screenWidth;
            desc.height = screenHeight - halfHeight;
            desc.enabled = true;
            bottomViewport->SetDesc(desc);
        }
        
        // Desabilita viewports extras
        for (size_t i = 2; i < _renderOrder.size(); ++i) {
            auto* vp = GetViewport(_renderOrder[i]);
            if (vp) {
                vp->SetEnabled(false);
            }
        }
    }
    
    void RenderManager::ApplyQuadLayout(int screenWidth, int screenHeight) {
        // Quatro viewports em quadrantes
        if (_renderOrder.size() < 4) return;
        
        int halfWidth = screenWidth / 2;
        int halfHeight = screenHeight / 2;
        
        // Quadrante superior esquerdo
        auto* topLeftViewport = GetViewport(_renderOrder[0]);
        if (topLeftViewport) {
            Engine::Viewport::ViewportDesc desc = topLeftViewport->GetDesc();
            desc.x = 0;
            desc.y = 0;
            desc.width = halfWidth;
            desc.height = halfHeight;
            desc.enabled = true;
            topLeftViewport->SetDesc(desc);
        }
        
        // Quadrante superior direito
        auto* topRightViewport = GetViewport(_renderOrder[1]);
        if (topRightViewport) {
            Engine::Viewport::ViewportDesc desc = topRightViewport->GetDesc();
            desc.x = halfWidth;
            desc.y = 0;
            desc.width = screenWidth - halfWidth;
            desc.height = halfHeight;
            desc.enabled = true;
            topRightViewport->SetDesc(desc);
        }
        
        // Quadrante inferior esquerdo
        auto* bottomLeftViewport = GetViewport(_renderOrder[2]);
        if (bottomLeftViewport) {
            Engine::Viewport::ViewportDesc desc = bottomLeftViewport->GetDesc();
            desc.x = 0;
            desc.y = halfHeight;
            desc.width = halfWidth;
            desc.height = screenHeight - halfHeight;
            desc.enabled = true;
            bottomLeftViewport->SetDesc(desc);
        }
        
        // Quadrante inferior direito
        auto* bottomRightViewport = GetViewport(_renderOrder[3]);
        if (bottomRightViewport) {
            Engine::Viewport::ViewportDesc desc = bottomRightViewport->GetDesc();
            desc.x = halfWidth;
            desc.y = halfHeight;
            desc.width = screenWidth - halfWidth;
            desc.height = screenHeight - halfHeight;
            desc.enabled = true;
            bottomRightViewport->SetDesc(desc);
        }
        
        // Desabilita viewports extras
        for (size_t i = 4; i < _renderOrder.size(); ++i) {
            auto* vp = GetViewport(_renderOrder[i]);
            if (vp) {
                vp->SetEnabled(false);
            }
        }
    }

} // namespace Drift::Renderer 