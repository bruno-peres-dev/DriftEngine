#pragma once
#include <string>
#include <iostream>

namespace Drift::Core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

// Função original para compatibilidade com código legado
void Log(const std::string& msg);

// Novas funções de logging com níveis
void Log(LogLevel level, const std::string& msg);
void LogDebug(const std::string& msg);
void LogInfo(const std::string& msg);
void LogWarning(const std::string& msg);
void LogError(const std::string& msg);

} // namespace Drift::Core

// Macros de logging simples
#define LOG_DEBUG(msg) Drift::Core::LogDebug(msg)
#define LOG_INFO(msg) Drift::Core::LogInfo(msg)
#define LOG_WARNING(msg) Drift::Core::LogWarning(msg)
#define LOG_ERROR(msg) Drift::Core::LogError(msg)
