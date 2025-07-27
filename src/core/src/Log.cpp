#include "Drift/Core/Log.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <d3d11.h>

namespace Drift::Core {

// Variável global para controlar o nível mínimo de log
LogLevel g_LogLevel = LogLevel::Info; // Por padrão, só mostra Info, Warning e Error

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
        case LogLevel::Trace:
            std::cout << "[TRACE] " << msg << std::endl;
            break;
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

void LogTrace(const std::string& msg) {
    Log(LogLevel::Trace, msg);
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

// Funções específicas para RHI debugging
void LogRHI(const std::string& msg) {
    std::cout << "[RHI] " << msg << std::endl;
}

void LogRHIError(const std::string& msg) {
    std::cout << "[RHI][ERROR] " << msg << std::endl;
}

void LogRHIDebug(const std::string& msg) {
    if (g_LogLevel <= LogLevel::Debug) {
        std::cout << "[RHI][DEBUG] " << msg << std::endl;
    }
}

// Função para log de exceções com contexto
void LogException(const std::string& context, const std::exception& e) {
    std::stringstream ss;
    ss << "[EXCEPTION][" << context << "] " << e.what();
    LogError(ss.str());
}

// Função para verificar HRESULT do DirectX
void LogHRESULT(const std::string& context, HRESULT hr) {
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
}

} // namespace Drift::Core
