#pragma once
#include <string>

// Forward declaration para evitar dependência circular
typedef long HRESULT;
#include <iostream>

namespace Drift::Core {

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3
};

// Variável global para controlar o nível mínimo de log
extern LogLevel g_LogLevel;

// Função para configurar o nível de log
void SetLogLevel(LogLevel level);

// Funções de logging com níveis
void Log(LogLevel level, const std::string& msg);
void LogDebug(const std::string& msg);
void LogInfo(const std::string& msg);
void LogWarning(const std::string& msg);
void LogError(const std::string& msg);

// Função original para compatibilidade
void Log(const std::string& msg);

// Funções específicas para RHI debugging
void LogRHI(const std::string& msg);
void LogRHIError(const std::string& msg);
void LogRHIDebug(const std::string& msg);

// Função para log de exceções com stack trace
void LogException(const std::string& context, const std::exception& e);

// Função para verificar HRESULT do DirectX
void LogHRESULT(const std::string& context, HRESULT hr);

} // namespace Drift::Core

// Macros de logging simples que respeitam o nível configurado
#define LOG_DEBUG(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Debug) Drift::Core::LogDebug(msg)
#define LOG_INFO(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Info) Drift::Core::LogInfo(msg)
#define LOG_WARNING(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Warning) Drift::Core::LogWarning(msg)
#define LOG_ERROR(msg) if (Drift::Core::g_LogLevel <= Drift::Core::LogLevel::Error) Drift::Core::LogError(msg)
