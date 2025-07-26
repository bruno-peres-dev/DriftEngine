// Teste do sistema de debugging do RHI
// Compile com: g++ -std=c++17 -I./src test_rhi_debug.cpp -o test_rhi_debug

#include "Drift/Core/Log.h"
#include "Drift/RHI/RHIException.h"
#include "Drift/RHI/RHIDebug.h"
#include <iostream>

int main() {
    std::cout << "=== Teste do Sistema de Debugging RHI ===" << std::endl;
    
    // Teste 1: Logging básico
    Drift::Core::LogRHI("Teste de logging RHI");
    Drift::Core::LogRHIError("Teste de erro RHI");
    Drift::Core::LogRHIDebug("Teste de debug RHI");
    
    // Teste 2: Validação de ponteiros
    int* ptr = nullptr;
    bool isValid = Drift::RHI::RHIDebug::ValidatePointer(ptr, "Teste ponteiro nulo");
    std::cout << "Ponteiro nulo válido: " << (isValid ? "SIM" : "NÃO") << std::endl;
    
    int value = 42;
    ptr = &value;
    isValid = Drift::RHI::RHIDebug::ValidatePointer(ptr, "Teste ponteiro válido");
    std::cout << "Ponteiro válido: " << (isValid ? "SIM" : "NÃO") << std::endl;
    
    // Teste 3: Validação de dimensões
    bool dimsValid = Drift::RHI::RHIDebug::ValidateDimensions(1920, 1080, "Teste dimensões válidas");
    std::cout << "Dimensões 1920x1080 válidas: " << (dimsValid ? "SIM" : "NÃO") << std::endl;
    
    dimsValid = Drift::RHI::RHIDebug::ValidateDimensions(0, 1080, "Teste dimensões inválidas");
    std::cout << "Dimensões 0x1080 válidas: " << (dimsValid ? "SIM" : "NÃO") << std::endl;
    
    // Teste 4: Exceções RHI
    try {
        throw Drift::RHI::DeviceException("Teste de exceção de device");
    } catch (const Drift::RHI::DeviceException& e) {
        Drift::Core::LogException("Teste DeviceException", e);
    }
    
    try {
        throw Drift::RHI::ContextException("Teste de exceção de context");
    } catch (const Drift::RHI::ContextException& e) {
        Drift::Core::LogException("Teste ContextException", e);
    }
    
    try {
        throw Drift::RHI::SwapChainException("Teste de exceção de swapchain");
    } catch (const Drift::RHI::SwapChainException& e) {
        Drift::Core::LogException("Teste SwapChainException", e);
    }
    
    std::cout << "=== Teste concluído ===" << std::endl;
    return 0;
} 