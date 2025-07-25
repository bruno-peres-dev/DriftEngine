#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Panel.h"
#include <memory>
#include <iostream>

using namespace Drift;

int main() {
    UI::UIContext ctx;
    ctx.Initialize();
    ctx.SetScreenSize(200.0f, 200.0f);

    auto parent = std::make_shared<UI::Panel>(&ctx);
    parent->SetSize({100.0f, 100.0f});
    UI::LayoutProperties layout;
    layout.layoutType = UI::LayoutType::None;
    layout.clipContent = true;
    parent->SetLayoutProperties(layout);
    ctx.GetRoot()->AddChild(parent);

    auto child = std::make_shared<UI::Panel>(&ctx);
    child->SetSize({50.0f, 50.0f});
    child->SetPosition({110.0f, 10.0f});
    child->SetLayoutProperties(layout);
    parent->AddChild(child);

    ctx.Update(0.0f);

    UI::UIElement* hit = ctx.HitTest({120.0f, 20.0f});
    if (hit != ctx.GetRoot().get()) {
        std::cerr << "HitTest ignored clipping" << std::endl;
        return 1;
    }

    std::cout << "HitTest respects clipping." << std::endl;
    return 0;
}
