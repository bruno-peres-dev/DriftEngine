#pragma once

#include "Drift/UI/UIElement.h"
#include <unordered_set>
#include <vector>

namespace Drift::UI {

// Sistema de otimização de layout com dirty flags
class DirtyFlagSystem {
public:
    // Marca um elemento como "sujo" (precisa recalcular layout)
    static void MarkDirty(UIElement* element);
    
    // Marca um elemento e todos os seus filhos como "sujos"
    static void MarkDirtyRecursive(UIElement* element);
    
    // Recalcula apenas os elementos marcados como "sujos"
    static void RecalculateOnlyDirty(UIElement* root);
    
    // Limpa todas as flags dirty
    static void ClearAllDirtyFlags(UIElement* root);
    
    // Verifica se um elemento está "sujo"
    static bool IsDirty(UIElement* element);
    
    // Obtém estatísticas do sistema
    struct Stats {
        size_t totalElements{0};
        size_t dirtyElements{0};
        size_t recalculationsThisFrame{0};
        float averageRecalculationTime{0.0f};
    };
    
    static Stats GetStats();

private:
    static std::unordered_set<UIElement*> s_DirtyElements;
    static Stats s_Stats;
    
    // Métodos internos
    static void CollectDirtyElements(UIElement* root, std::vector<UIElement*>& dirtyList);
    static void RecalculateElement(UIElement* element);
};

} // namespace Drift::UI 