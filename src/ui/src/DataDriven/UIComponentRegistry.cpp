#include "Drift/UI/DataDriven/UIComponentRegistry.h"
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