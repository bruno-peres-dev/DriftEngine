#pragma once
#include "Drift/Core/Log.h"
#include <d3d11.h>
#include <string>

namespace Drift::RHI {

// Classe para debugging e validação do RHI
class RHIDebug {
public:
    // Verifica se um ponteiro é válido
    template<typename T>
    static bool ValidatePointer(const T* ptr, const std::string& context) {
        if (!ptr) {
            Drift::Core::LogRHIError("[" + context + "] Ponteiro é nullptr");
            return false;
        }
        return true;
    }

    // Verifica HRESULT e loga erro se falhar
    static bool ValidateHRESULT(HRESULT hr, const std::string& context) {
        Drift::Core::LogHRESULT(context, hr);
        return SUCCEEDED(hr);
    }

    // Verifica se um recurso DX11 é válido
    static bool ValidateDX11Resource(ID3D11Resource* resource, const std::string& context) {
        if (!ValidatePointer(resource, context)) {
            return false;
        }
        
        // Verifica se o recurso ainda é válido
        ULONG refCount = resource->AddRef();
        resource->Release();
        
        if (refCount == 0) {
            Drift::Core::LogRHIError("[" + context + "] Recurso DX11 inválido (refCount = 0)");
            return false;
        }
        
        return true;
    }

    // Verifica se um device DX11 é válido
    static bool ValidateDX11Device(ID3D11Device* device, const std::string& context) {
        if (!ValidatePointer(device, context)) {
            return false;
        }
        
        // Verifica se o device foi removido
        HRESULT removedReason = device->GetDeviceRemovedReason();
        if (removedReason != S_OK) {
            Drift::Core::LogHRESULT(context + " - DeviceRemovedReason", removedReason);
            return false;
        }
        
        return true;
    }

    // Verifica se um context DX11 é válido
    static bool ValidateDX11Context(ID3D11DeviceContext* context, const std::string& contextName) {
        if (!ValidatePointer(context, contextName)) {
            return false;
        }
        
        // Verifica se o context ainda é válido
        ULONG refCount = context->AddRef();
        context->Release();
        
        if (refCount == 0) {
            Drift::Core::LogRHIError("[" + contextName + "] Context DX11 inválido (refCount = 0)");
            return false;
        }
        
        return true;
    }

    // Loga informações de debug sobre um recurso
    static void LogResourceInfo(const std::string& resourceType, const std::string& resourceName) {
        Drift::Core::LogRHIDebug("Criando " + resourceType + ": " + resourceName);
    }

    // Loga informações de debug sobre operações de renderização
    static void LogRenderOperation(const std::string& operation) {
        Drift::Core::LogRHIDebug("Render: " + operation);
    }

    // Verifica se as dimensões são válidas
    static bool ValidateDimensions(unsigned width, unsigned height, const std::string& context) {
        if (width == 0 || height == 0) {
            Drift::Core::LogRHIError("[" + context + "] Dimensões inválidas: " + 
                                   std::to_string(width) + "x" + std::to_string(height));
            return false;
        }
        
        if (width > 16384 || height > 16384) {
            Drift::Core::LogRHIError("[" + context + "] Dimensões muito grandes: " + 
                                   std::to_string(width) + "x" + std::to_string(height));
            return false;
        }
        
        return true;
    }

    // Verifica se um formato é suportado
    static bool ValidateFormat(DXGI_FORMAT format, const std::string& context) {
        if (format == DXGI_FORMAT_UNKNOWN) {
            Drift::Core::LogRHIError("[" + context + "] Formato DXGI desconhecido");
            return false;
        }
        return true;
    }
};

// Macros para facilitar o uso do debugging
#ifdef _DEBUG
    #define RHI_VALIDATE_POINTER(ptr, context) \
        if (!Drift::RHI::RHIDebug::ValidatePointer(ptr, context)) { \
            throw Drift::RHI::RHIException(context + std::string(": Ponteiro inválido")); \
        }

    #define RHI_VALIDATE_HRESULT(hr, context) \
        if (!Drift::RHI::RHIDebug::ValidateHRESULT(hr, context)) { \
            throw Drift::RHI::RHIException(context + std::string(": HRESULT falhou: 0x") + std::to_string(hr)); \
        }

    #define RHI_VALIDATE_DEVICE(device, context) \
        if (!Drift::RHI::RHIDebug::ValidateDX11Device(device, context)) { \
            throw Drift::RHI::DeviceException("Device inválido em " + context); \
        }

    #define RHI_VALIDATE_CONTEXT(context_ptr, context_name) \
        if (!Drift::RHI::RHIDebug::ValidateDX11Context(context_ptr, context_name)) { \
            throw Drift::RHI::ContextException("Context inválido em " + context_name); \
        }

    #define RHI_LOG_RESOURCE(type, name) \
        Drift::RHI::RHIDebug::LogResourceInfo(type, name)

    #define RHI_LOG_RENDER(op) \
        Drift::RHI::RHIDebug::LogRenderOperation(op)
#else
    #define RHI_VALIDATE_POINTER(ptr, context)
    #define RHI_VALIDATE_HRESULT(hr, context)
    #define RHI_VALIDATE_DEVICE(device, context)
    #define RHI_VALIDATE_CONTEXT(context_ptr, context_name)
    #define RHI_LOG_RESOURCE(type, name)
    #define RHI_LOG_RENDER(op)
#endif

} // namespace Drift::RHI 