#include "Drift/Core/Log.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <d3d11.h>
#endif
#include <filesystem>
#include <thread>
#include <algorithm>
#include <chrono>
#include <ctime>

#ifndef _WIN32
inline bool FAILED(long hr) { return hr < 0; }
#endif

namespace Drift {
namespace Core {

// Forward declaration of helper
std::string GetTimestamp();
// Implementação do ConsoleLogOutput
void ConsoleLogOutput::Write(LogLevel level, const std::string& message) {
    std::cout << message << std::endl;
}

// Implementação do FileLogOutput
FileLogOutput::FileLogOutput(const std::string& filename) : m_Filename(filename) {
    m_File.open(filename, std::ios::app);
    if (m_File.is_open()) {
        m_File << "\n=== Log iniciado em " << Drift::Core::GetTimestamp() << " ===" << std::endl;
    }
}

FileLogOutput::~FileLogOutput() {
    if (m_File.is_open()) {
        m_File.close();
    }
}

void FileLogOutput::Write(LogLevel level, const std::string& message) {
    if (m_File.is_open()) {
        m_File << message << std::endl;
        m_File.flush();
    }
}

// Implementação do LogSystem
LogSystem& LogSystem::GetInstance() {
    static LogSystem instance;
    return instance;
}

void LogSystem::Configure(const LogConfig& config) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Config = config;
    
    // Adicionar output padrão se não houver nenhum
    if (m_Outputs.empty()) {
        m_Outputs.push_back(std::make_shared<ConsoleLogOutput>());
    }
    
    // Adicionar output de arquivo se especificado
    if (!config.outputFile.empty()) {
        m_Outputs.push_back(std::make_shared<FileLogOutput>(config.outputFile));
    }
}

void LogSystem::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Config.minLevel = level;
}

void LogSystem::AddOutput(std::shared_ptr<ILogOutput> output) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Outputs.push_back(output);
}

void LogSystem::RemoveOutput(std::shared_ptr<ILogOutput> output) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Outputs.erase(
        std::remove(m_Outputs.begin(), m_Outputs.end(), output),
        m_Outputs.end()
    );
}

void LogSystem::Log(LogLevel level, const std::string& message) {
    if (level < m_Config.minLevel) {
        return;
    }
    
    std::string formattedMessage = FormatLogMessage(level, nullptr, 0, nullptr, message);
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& output : m_Outputs) {
        output->Write(level, formattedMessage);
    }
    
    if (m_Config.customOutput) {
        m_Config.customOutput(level, formattedMessage);
    }
}

void LogSystem::Log(LogLevel level, const char* file, int line, const char* function, const std::string& message) {
    if (level < m_Config.minLevel) {
        return;
    }
    
    std::string formattedMessage = FormatLogMessage(level, file, line, function, message);
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& output : m_Outputs) {
        output->Write(level, formattedMessage);
    }
    
    if (m_Config.customOutput) {
        m_Config.customOutput(level, formattedMessage);
    }
}

void LogSystem::LogTrace(const std::string& message) {
    Log(LogLevel::Trace, message);
}

void LogSystem::LogDebug(const std::string& message) {
    Log(LogLevel::Debug, message);
}

void LogSystem::LogInfo(const std::string& message) {
    Log(LogLevel::Info, message);
}

void LogSystem::LogWarning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void LogSystem::LogError(const std::string& message) {
    Log(LogLevel::Error, message);
}

void LogSystem::LogFatal(const std::string& message) {
    Log(LogLevel::Fatal, message);
}

void LogSystem::LogTrace(const char* file, int line, const char* function, const std::string& message) {
    Log(LogLevel::Trace, file, line, function, message);
}

void LogSystem::LogDebug(const char* file, int line, const char* function, const std::string& message) {
    Log(LogLevel::Debug, file, line, function, message);
}

void LogSystem::LogInfo(const char* file, int line, const char* function, const std::string& message) {
    Log(LogLevel::Info, file, line, function, message);
}

void LogSystem::LogWarning(const char* file, int line, const char* function, const std::string& message) {
    Log(LogLevel::Warning, file, line, function, message);
}

void LogSystem::LogError(const char* file, int line, const char* function, const std::string& message) {
    Log(LogLevel::Error, file, line, function, message);
}

void LogSystem::LogFatal(const char* file, int line, const char* function, const std::string& message) {
    Log(LogLevel::Fatal, file, line, function, message);
}

void LogSystem::LogRHI(const std::string& message) {
    std::string formattedMessage = "[RHI] " + message;
    Log(LogLevel::Info, formattedMessage);
}

void LogSystem::LogRHIError(const std::string& message) {
    std::string formattedMessage = "[RHI][ERROR] " + message;
    Log(LogLevel::Error, formattedMessage);
}

void LogSystem::LogRHIDebug(const std::string& message) {
    std::string formattedMessage = "[RHI][DEBUG] " + message;
    Log(LogLevel::Debug, formattedMessage);
}

void LogSystem::LogException(const std::string& context, const std::exception& e) {
    std::stringstream ss;
    ss << "[EXCEPTION][" << context << "] " << e.what();
    LogError(ss.str());
}

void LogSystem::LogHRESULT(const std::string& context, HRESULT hr) {
#ifdef _WIN32
    if (FAILED(hr)) {
        std::stringstream ss;
        ss << "[HRESULT][" << context << "] 0x" << std::hex << std::setw(8) << std::setfill('0') << hr;

        // Adicionar descrição comum do HRESULT
        switch (hr) {
            case E_INVALIDARG:
                ss << " (E_INVALIDARG - Argumento inválido)";
                break;
            case E_OUTOFMEMORY:
                ss << " (E_OUTOFMEMORY - Memória insuficiente)";
                break;
            case E_NOTIMPL:
                ss << " (E_NOTIMPL - Não implementado)";
                break;
            case E_FAIL:
                ss << " (E_FAIL - Falha genérica)";
                break;
            case DXGI_ERROR_DEVICE_REMOVED:
                ss << " (DXGI_ERROR_DEVICE_REMOVED - Dispositivo removido)";
                break;
            case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
                ss << " (DXGI_ERROR_DRIVER_INTERNAL_ERROR - Erro interno do driver)";
                break;
            case DXGI_ERROR_INVALID_CALL:
                ss << " (DXGI_ERROR_INVALID_CALL - Chamada inválida)";
                break;
            case D3D11_ERROR_FILE_NOT_FOUND:
                ss << " (D3D11_ERROR_FILE_NOT_FOUND - Arquivo não encontrado)";
                break;
            case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
                ss << " (D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS - Muitos objetos de estado únicos)";
                break;
            case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
                ss << " (D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS - Muitos objetos de view únicos)";
                break;
            case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
                ss << " (D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD - Map sem discard inicial)";
                break;
            default:
                ss << " (HRESULT desconhecido)";
                break;
        }

        LogRHIError(ss.str());
    } else {
        LogRHIDebug("[HRESULT][" + context + "] Sucesso (0x" + std::to_string(hr) + ")");
    }
#else
    std::stringstream ss;
    ss << "[HRESULT][" << context << "] 0x" << std::hex << std::setw(8) << std::setfill('0') << hr;
    LogRHIError(ss.str());
#endif
}

void LogSystem::Log(const std::string& message) {
    Log(LogLevel::Info, message);
}

std::string LogSystem::FormatLogMessage(LogLevel level, const char* file, int line, const char* function, const std::string& message) {
    std::stringstream ss;
    
    // Timestamp
    if (m_Config.enableTimestamps) {
        ss << "[" << Drift::Core::GetTimestamp() << "] ";
    }
    
    // Nível de log
    ss << "[" << GetLevelString(level) << "] ";
    
    // Informações de thread
    if (m_Config.enableThreadInfo) {
        ss << "[" << GetThreadInfo() << "] ";
    }
    
    // Informações de arquivo/linha
    if (m_Config.enableFileInfo && file && line > 0) {
        ss << "[" << GetFileInfo(file, line, function) << "] ";
    }
    
    // Mensagem
    ss << message;
    
    return ss.str();
}

std::string LogSystem::GetLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "UNKNOWN";
    }
}

// Função utilitária global para timestamp
std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string LogSystem::GetThreadInfo() {
    std::stringstream ss;
    ss << "TID:" << std::this_thread::get_id();
    return ss.str();
}

std::string LogSystem::GetFileInfo(const char* file, int line, const char* function) {
    std::stringstream ss;
    
    if (file) {
        std::filesystem::path path(file);
        ss << path.filename().string();
    }
    
    if (line > 0) {
        ss << ":" << line;
    }
    
    if (function) {
        ss << ":" << function;
    }
    
    return ss.str();
}

// Instância global do sistema de log
LogSystem& g_LogSystem = LogSystem::GetInstance();

} // namespace Drift::Core
} // namespace Drift
