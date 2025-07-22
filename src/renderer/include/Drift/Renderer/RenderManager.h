#pragma once

#include "Drift/Engine/Viewport/IViewport.h"
#include "Drift/Engine/Input/InputTypes.h"
#include "Drift/RHI/Context.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>

namespace Drift::Renderer {

    // Estatísticas de renderização
    struct RenderStats {
        uint32_t viewportsRendered = 0;
        uint32_t renderPassesExecuted = 0;
        float frameTime = 0.0f;
        uint32_t drawCalls = 0;
        uint32_t triangles = 0;
        uint64_t gpuMemoryUsed = 0;
        
        void Reset() {
            viewportsRendered = 0;
            renderPassesExecuted = 0;
            frameTime = 0.0f;
            drawCalls = 0;
            triangles = 0;
            gpuMemoryUsed = 0;
        }
    };
    
    // Layout para organizar viewports automaticamente
    enum class ViewportLayout {
        Single,      // Uma viewport ocupando toda a área
        SplitHorizontal, // Duas viewports lado a lado
        SplitVertical,   // Duas viewports em cima/baixo
        Quad,        // Quatro viewports
        Custom       // Layout manual
    };

    // Gerenciador central de renderização e viewports
    class RenderManager {
    public:
        RenderManager() = default;
        ~RenderManager() = default;
        
        // Gerenciamento de viewports
        void AddViewport(const std::string& name, std::unique_ptr<Engine::Viewport::IViewport> viewport);
        void RemoveViewport(const std::string& name);
        Engine::Viewport::IViewport* GetViewport(const std::string& name);
        const Engine::Viewport::IViewport* GetViewport(const std::string& name) const;
        
        // Lista todas as viewports
        std::vector<std::string> GetViewportNames() const;
        size_t GetViewportCount() const { return _viewports.size(); }
        
        // Atualização e renderização
        void Update(float deltaTime, const Engine::Input::InputFrame& input);
        void Render(RHI::IContext& context);
        
        // Layout automático
        void SetLayout(ViewportLayout layout, int screenWidth, int screenHeight);
        ViewportLayout GetCurrentLayout() const { return _currentLayout; }
        
        // Controle global
        void SetAllViewportsEnabled(bool enabled);
        void ResizeAllViewports(int screenWidth, int screenHeight);
        
        // Estatísticas
        const RenderStats& GetStats() const { return _stats; }
        void ResetStats() { _stats.Reset(); }
        
        // Debug e performance
        void SetVSyncEnabled(bool enabled) { _vsyncEnabled = enabled; }
        bool IsVSyncEnabled() const { return _vsyncEnabled; }
        
        void SetWireframeMode(bool enabled) { _wireframeMode = enabled; }
        bool IsWireframeMode() const { return _wireframeMode; }

        // Foco de viewport
        void SetActiveViewport(const std::string& name) { _activeViewport = name; }
        const std::string& GetActiveViewport() const { return _activeViewport; }

    private:
        // Storage de viewports
        std::unordered_map<std::string, std::unique_ptr<Engine::Viewport::IViewport>> _viewports;
        std::vector<std::string> _renderOrder; // Ordem de renderização
        
        // Estado atual
        ViewportLayout _currentLayout = ViewportLayout::Single;
        RenderStats _stats;
        
        // Configurações globais
        bool _vsyncEnabled = true;
        bool _wireframeMode = false;

        // Viewport atualmente com foco/input
        std::string _activeViewport;

        // Helpers para layout
        void ApplySingleLayout(int screenWidth, int screenHeight);
        void ApplySplitHorizontalLayout(int screenWidth, int screenHeight);
        void ApplySplitVerticalLayout(int screenWidth, int screenHeight);
        void ApplyQuadLayout(int screenWidth, int screenHeight);
        
        // Timer para estatísticas
        mutable float _statsTimer = 0.0f;
        mutable std::chrono::high_resolution_clock::time_point _frameStartTime;
    };

    // Implementação inline de métodos simples
    inline void RenderManager::AddViewport(const std::string& name, std::unique_ptr<Engine::Viewport::IViewport> viewport) {
        if (!viewport) return;
        
        // Remove viewport anterior se existir
        RemoveViewport(name);
        
        // Adiciona nova viewport
        _viewports[name] = std::move(viewport);
        _renderOrder.push_back(name);

        // Se ainda não houver viewport ativa, define esta
        if (_activeViewport.empty()) {
            _activeViewport = name;
        }
    }
    
    inline void RenderManager::RemoveViewport(const std::string& name) {
        auto it = _viewports.find(name);
        if (it != _viewports.end()) {
            _viewports.erase(it);
            
            // Remove da ordem de renderização
            auto orderIt = std::find(_renderOrder.begin(), _renderOrder.end(), name);
            if (orderIt != _renderOrder.end()) {
                _renderOrder.erase(orderIt);
            }
        }
    }
    
    inline Engine::Viewport::IViewport* RenderManager::GetViewport(const std::string& name) {
        auto it = _viewports.find(name);
        return (it != _viewports.end()) ? it->second.get() : nullptr;
    }
    
    inline const Engine::Viewport::IViewport* RenderManager::GetViewport(const std::string& name) const {
        auto it = _viewports.find(name);
        return (it != _viewports.end()) ? it->second.get() : nullptr;
    }
    
    inline std::vector<std::string> RenderManager::GetViewportNames() const {
        std::vector<std::string> names;
        names.reserve(_viewports.size());
        for (const auto& [name, viewport] : _viewports) {
            names.push_back(name);
        }
        return names;
    }

} // namespace Drift::Renderer 