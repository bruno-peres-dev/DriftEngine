#include "Drift/Core/Assets/AssetsManagerExample.h"
#include "Drift/Core/Log.h"

namespace Drift::Core::Assets {

void AssetsManagerExample::SetupAssetsManager(RHI::IDevice* device) {
    Log("[AssetsManagerExample] Configurando AssetsManager...");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Configuração do cache
    AssetCacheConfig config;
    config.maxAssets = 500;                    // Máximo de 500 assets
    config.maxMemoryUsage = 512 * 1024 * 1024; // 512MB
    config.enableLazyLoading = true;
    config.enablePreloading = true;
    config.enableAsyncLoading = false;         // Por enquanto, síncrono
    config.trimThreshold = 0.8f;               // Limpa quando usar 80% da memória
    
    assetsManager.SetCacheConfig(config);
    
    // Registra loaders
    assetsManager.RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(device));
    assetsManager.RegisterLoader<FontAsset>(std::make_unique<FontLoader>());
    
    // Configura callbacks
    assetsManager.SetAssetLoadedCallback(OnAssetLoaded);
    assetsManager.SetAssetUnloadedCallback(OnAssetUnloaded);
    
    Log("[AssetsManagerExample] AssetsManager configurado com sucesso!");
}

void AssetsManagerExample::TextureLoadingExample() {
    Log("[AssetsManagerExample] === Exemplo de Carregamento de Texturas ===");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Carregamento básico
    auto grass = assetsManager.LoadAsset<TextureAsset>("textures/grass.png");
    if (grass) {
        Log("[AssetsManagerExample] Textura carregada: " + grass->GetPath());
        Log("[AssetsManagerExample] Uso de memória: " + std::to_string(grass->GetMemoryUsage() / 1024) + " KB");
    }
    
    // Carregamento com parâmetros
    TextureLoadParams params;
    params.format = RHI::Format::R8G8B8A8_UNORM;
    params.generateMips = true;
    params.sRGB = true;
    
    auto logo = assetsManager.LoadAsset<TextureAsset>("textures/logo.png", "", params);
    if (logo) {
        Log("[AssetsManagerExample] Textura com parâmetros carregada: " + logo->GetPath());
    }
    
    // Obtém asset existente (cache hit)
    auto grassCached = assetsManager.GetAsset<TextureAsset>("textures/grass.png");
    if (grassCached) {
        Log("[AssetsManagerExample] Textura obtida do cache (mesmo ponteiro: " + 
            std::to_string(grass.get() == grassCached.get()) + ")");
    }
    
    // Carregamento com variante (diferentes qualidades)
    auto iconHigh = assetsManager.LoadAsset<TextureAsset>("textures/icon.png", "high_quality", params);
    
    params.generateMips = false;
    auto iconLow = assetsManager.LoadAsset<TextureAsset>("textures/icon.png", "low_quality", params);
    
    Log("[AssetsManagerExample] Carregadas duas variantes do mesmo arquivo");
}

void AssetsManagerExample::FontLoadingExample() {
    Log("[AssetsManagerExample] === Exemplo de Carregamento de Fontes ===");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Carregamento básico
    auto arial16 = assetsManager.LoadAsset<FontAsset>("fonts/Arial-Regular.ttf");
    if (arial16) {
        Log("[AssetsManagerExample] Fonte carregada: " + arial16->GetPath());
        Log("[AssetsManagerExample] Tamanho: " + std::to_string(arial16->GetSize()));
    }
    
    // Diferentes tamanhos da mesma fonte
    FontLoadParams params24;
    params24.size = 24.0f;
    params24.quality = UI::FontQuality::High;
    params24.name = "arial";
    
    auto arial24 = assetsManager.LoadAsset<FontAsset>("fonts/Arial-Regular.ttf", "size_24", params24);
    
    FontLoadParams params32;
    params32.size = 32.0f;
    params32.quality = UI::FontQuality::Ultra;
    params32.name = "arial";
    
    auto arial32 = assetsManager.LoadAsset<FontAsset>("fonts/Arial-Regular.ttf", "size_32", params32);
    
    Log("[AssetsManagerExample] Carregados 3 tamanhos diferentes da mesma fonte");
    
    // Fonte diferente
    FontLoadParams robotoParams;
    robotoParams.size = 18.0f;
    robotoParams.quality = UI::FontQuality::Medium;
    robotoParams.name = "roboto";
    
    auto roboto = assetsManager.LoadAsset<FontAsset>("fonts/Roboto-Regular.ttf", "", robotoParams);
    if (roboto) {
        Log("[AssetsManagerExample] Fonte Roboto carregada");
    }
}

void AssetsManagerExample::PreloadingExample() {
    Log("[AssetsManagerExample] === Exemplo de Pré-carregamento ===");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Pré-carregamento individual
    TextureLoadParams texParams;
    texParams.format = RHI::Format::R8G8B8A8_UNORM;
    
    assetsManager.PreloadAsset<TextureAsset>("textures/ui/button.png", "", texParams);
    assetsManager.PreloadAsset<TextureAsset>("textures/ui/panel.png", "", texParams);
    
    FontLoadParams fontParams;
    fontParams.size = 16.0f;
    fontParams.quality = UI::FontQuality::High;
    
    assetsManager.PreloadAsset<FontAsset>("fonts/UI-Regular.ttf", "", fontParams);
    
    // Pré-carregamento em lote
    std::vector<std::string> uiAssets = {
        "textures/ui/cursor.png",
        "textures/ui/icons/save.png",
        "textures/ui/icons/load.png",
        "fonts/UI-Bold.ttf"
    };
    
    assetsManager.PreloadAssets(uiAssets);
    
    Log("[AssetsManagerExample] Pré-carregamento concluído");
}

void AssetsManagerExample::CacheManagementExample() {
    Log("[AssetsManagerExample] === Exemplo de Gerenciamento de Cache ===");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Carrega alguns assets
    assetsManager.LoadAsset<TextureAsset>("textures/temp1.png");
    assetsManager.LoadAsset<TextureAsset>("textures/temp2.png");
    assetsManager.LoadAsset<FontAsset>("fonts/temp.ttf");
    
    // Mostra estatísticas
    assetsManager.LogCacheStats();
    
    // Remove assets não utilizados
    assetsManager.UnloadUnusedAssets();
    Log("[AssetsManagerExample] Assets não utilizados removidos");
    
    // Remove todos os assets de um tipo
    assetsManager.UnloadAssets(std::type_index(typeid(TextureAsset)));
    Log("[AssetsManagerExample] Todas as texturas removidas");
    
    // Trim do cache
    assetsManager.TrimCache();
    Log("[AssetsManagerExample] Cache trimmed");
    
    // Estatísticas finais
    assetsManager.LogCacheStats();
}

void AssetsManagerExample::AdvancedUsageExample() {
    Log("[AssetsManagerExample] === Exemplo de Uso Avançado ===");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Verifica se um asset pode ser carregado
    bool canLoadPNG = assetsManager.CanLoadAsset("test.png", std::type_index(typeid(TextureAsset)));
    bool canLoadTTF = assetsManager.CanLoadAsset("test.ttf", std::type_index(typeid(FontAsset)));
    
    Log("[AssetsManagerExample] Pode carregar PNG: " + std::to_string(canLoadPNG));
    Log("[AssetsManagerExample] Pode carregar TTF: " + std::to_string(canLoadTTF));
    
    // Carrega ou obtém asset (lazy loading)
    auto texture = assetsManager.GetOrLoadAsset<TextureAsset>("textures/lazy_loaded.png");
    if (texture) {
        Log("[AssetsManagerExample] Asset carregado sob demanda");
    }
    
    // Verifica se asset está carregado
    bool isLoaded = assetsManager.IsAssetLoaded("textures/lazy_loaded.png", 
                                               std::type_index(typeid(TextureAsset)));
    Log("[AssetsManagerExample] Asset está carregado: " + std::to_string(isLoaded));
    
    // Obtém estatísticas detalhadas
    auto stats = assetsManager.GetCacheStats();
    Log("[AssetsManagerExample] Cache hits: " + std::to_string(stats.cacheHits));
    Log("[AssetsManagerExample] Cache misses: " + std::to_string(stats.cacheMisses));
    Log("[AssetsManagerExample] Tempo médio de carregamento: " + 
        std::to_string(stats.averageLoadTime * 1000.0) + " ms");
}

void AssetsManagerExample::CompleteExample(RHI::IDevice* device) {
    Log("[AssetsManagerExample] === Exemplo Completo ===");
    
    // Configuração inicial
    SetupAssetsManager(device);
    
    // Exemplos de uso
    TextureLoadingExample();
    FontLoadingExample();
    PreloadingExample();
    CacheManagementExample();
    AdvancedUsageExample();
    
    // Limpeza final
    auto& assetsManager = AssetsManager::GetInstance();
    assetsManager.ClearCache();
    
    Log("[AssetsManagerExample] Exemplo completo finalizado");
}

void AssetsManagerExample::OnAssetLoaded(const std::string& path, std::type_index type) {
    Log("[AssetsManagerExample] [CALLBACK] Asset carregado: " + path + 
        " (tipo: " + std::string(type.name()) + ")");
}

void AssetsManagerExample::OnAssetUnloaded(const std::string& path, std::type_index type) {
    Log("[AssetsManagerExample] [CALLBACK] Asset descarregado: " + path + 
        " (tipo: " + std::string(type.name()) + ")");
}

} // namespace Drift::Core::Assets