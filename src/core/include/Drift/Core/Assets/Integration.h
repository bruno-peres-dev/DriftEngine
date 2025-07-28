#pragma once

#include "Drift/Core/AssetsManager.h"
#include "Drift/Core/Assets/TextureAsset.h"
#include "Drift/Core/Assets/FontAsset.h"
#include "Drift/RHI/Device.h"
#include "Drift/UI/FontSystem/FontManager.h"

namespace Drift::Core::Assets {

/**
 * @brief Classe utilitária para integração do AssetsManager com o DriftEngine
 */
class DriftEngineIntegration {
public:
    /**
     * @brief Inicializa o AssetsManager com configurações otimizadas para o DriftEngine
     * @param device Device RHI para carregamento de texturas
     */
    static void Initialize(RHI::IDevice* device);
    
    /**
     * @brief Configura callbacks para integração com sistemas existentes
     */
    static void SetupCallbacks();
    
    /**
     * @brief Pré-carrega assets críticos do DriftEngine
     */
    static void PreloadCriticalAssets();
    
    /**
     * @brief Atualização periódica do sistema de assets
     * Deve ser chamada no loop principal da aplicação
     */
    static void Update();
    
    /**
     * @brief Finaliza o sistema e limpa recursos
     */
    static void Shutdown();
    
    /**
     * @brief Carrega texturas de forma compatível com o sistema existente
     * @param path Caminho da textura
     * @param variant Variante opcional (qualidade, etc)
     * @return Textura RHI pronta para uso
     */
    static std::shared_ptr<RHI::ITexture> LoadTexture(const std::string& path, const std::string& variant = "");
    
    /**
     * @brief Carrega fontes de forma compatível com o FontManager existente
     * @param path Caminho da fonte
     * @param size Tamanho da fonte
     * @param quality Qualidade da fonte
     * @param name Nome da fonte (opcional)
     * @return Fonte UI pronta para uso
     */
    static std::shared_ptr<UI::Font> LoadFont(const std::string& path, float size, 
                                             UI::FontQuality quality = UI::FontQuality::High,
                                             const std::string& name = "");
    
    /**
     * @brief Obtém estatísticas do sistema de assets para debug
     */
    static void LogSystemStats();

private:
    static void OnTextureLoaded(const std::string& path, std::type_index type);
    static void OnFontLoaded(const std::string& path, std::type_index type);
    static void OnAssetUnloaded(const std::string& path, std::type_index type);
    
    static bool s_Initialized;
    static RHI::IDevice* s_Device;
    static std::chrono::steady_clock::time_point s_LastCleanup;
};

} // namespace Drift::Core::Assets