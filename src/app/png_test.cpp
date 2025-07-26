#include "Drift/Core/Log.h"
#include "Drift/RHI/Device.h"
#include "Drift/RHI/Texture.h"
#include "Drift/RHI/DX11/DeviceDX11.h"
#include <iostream>

int main() {
    try {
        Drift::Core::Log("[PNG Test] Iniciando teste de carregamento PNG...");
        
        // Cria Device DX11
        Drift::RHI::DeviceDesc desc{ 1280, 720, false };
        auto device = Drift::RHI::DX11::CreateDeviceDX11(desc);
        
        // Tenta carregar a textura PNG
        Drift::RHI::TextureDesc textureDesc;
        textureDesc.path = L"textures/grass.png";
        
        Drift::Core::Log("[PNG Test] Tentando carregar: textures/grass.png");
        
        auto texture = device->CreateTexture(textureDesc);
        
        Drift::Core::Log("[PNG Test] Sucesso! Textura PNG carregada com sucesso.");
        
        return 0;
    }
    catch (const std::exception& e) {
        Drift::Core::Log("[PNG Test] ERRO: " + std::string(e.what()));
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
} 