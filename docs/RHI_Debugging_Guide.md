# Guia de Debugging do Sistema RHI

## Visão Geral

O sistema RHI (Rendering Hardware Interface) do DriftEngine agora possui um sistema robusto de debugging e tratamento de exceções que ajuda a identificar e resolver crashes e problemas de renderização.

## Componentes do Sistema de Debugging

### 1. Sistema de Logging Aprimorado

#### Funções de Logging RHI
```cpp
// Logging específico para RHI
Drift::Core::LogRHI("Mensagem informativa");
Drift::Core::LogRHIError("Mensagem de erro");
Drift::Core::LogRHIDebug("Mensagem de debug");

// Logging de exceções com contexto
Drift::Core::LogException("Contexto da operação", exception);

// Logging de HRESULT do DirectX
Drift::Core::LogHRESULT("Operação", hr);
```

### 2. Sistema de Exceções Específicas

#### Hierarquia de Exceções
```cpp
Drift::RHI::RHIException              // Exceção base
├── Drift::RHI::DeviceException       // Erros de dispositivo
├── Drift::RHI::ContextException      // Erros de contexto
├── Drift::RHI::SwapChainException    // Erros de swapchain
├── Drift::RHI::ResourceCreationException // Erros de criação de recursos
└── Drift::RHI::ShaderException       // Erros de shader
```

### 3. Sistema de Validação

#### Macros de Validação (Debug Mode)
```cpp
RHI_VALIDATE_POINTER(ptr, "contexto");           // Valida ponteiro
RHI_VALIDATE_HRESULT(hr, "contexto");            // Valida HRESULT
RHI_VALIDATE_DEVICE(device, "contexto");         // Valida device DX11
RHI_VALIDATE_CONTEXT(context, "contexto");       // Valida context DX11
RHI_LOG_RESOURCE("tipo", "nome");                // Log de criação de recurso
RHI_LOG_RENDER("operação");                      // Log de operação de renderização
```

#### Funções de Validação
```cpp
// Validação manual
Drift::RHI::RHIDebug::ValidatePointer(ptr, "contexto");
Drift::RHI::RHIDebug::ValidateHRESULT(hr, "contexto");
Drift::RHI::RHIDebug::ValidateDX11Device(device, "contexto");
Drift::RHI::RHIDebug::ValidateDX11Context(context, "contexto");
Drift::RHI::RHIDebug::ValidateDimensions(width, height, "contexto");
Drift::RHI::RHIDebug::ValidateFormat(format, "contexto");
```

## Como Usar

### 1. Configuração do Nível de Log

```cpp
// No início da aplicação
Drift::Core::SetLogLevel(Drift::Core::LogLevel::Debug);  // Log completo
Drift::Core::SetLogLevel(Drift::Core::LogLevel::Warning); // Apenas warnings e erros
```

### 2. Tratamento de Exceções

```cpp
try {
    // Código RHI
    auto device = RHI::DX11::CreateDeviceDX11(desc);
} catch (const Drift::RHI::DeviceException& e) {
    Drift::Core::LogException("Criação de Device", e);
    // Tratamento específico
} catch (const Drift::RHI::RHIException& e) {
    Drift::Core::LogException("Erro RHI", e);
    // Tratamento genérico
}
```

### 3. Validação de Recursos

```cpp
// Em funções de criação de recursos
void CreateTexture() {
    RHI_VALIDATE_DEVICE(_device.Get(), "CreateTexture");
    RHI_LOG_RESOURCE("Texture", "grass.png");
    
    HRESULT hr = _device->CreateTexture2D(&desc, nullptr, &texture);
    RHI_VALIDATE_HRESULT(hr, "CreateTexture2D");
}
```

## Exemplos de Uso

### Exemplo 1: Criação de Device com Debugging

```cpp
DeviceDX11::DeviceDX11(const DeviceDesc& desc) : _desc(desc) {
    Drift::Core::LogRHI("Iniciando criação do Device DX11");
    
    // Validar dimensões
    RHIDebug::ValidateDimensions(desc.width, desc.height, "DeviceDX11 constructor");
    
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    Drift::Core::LogRHIDebug("Device DX11 criado com flags de debug");
#endif

    HRESULT hr = D3D11CreateDevice(/* parâmetros */);
    RHI_VALIDATE_HRESULT(hr, "D3D11CreateDevice");
    
    // Validar device e context criados
    RHI_VALIDATE_DEVICE(_device.Get(), "DeviceDX11 constructor");
    RHI_VALIDATE_CONTEXT(_context.Get(), "DeviceDX11 constructor");
    
    Drift::Core::LogRHI("Device DX11 criado com sucesso");
}
```

### Exemplo 2: Tratamento de Exceções no Main

```cpp
int main() {
    try {
        // Código da aplicação
    } catch (const Drift::RHI::DeviceException& e) {
        Core::LogException("Device Exception", e);
        MessageBoxA(nullptr, ("Erro de Device: " + std::string(e.what())).c_str(),
                   "DriftEngine Device Error", MB_ICONERROR);
        return -1;
    } catch (const Drift::RHI::ContextException& e) {
        Core::LogException("Context Exception", e);
        MessageBoxA(nullptr, ("Erro de Context: " + std::string(e.what())).c_str(),
                   "DriftEngine Context Error", MB_ICONERROR);
        return -1;
    }
}
```

## Debugging de Problemas Comuns

### 1. Device Removido
```cpp
// O sistema automaticamente detecta e reporta
// DXGI_ERROR_DEVICE_REMOVED
// DXGI_ERROR_DRIVER_INTERNAL_ERROR
```

### 2. Recursos Inválidos
```cpp
// Validação automática de ponteiros e recursos
RHI_VALIDATE_POINTER(texture, "PSSetTexture");
RHI_VALIDATE_DX11_RESOURCE(texture, "PSSetTexture");
```

### 3. Dimensões Inválidas
```cpp
// Validação de dimensões de viewport/textura
RHIDebug::ValidateDimensions(width, height, "SetViewport");
```

## Compilação

### Debug Mode (Recomendado para desenvolvimento)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

### Release Mode (Sem validações de debug)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Logs de Exemplo

### Log de Sucesso
```
[RHI] Iniciando criação do Device DX11
[RHI][DEBUG] Device DX11 criado com flags de debug
[RHI][DEBUG] Chamando D3D11CreateDevice...
[RHI] Device DX11 criado com sucesso. FeatureLevel: 11.0
```

### Log de Erro
```
[RHI][ERROR] [CreateTexture] Ponteiro é nullptr
[EXCEPTION][CreateTexture] [ResourceCreation] Texture: Ponteiro inválido
```

### Log de HRESULT
```
[HRESULT][D3D11CreateDevice] 0x80070057 (E_INVALIDARG - Argumento inválido)
```

## Dicas de Debugging

1. **Sempre compile em Debug mode** durante desenvolvimento
2. **Configure o nível de log para Debug** para ver todas as mensagens
3. **Use as macros de validação** em pontos críticos do código
4. **Capture e log exceções** com contexto específico
5. **Monitore os logs** para identificar padrões de erro

## Troubleshooting

### Problema: Aplicação crasha sem mensagem
**Solução**: Configure o tratamento global de exceções no main()

### Problema: Não vejo logs de debug
**Solução**: Verifique se está compilando em Debug mode e se o nível de log está configurado corretamente

### Problema: HRESULT desconhecido
**Solução**: O sistema já inclui mapeamento dos HRESULTs mais comuns. Para novos códigos, adicione ao switch case em LogHRESULT() 