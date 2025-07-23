#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/UI/UIContext.h"
#include "Drift/UI/DataDriven/UIComponentRegistry.h"
#include "Drift/UI/Styling/StyleSheet.h"
#include <string>
#include <memory>
#include <vector>

namespace Drift::UI {

// Atributos de um elemento UXML
struct UXMLAttributes {
    std::unordered_map<std::string, std::string> attributes;
    
    // Getters tipados
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;
    int GetInt(const std::string& key, int defaultValue = 0) const;
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const;
    bool GetBool(const std::string& key, bool defaultValue = false) const;
    glm::vec2 GetVec2(const std::string& key, const glm::vec2& defaultValue = glm::vec2(0.0f)) const;
    glm::vec4 GetVec4(const std::string& key, const glm::vec4& defaultValue = glm::vec4(0.0f)) const;
};

// Nó da árvore UXML
struct UXMLNode {
    std::string elementType;
    UXMLAttributes attributes;
    std::vector<std::unique_ptr<UXMLNode>> children;
    
    UXMLNode(const std::string& type) : elementType(type) {}
};

// Parser UXML para criar hierarquias de UI
class UXMLParser {
public:
    UXMLParser(UIContext* context);
    ~UXMLParser() = default;
    
    // Carrega e cria uma hierarquia de UI a partir de um arquivo UXML
    std::shared_ptr<UIElement> LoadFromFile(const std::string& filename);
    
    // Carrega e cria uma hierarquia de UI a partir de uma string UXML
    std::shared_ptr<UIElement> LoadFromString(const std::string& uxml);
    
    // Define a folha de estilos a ser usada
    void SetStyleSheet(std::shared_ptr<StyleSheet> styleSheet);
    
    // Obtém a folha de estilos atual
    std::shared_ptr<StyleSheet> GetStyleSheet() const { return m_StyleSheet; }

private:
    UIContext* m_Context;
    std::shared_ptr<StyleSheet> m_StyleSheet;
    
    // Parsing
    std::unique_ptr<UXMLNode> ParseUXML(const std::string& uxml);
    std::unique_ptr<UXMLNode> ParseNode(const std::string& nodeStr);
    UXMLAttributes ParseAttributes(const std::string& attributesStr);
    
    // Criação de elementos
    std::shared_ptr<UIElement> CreateElementFromNode(const UXMLNode* node);
    void ApplyAttributesToElement(UIElement* element, const UXMLAttributes& attributes);
    void ApplyStylesToElement(UIElement* element, const UXMLAttributes& attributes);
    
    // Helpers
    std::string Trim(const std::string& str);
    std::vector<std::string> Split(const std::string& str, char delimiter);
    bool StartsWith(const std::string& str, const std::string& prefix);
    bool EndsWith(const std::string& str, const std::string& suffix);
};

// Exemplo de uso:
/*
UXML:
<panel class="main-menu" id="root">
    <button class="primary" text="Play Game" />
    <button class="secondary" text="Settings" />
    <button class="danger" text="Quit" />
</panel>

CSS:
.main-menu {
    background-color: #2a2a2a;
    padding: 20px;
    display: flex;
    flex-direction: column;
    gap: 10px;
}

.button.primary {
    background-color: #4CAF50;
    color: white;
    padding: 10px 20px;
}

.button.secondary {
    background-color: #2196F3;
    color: white;
    padding: 10px 20px;
}

.button.danger {
    background-color: #F44336;
    color: white;
    padding: 10px 20px;
}
*/

} // namespace Drift::UI 