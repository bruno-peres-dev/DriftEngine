#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Panel.h"
#include <memory>
#include <iostream>

using namespace Drift;

int main() {
    UI::UIContext ctx;
    ctx.Initialize();
    ctx.SetScreenSize(100.0f, 100.0f);

    // Parent with absolute layout
    auto parent = std::make_shared<UI::Panel>(&ctx);
    UI::LayoutProperties parentLayout;
    parentLayout.layoutType = UI::LayoutType::Absolute;
    parent->SetLayoutProperties(parentLayout);
    parent->SetSize({100.0f, 100.0f});
    ctx.GetRoot()->AddChild(parent);

    // Child stretched with excessive margins
    auto child = std::make_shared<UI::Panel>(&ctx);
    UI::LayoutProperties childLayout;
    childLayout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Stretch;
    childLayout.verticalAlign = UI::LayoutProperties::VerticalAlign::Stretch;
    childLayout.margin = UI::LayoutMargins(80.0f).ToVec4();
    child->SetLayoutProperties(childLayout);
    child->SetSize({10.0f, 10.0f});
    parent->AddChild(child);

    ctx.Update(0.0f);

    auto size = child->GetSize();
    if (size.x < 0.0f || size.y < 0.0f) {
        std::cerr << "Negative size after layout: " << size.x << ", " << size.y << std::endl;
        return 1;
    }

    std::cout << "Absolute layout clamps negative sizes." << std::endl;
    return 0;
}
