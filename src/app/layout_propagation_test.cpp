#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Panel.h"
#include <memory>
#include <iostream>

using namespace Drift;

int main() {
    UI::UIContext ctx;
    ctx.Initialize();

    auto parent = std::make_shared<UI::Panel>(&ctx);
    UI::LayoutProperties layout;
    layout.layoutType = UI::LayoutType::None;
    parent->SetLayoutProperties(layout);
    ctx.GetRoot()->AddChild(parent);

    // Clear initial dirty flags
    ctx.Update(0.0f);
    if (ctx.GetRoot()->IsLayoutDirty()) {
        std::cerr << "Root still dirty after initial update" << std::endl;
        return 1;
    }

    auto child = std::make_shared<UI::Panel>(&ctx);
    child->SetLayoutProperties(layout);
    parent->AddChild(child);
    ctx.Update(0.0f);

    // Change child layout to trigger propagation
    layout.horizontalAlign = UI::LayoutProperties::HorizontalAlign::Center;
    child->SetLayoutProperties(layout);

    if (!ctx.GetRoot()->IsLayoutDirty()) {
        std::cerr << "Root not marked dirty after child layout change" << std::endl;
        return 1;
    }

    ctx.Update(0.0f);

    if (ctx.GetRoot()->IsLayoutDirty()) {
        std::cerr << "Root still dirty after update" << std::endl;
        return 1;
    }

    std::cout << "Layout dirty propagation success." << std::endl;
    return 0;
}
