#pragma once

#include "Drift/Core/AssetsManager.h"
#include "Drift/Core/Assets/TextureAsset.h"
#include "Drift/Core/Assets/FontAsset.h"
#include "Drift/RHI/Device.h"

namespace Drift::Core::Assets {

/**
 * @brief Classe de exemplo mostrando como configurar e usar o AssetsManager
 */
class AssetsManagerExample {
public:
    /**
     * @brief Configura o AssetsManager com loaders padrão
     * @param device Device RHI para carregamento de texturas
     */
    static void SetupAssetsManager(RHI::IDevice* device);
    
    /**
     * @brief Exemplo de carregamento de texturas
     */
    static void TextureLoadingExample();
    
    /**
     * @brief Exemplo de carregamento de fontes
     */
    static void FontLoadingExample();
    
    /**
     * @brief Exemplo de pré-carregamento de assets
     */
    static void PreloadingExample();
    
    /**
     * @brief Exemplo de gerenciamento de cache
     */
    static void CacheManagementExample();
    
    /**
     * @brief Exemplo de uso avançado com callbacks
     */
    static void AdvancedUsageExample();
    
    /**
     * @brief Exemplo completo de uso do sistema
     */
    static void CompleteExample(RHI::IDevice* device);

private:
    static void OnAssetLoaded(const std::string& path, std::type_index type);
    static void OnAssetUnloaded(const std::string& path, std::type_index type);
};

} // namespace Drift::Core::Assets