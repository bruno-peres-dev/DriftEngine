#pragma once

// Sistema de UI completo do DriftEngine
// Inclui todos os componentes principais

// Core UI
#include "Drift/UI/UIContext.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/LayoutTypes.h"

// Input
#include "Drift/UI/UIInputHandler.h"

// Widgets
#include "Drift/UI/Widgets/Button.h"
#include "Drift/UI/Widgets/Label.h"
#include "Drift/UI/Widgets/Panel.h"
#include "Drift/UI/Widgets/Image.h"

// Data-Driven
#include "Drift/UI/DataDriven/UIComponentRegistry.h"

// Font System
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/FontRendering.h"

namespace Drift::UI {
    // Namespace principal para todos os componentes de UI
    // Todos os componentes estão disponíveis através deste namespace
    
    // Alias para facilitar acesso ao sistema de fontes
    using FontManager = Drift::UI::FontManager;
    using TextRenderer = Drift::UI::FontRendering;
} 