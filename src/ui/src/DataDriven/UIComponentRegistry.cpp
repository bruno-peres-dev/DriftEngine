#include "Drift/UI/DataDriven/UIComponentRegistry.h"
#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/Widgets/Label.h"
#include "Drift/UI/Widgets/Panel.h"
#include "Drift/UI/Widgets/Image.h"
#include "Drift/UI/Widgets/StackPanel.h"
#include "Drift/UI/Widgets/Grid.h"
#include "Drift/Core/Log.h"
#include <algorithm>

using namespace Drift::UI;

UIComponentRegistry& UIComponentRegistry::GetInstance()
{
    static UIComponentRegistry instance;
    return instance;
}

void UIComponentRegistry::RegisterWidget(const std::string& typeName, WidgetFactory factory)
{
    if (m_Factories.find(typeName) != m_Factories.end()) {
        Core::Log("[UI] Warning: Widget type '" + typeName + "' already registered, overwriting");
    }
    
    m_Factories[typeName] = factory;
    Core::Log("[UI] Registered widget type: " + typeName);
}

void UIComponentRegistry::RegisterDefaultWidgets()
{
    // Register built-in widgets
    RegisterWidget("button", [](UIContext* context) -> std::shared_ptr<UIElement> {
        return std::make_shared<Button>(context);
    });
    
    RegisterWidget("label", [](UIContext* context) -> std::shared_ptr<UIElement> {
        return std::make_shared<Label>(context);
    });
    
    RegisterWidget("panel", [](UIContext* context) -> std::shared_ptr<UIElement> {
        return std::make_shared<Panel>(context);
    });
    
    RegisterWidget("image", [](UIContext* context) -> std::shared_ptr<UIElement> {
        return std::make_shared<Image>(context);
    });
    
    RegisterWidget("stackpanel", [](UIContext* context) -> std::shared_ptr<UIElement> {
        return std::make_shared<StackPanel>(context);
    });
    
    RegisterWidget("grid", [](UIContext* context) -> std::shared_ptr<UIElement> {
        return std::make_shared<Grid>(context);
    });
}

std::shared_ptr<UIElement> UIComponentRegistry::CreateWidget(const std::string& typeName, UIContext* context)
{
    auto it = m_Factories.find(typeName);
    if (it == m_Factories.end()) {
        Core::Log("[UI] Error: Unknown widget type '" + typeName + "'");
        return nullptr;
    }
    
    try {
        return it->second(context);
    } catch (const std::exception& e) {
        Core::Log("[UI] Error creating widget '" + typeName + "': " + e.what());
        return nullptr;
    }
}

bool UIComponentRegistry::IsWidgetTypeRegistered(const std::string& typeName) const
{
    return m_Factories.find(typeName) != m_Factories.end();
}

std::vector<std::string> UIComponentRegistry::GetRegisteredTypes() const
{
    std::vector<std::string> types;
    types.reserve(m_Factories.size());
    
    for (const auto& pair : m_Factories) {
        types.push_back(pair.first);
    }
    
    return types;
} 