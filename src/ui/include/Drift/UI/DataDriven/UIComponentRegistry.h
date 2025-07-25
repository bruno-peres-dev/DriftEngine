#pragma once

#include "Drift/UI/UIElement.h"
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>

namespace Drift::UI {

// Factory function para criar widgets
using WidgetFactory = std::function<std::shared_ptr<UIElement>(UIContext*)>;

// Registro de componentes para criação data-driven
class UIComponentRegistry {
public:
    static UIComponentRegistry& GetInstance();
    
    // Registra um novo tipo de widget
    void RegisterWidget(const std::string& typeName, WidgetFactory factory);
    
    // Registra todos os widgets padrão
    void RegisterDefaultWidgets();
    
    // Cria um widget pelo nome
    std::shared_ptr<UIElement> CreateWidget(const std::string& typeName, UIContext* context);
    
    // Verifica se um tipo está registrado
    bool IsWidgetTypeRegistered(const std::string& typeName) const;
    
    // Lista todos os tipos registrados
    std::vector<std::string> GetRegisteredTypes() const;

private:
    UIComponentRegistry() = default;
    ~UIComponentRegistry() = default;
    UIComponentRegistry(const UIComponentRegistry&) = delete;
    UIComponentRegistry& operator=(const UIComponentRegistry&) = delete;
    
    std::unordered_map<std::string, WidgetFactory> m_Factories;
};

// Macro para facilitar o registro de widgets
#define REGISTER_UI_WIDGET(TypeName, WidgetClass) \
    static bool s_##WidgetClass##_Registered = []() { \
        UIComponentRegistry::GetInstance().RegisterWidget(TypeName, \
            [](UIContext* context) -> std::shared_ptr<UIElement> { \
                return std::make_shared<WidgetClass>(context); \
            }); \
        return true; \
    }();

} // namespace Drift::UI 