#pragma once

// Sistema de UI completo do DriftEngine
// Inclui todos os componentes principais

// Core UI
#include "Drift/UI/UIContext.h"
#include "Drift/UI/UIElement.h"
#include "Drift/UI/EventBus.h"
#include "Drift/UI/LayoutEngine.h"
#include "Drift/UI/LayoutTypes.h"

// Input
#include "Drift/UI/UIInputHandler.h"

// Widgets
#include "Drift/UI/Widgets/Button.h"

// Data-Driven
#include "Drift/UI/DataDriven/UIComponentRegistry.h"
#include "Drift/UI/DataDriven/UIXMLParser.h"

// Styling
#include "Drift/UI/Styling/StyleSheet.h"

namespace Drift::UI {
    // Namespace principal para todos os componentes de UI
    // Todos os componentes estão disponíveis através deste namespace
} 