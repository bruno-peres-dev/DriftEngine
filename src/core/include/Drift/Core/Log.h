#pragma once
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>

// Forward declaration para evitar dependência circular
typedef long HRESULT;

namespace Drift {
namespace Core {

enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

// Configuração do sistema de log
struct LogConfig {
    LogLevel minLevel = LogLevel::Info;
    bool enableTimestamps = true;
    bool enableThreadInfo = false;
    bool enableFileInfo = false;
    std::string outputFile = "";
    std::function<void(LogLevel, const std::string&)> customOutput = nullptr;
};

// Interface para output de log
class ILogOutput {
public:
    virtual ~ILogOutput() = default;
    virtual void Write(LogLevel level, const std::string& message) = 0;
};

// Output padrão para console
class ConsoleLogOutput : public ILogOutput {
public:
    void Write(LogLevel level, const std::string& message) override;
};

// Output para arquivo
class FileLogOutput : public ILogOutput {
public:
    explicit FileLogOutput(const std::string& filename);
    ~FileLogOutput();
    void Write(LogLevel level, const std::string& message) override;

private:
    std::string m_Filename;
    std::ofstream m_File;
};

// Sistema de log principal
class LogSystem {
public:
    static LogSystem& GetInstance();
    
    // Configuração
    void Configure(const LogConfig& config);
    void SetLogLevel(LogLevel level);
    void AddOutput(std::shared_ptr<ILogOutput> output);
    void RemoveOutput(std::shared_ptr<ILogOutput> output);
    
    // Funções de logging
    void Log(LogLevel level, const std::string& message);
    void Log(LogLevel level, const char* file, int line, const char* function, const std::string& message);
    
    // Funções específicas por nível
    void LogTrace(const std::string& message);
    void LogDebug(const std::string& message);
    void LogInfo(const std::string& message);
    void LogWarning(const std::string& message);
    void LogError(const std::string& message);
    void LogFatal(const std::string& message);
    
    // Funções com contexto de arquivo/linha
    void LogTrace(const char* file, int line, const char* function, const std::string& message);
    void LogDebug(const char* file, int line, const char* function, const std::string& message);
    void LogInfo(const char* file, int line, const char* function, const std::string& message);
    void LogWarning(const char* file, int line, const char* function, const std::string& message);
    void LogError(const char* file, int line, const char* function, const std::string& message);
    void LogFatal(const char* file, int line, const char* function, const std::string& message);
    
    // Funções específicas para RHI
    void LogRHI(const std::string& message);
    void LogRHIError(const std::string& message);
    void LogRHIDebug(const std::string& message);
    
    // Funções utilitárias
    void LogException(const std::string& context, const std::exception& e);
    void LogHRESULT(const std::string& context, HRESULT hr);
    
    // Função para compatibilidade com código legado
    void Log(const std::string& message);

private:
    LogSystem() = default;
    ~LogSystem() = default;
    LogSystem(const LogSystem&) = delete;
    LogSystem& operator=(const LogSystem&) = delete;
    
    std::string FormatLogMessage(LogLevel level, const char* file, int line, const char* function, const std::string& message);
    std::string GetLevelString(LogLevel level);
    std::string GetThreadInfo();
    std::string GetFileInfo(const char* file, int line, const char* function);
    
    LogConfig m_Config;
    std::vector<std::shared_ptr<ILogOutput>> m_Outputs;
    std::mutex m_Mutex;
};

// Funções globais para compatibilidade
extern LogSystem& g_LogSystem;

// Funções de conveniência
inline void SetLogLevel(LogLevel level) { g_LogSystem.SetLogLevel(level); }
inline void Log(LogLevel level, const std::string& msg) { g_LogSystem.Log(level, msg); }
inline void LogTrace(const std::string& msg) { g_LogSystem.LogTrace(msg); }
inline void LogDebug(const std::string& msg) { g_LogSystem.LogDebug(msg); }
inline void LogInfo(const std::string& msg) { g_LogSystem.LogInfo(msg); }
inline void LogWarning(const std::string& msg) { g_LogSystem.LogWarning(msg); }
inline void LogError(const std::string& msg) { g_LogSystem.LogError(msg); }
inline void LogFatal(const std::string& msg) { g_LogSystem.LogFatal(msg); }
inline void LogRHI(const std::string& msg) { g_LogSystem.LogRHI(msg); }
inline void LogRHIError(const std::string& msg) { g_LogSystem.LogRHIError(msg); }
inline void LogRHIDebug(const std::string& msg) { g_LogSystem.LogRHIDebug(msg); }
inline void LogException(const std::string& context, const std::exception& e) { g_LogSystem.LogException(context, e); }
inline void LogHRESULT(const std::string& context, HRESULT hr) { g_LogSystem.LogHRESULT(context, hr); }

// Função utilitária global para timestamp
std::string GetTimestamp();

} // namespace Drift::Core

// Macros de logging básicas
#define LOG_TRACE(msg) Drift::Core::g_LogSystem.LogTrace(__FILE__, __LINE__, __FUNCTION__, msg)
#define LOG_DEBUG(msg) Drift::Core::g_LogSystem.LogDebug(__FILE__, __LINE__, __FUNCTION__, msg)
#define LOG_INFO(msg) Drift::Core::g_LogSystem.LogInfo(__FILE__, __LINE__, __FUNCTION__, msg)
#define LOG_WARNING(msg) Drift::Core::g_LogSystem.LogWarning(__FILE__, __LINE__, __FUNCTION__, msg)
#define LOG_ERROR(msg) Drift::Core::g_LogSystem.LogError(__FILE__, __LINE__, __FUNCTION__, msg)
#define LOG_FATAL(msg) Drift::Core::g_LogSystem.LogFatal(__FILE__, __LINE__, __FUNCTION__, msg)

// Macros DRIFT_LOG_* para compatibilidade com o sistema de fontes
// Estas macros suportam formatação estilo fmt
#define DRIFT_LOG_TRACE(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    Drift::Core::g_LogSystem.LogTrace(__FILE__, __LINE__, __FUNCTION__, ss.str()); \
} while(0)

#define DRIFT_LOG_DEBUG(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    Drift::Core::g_LogSystem.LogDebug(__FILE__, __LINE__, __FUNCTION__, ss.str()); \
} while(0)

#define DRIFT_LOG_INFO(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    Drift::Core::g_LogSystem.LogInfo(__FILE__, __LINE__, __FUNCTION__, ss.str()); \
} while(0)

#define DRIFT_LOG_WARNING(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    Drift::Core::g_LogSystem.LogWarning(__FILE__, __LINE__, __FUNCTION__, ss.str()); \
} while(0)

#define DRIFT_LOG_ERROR(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    Drift::Core::g_LogSystem.LogError(__FILE__, __LINE__, __FUNCTION__, ss.str()); \
} while(0)

#define DRIFT_LOG_FATAL(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    Drift::Core::g_LogSystem.LogFatal(__FILE__, __LINE__, __FUNCTION__, ss.str()); \
} while(0)

// Macros condicionais que verificam o nível de log antes de executar
#define LOG_TRACE_IF(condition, msg) if (condition) LOG_TRACE(msg)
#define LOG_DEBUG_IF(condition, msg) if (condition) LOG_DEBUG(msg)
#define LOG_INFO_IF(condition, msg) if (condition) LOG_INFO(msg)
#define LOG_WARNING_IF(condition, msg) if (condition) LOG_WARNING(msg)
#define LOG_ERROR_IF(condition, msg) if (condition) LOG_ERROR(msg)
#define LOG_FATAL_IF(condition, msg) if (condition) LOG_FATAL(msg)

// Macros para logging de performance
#define LOG_PERF(msg) LOG_DEBUG("[PERF] " << msg)
#define LOG_PERF_IF(condition, msg) LOG_DEBUG_IF(condition, "[PERF] " << msg)

// Macros para logging de memória
#define LOG_MEM(msg) LOG_DEBUG("[MEM] " << msg)
#define LOG_MEM_IF(condition, msg) LOG_DEBUG_IF(condition, "[MEM] " << msg)

} // namespace Drift
