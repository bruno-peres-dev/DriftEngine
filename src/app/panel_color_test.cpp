#include "Drift/UI/UIContext.h"
#include "Drift/UI/Widgets/Panel.h"
#include <memory>
#include <iostream>

using namespace Drift;

int main() {
    UI::UIContext ctx;
    ctx.Initialize();

    auto panel = std::make_shared<UI::Panel>(&ctx);
    unsigned newColor = 0xFF123456;
    panel->SetBackgroundColor(newColor);

    if (panel->GetRenderColor() != newColor) {
        std::cerr << "Background color not updated!" << std::endl;
        return 1;
    }

    std::cout << "Background color update success." << std::endl;
    return 0;
}
