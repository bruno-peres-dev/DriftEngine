#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

namespace Drift::UI {

// Propriedades de estilo CSS-like
struct StyleProperties {
    // Cores
    glm::vec4 backgroundColor{0.2f, 0.2f, 0.2f, 1.0f};
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 borderColor{0.5f, 0.5f, 0.5f, 1.0f};
    
    // Dimensões
    glm::vec2 size{100.0f, 30.0f};
    glm::vec2 minSize{0.0f, 0.0f};
    glm::vec2 maxSize{10000.0f, 10000.0f};
    
    // Margens e padding
    float marginLeft{0.0f}, marginTop{0.0f}, marginRight{0.0f}, marginBottom{0.0f};
    float paddingLeft{5.0f}, paddingTop{5.0f}, paddingRight{5.0f}, paddingBottom{5.0f};
    
    // Bordas
    float borderWidth{1.0f};
    float borderRadius{0.0f};
    
    // Tipografia
    std::string fontFamily{"Arial"};
    float fontSize{14.0f};
    std::string fontWeight{"normal"}; // normal, bold
    std::string textAlign{"left"}; // left, center, right
    
    // Layout
    std::string display{"block"}; // block, inline, flex, grid
    std::string flexDirection{"row"}; // row, column
    std::string justifyContent{"flex-start"}; // flex-start, center, flex-end, space-between
    std::string alignItems{"stretch"}; // stretch, center, flex-start, flex-end
    
    // Estados
    glm::vec4 hoverBackgroundColor{0.3f, 0.3f, 0.3f, 1.0f};
    glm::vec4 pressedBackgroundColor{0.1f, 0.1f, 0.1f, 1.0f};
    glm::vec4 disabledBackgroundColor{0.1f, 0.1f, 0.1f, 0.5f};
    
    // Animações
    float transitionDuration{0.2f};
    std::string transitionEasing{"ease"}; // linear, ease, ease-in, ease-out, ease-in-out
    
    // Visibilidade
    bool visible{true};
    float opacity{1.0f};
    
    // Z-index para ordenação
    int zIndex{0};
};

// Seletor CSS-like
struct StyleSelector {
    std::string elementType; // "button", "panel", etc.
    std::string className;   // ".primary", ".secondary", etc.
    std::string id;          // "#main-menu", "#settings", etc.
    std::string pseudoClass; // ":hover", ":pressed", ":disabled", etc.
    
    // Especificidade do seletor (maior = mais específico)
    int GetSpecificity() const;
    
    // Verifica se o seletor corresponde a um elemento
    bool Matches(const std::string& elementType, const std::string& className, 
                 const std::string& id, const std::string& state) const;
};

// Regra de estilo
struct StyleRule {
    StyleSelector selector;
    StyleProperties properties;
    int specificity;
    
    StyleRule(const StyleSelector& sel, const StyleProperties& props)
        : selector(sel), properties(props), specificity(sel.GetSpecificity()) {}
};

// Folha de estilos
class StyleSheet {
public:
    StyleSheet() = default;
    ~StyleSheet() = default;
    
    // Adiciona uma regra de estilo
    void AddRule(const StyleRule& rule);
    
    // Aplica estilos a um elemento
    StyleProperties GetComputedStyles(const std::string& elementType, 
                                     const std::string& className,
                                     const std::string& id,
                                     const std::string& state) const;
    
    // Carrega estilos de um arquivo CSS
    bool LoadFromFile(const std::string& filename);
    
    // Carrega estilos de uma string CSS
    bool LoadFromString(const std::string& css);
    
    // Limpa todas as regras
    void Clear();

private:
    std::vector<StyleRule> m_Rules;
    
    // Parser CSS simples
    bool ParseCSS(const std::string& css);
    StyleSelector ParseSelector(const std::string& selectorStr);
    StyleProperties ParseProperties(const std::string& propertiesStr);
};

} // namespace Drift::UI 