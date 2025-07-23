#include "Drift/UI/LayoutEngine/DirtyFlagSystem.h"
#include "Drift/UI/LayoutEngine/FlexLayout.h"
#include "Drift/Core/Log.h"
#include <chrono>

using namespace Drift::UI;

// Variáveis estáticas
std::unordered_set<UIElement*> DirtyFlagSystem::s_DirtyElements;
DirtyFlagSystem::Stats DirtyFlagSystem::s_Stats;

void DirtyFlagSystem::MarkDirty(UIElement* element) {
    if (!element) return;
    
    s_DirtyElements.insert(element);
    s_Stats.dirtyElements = s_DirtyElements.size();
    
    // Marca também o pai para recalcular layout
    if (element->GetParent()) {
        s_DirtyElements.insert(element->GetParent());
    }
}

void DirtyFlagSystem::MarkDirtyRecursive(UIElement* element) {
    if (!element) return;
    
    // Marca o elemento atual
    MarkDirty(element);
    
    // Marca todos os filhos recursivamente
    for (auto& child : element->GetChildren()) {
        MarkDirtyRecursive(child.get());
    }
}

void DirtyFlagSystem::RecalculateOnlyDirty(UIElement* root) {
    if (!root) return;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    s_Stats.recalculationsThisFrame = 0;
    
    // Coleta todos os elementos "sujos" em ordem hierárquica
    std::vector<UIElement*> dirtyList;
    CollectDirtyElements(root, dirtyList);
    
    // Recalcula apenas os elementos "sujos"
    for (auto* element : dirtyList) {
        if (s_DirtyElements.find(element) != s_DirtyElements.end()) {
            RecalculateElement(element);
            s_Stats.recalculationsThisFrame++;
        }
    }
    
    // Limpa as flags dirty
    s_DirtyElements.clear();
    s_Stats.dirtyElements = 0;
    
    // Calcula tempo médio
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    if (s_Stats.recalculationsThisFrame > 0) {
        s_Stats.averageRecalculationTime = duration.count() / 1000.0f; // em ms
    }
}

void DirtyFlagSystem::ClearAllDirtyFlags(UIElement* root) {
    if (!root) return;
    
    s_DirtyElements.clear();
    s_Stats.dirtyElements = 0;
    
    // Limpa recursivamente
    for (auto& child : root->GetChildren()) {
        ClearAllDirtyFlags(child.get());
    }
}

bool DirtyFlagSystem::IsDirty(UIElement* element) {
    return element && s_DirtyElements.find(element) != s_DirtyElements.end();
}

DirtyFlagSystem::Stats DirtyFlagSystem::GetStats() {
    return s_Stats;
}

void DirtyFlagSystem::CollectDirtyElements(UIElement* root, std::vector<UIElement*>& dirtyList) {
    if (!root) return;
    
    // Adiciona elementos "sujos" em ordem hierárquica (pais antes dos filhos)
    if (s_DirtyElements.find(root) != s_DirtyElements.end()) {
        dirtyList.push_back(root);
    }
    
    // Processa filhos recursivamente
    for (auto& child : root->GetChildren()) {
        CollectDirtyElements(child.get(), dirtyList);
    }
}

void DirtyFlagSystem::RecalculateElement(UIElement* element) {
    if (!element) return;
    
    // Por enquanto, apenas aplica layout flex se o elemento tiver propriedades flex
    // TODO: Implementar detecção automática de tipo de layout
    
    // Exemplo: se o elemento tem filhos e pode ser um container flex
    if (!element->GetChildren().empty()) {
        // TODO: Carregar FlexProperties do elemento
        FlexProperties flexProps;
        flexProps.direction = FlexProperties::Direction::Row;
        flexProps.justifyContent = FlexProperties::JustifyContent::FlexStart;
        flexProps.alignItems = FlexProperties::AlignItems::Stretch;
        
        // Aplica layout flex
        FlexLayoutEngine::LayoutFlexContainer(element, flexProps);
    }
    
    // Marca o elemento como não "sujo" mais
    s_DirtyElements.erase(element);
} 