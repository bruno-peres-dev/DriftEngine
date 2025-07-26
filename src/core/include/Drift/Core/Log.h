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

// Variável global para controlar o nível mínimo de log
extern LogLevel g_LogLevel;

// Função para configurar o nível de log
void SetLogLevel(LogLevel level);

// Função original para compatibilidade com código legado
void Log(const std::string& msg);

// Novas funções de logging com níveis
void Log(LogLevel level, const std::string& msg);
void LogDebug(const std::string& msg);
void LogInfo(const std::string& msg);
void LogWarning(const std::string& msg);
void LogError(const std::string& msg);

} // namespace Drift::Core

// Macros de logging simples que respeitam o nível configurado
#define LOG_DEBUG(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Debug) Drift::Core::LogDebug(msg)
#define LOG_INFO(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Info) Drift::Core::LogInfo(msg)
#define LOG_WARNING(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Warning) Drift::Core::LogWarning(msg)
#define LOG_ERROR(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Error) Drift::Core::LogError(msg)
