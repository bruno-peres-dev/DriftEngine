#pragma once
#include <stdexcept>
#include <string>

namespace Drift::RHI {

// Exceção base para erros do RHI
class RHIException : public std::runtime_error {
public:
    explicit RHIException(const std::string& message) 
        : std::runtime_error(message) {}
    
    explicit RHIException(const std::string& context, const std::string& message)
        : std::runtime_error("[" + context + "] " + message) {}
};

// Exceção específica para erros de criação de recursos
class ResourceCreationException : public RHIException {
public:
    explicit ResourceCreationException(const std::string& resourceType, const std::string& details)
        : RHIException("ResourceCreation", resourceType + ": " + details) {}
};

// Exceção específica para erros de shader
class ShaderException : public RHIException {
public:
    explicit ShaderException(const std::string& shaderType, const std::string& details)
        : RHIException("Shader", shaderType + ": " + details) {}
};

// Exceção específica para erros de dispositivo
class DeviceException : public RHIException {
public:
    explicit DeviceException(const std::string& details)
        : RHIException("Device", details) {}
};

// Exceção específica para erros de contexto
class ContextException : public RHIException {
public:
    explicit ContextException(const std::string& details)
        : RHIException("Context", details) {}
};

// Exceção específica para erros de swapchain
class SwapChainException : public RHIException {
public:
    explicit SwapChainException(const std::string& details)
        : RHIException("SwapChain", details) {}
};

} // namespace Drift::RHI 