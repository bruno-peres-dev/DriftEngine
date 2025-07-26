#include "Drift/Core/Log.h"
#include <iostream>

namespace Drift::Core {

// Variável global para controlar o nível mínimo de log
LogLevel g_LogLevel = LogLevel::Info; // Por padrão, só mostra Warning e Error

// Função para configurar o nível de log
void SetLogLevel(LogLevel level) {
    g_LogLevel = level;
}

// Função original para compatibilidade com código legado
void Log(const std::string& msg) {
    std::cout << "[Drift] " << msg << std::endl;
}

void Log(LogLevel level, const std::string& msg) {
    // Verificar se o nível atual permite este log
    if (level < g_LogLevel) {
        return;
    }
    
    switch (level) {
        case LogLevel::Debug:
            std::cout << "[DEBUG] " << msg << std::endl;
            break;
        case LogLevel::Info:
            std::cout << "[INFO] " << msg << std::endl;
            break;
        case LogLevel::Warning:
            std::cout << "[WARNING] " << msg << std::endl;
            break;
        case LogLevel::Error:
            std::cout << "[ERROR] " << msg << std::endl;
            break;
    }
}

void LogDebug(const std::string& msg) {
    Log(LogLevel::Debug, msg);
}

void LogInfo(const std::string& msg) {
    Log(LogLevel::Info, msg);
}

void LogWarning(const std::string& msg) {
    Log(LogLevel::Warning, msg);
}

void LogError(const std::string& msg) {
    Log(LogLevel::Error, msg);
}

} // namespace Drift::Core
