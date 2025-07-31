#include "Drift/UI/FontSystem/FontSystem.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Profiler.h"

namespace Drift::UI {

// Variáveis globais do sistema
static FontSystemConfig s_GlobalConfig;
static bool s_Initialized = false;
static Drift::RHI::IDevice* s_Device = nullptr;

void InitializeFontSystem(const FontSystemConfig& config) {
    DRIFT_PROFILE_FUNCTION();
    
    if (s_Initialized) {
        DRIFT_LOG_WARNING("FontSystem já foi inicializado");
        return;
    }
    
    DRIFT_LOG_INFO("Inicializando FontSystem...");
    
    // Configuração global
    s_GlobalConfig = config;
    
    // Inicializar FontManager
    auto& fontManager = FontManager::GetInstance();
    fontManager.Initialize(config);
    
    // Configurar fontes de fallback padrão se não especificadas
    if (s_GlobalConfig.fallbackFonts.empty()) {
        s_GlobalConfig.fallbackFonts = {
            "fonts/Arial-Regular.ttf",
            "fonts/DejaVuSans.ttf",
            "fonts/LiberationSans-Regular.ttf"
        };
    }
    
    // Registrar fontes de fallback
    for (const auto& fallbackPath : s_GlobalConfig.fallbackFonts) {
        fontManager.RegisterFallbackFont(fallbackPath, "fallback");
    }
    
    s_Initialized = true;
    DRIFT_LOG_INFO("FontSystem inicializado com sucesso");
}

void ShutdownFontSystem() {
    DRIFT_PROFILE_FUNCTION();
    
    if (!s_Initialized) {
        DRIFT_LOG_WARNING("FontSystem não foi inicializado");
        return;
    }
    
    DRIFT_LOG_INFO("Finalizando FontSystem...");
    
    // Finalizar FontManager
    auto& fontManager = FontManager::GetInstance();
    fontManager.Shutdown();
    
    s_Initialized = false;
    s_Device = nullptr;
    
    DRIFT_LOG_INFO("FontSystem finalizado");
}

const FontSystemConfig& GetFontSystemConfig() {
    return s_GlobalConfig;
}

void SetFontSystemConfig(const FontSystemConfig& config) {
    s_GlobalConfig = config;
    
    if (s_Initialized) {
        auto& fontManager = FontManager::GetInstance();
        fontManager.SetConfig(config);
    }
}

bool IsFontSystemInitialized() {
    return s_Initialized;
}

void SetFontSystemDevice(Drift::RHI::IDevice* device) {
    s_Device = device;
    
    if (s_Initialized) {
        auto& fontManager = FontManager::GetInstance();
        fontManager.SetDevice(device);
    }
}

Drift::RHI::IDevice* GetFontSystemDevice() {
    return s_Device;
}

} // namespace Drift::UI 