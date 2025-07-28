#include "Drift/Core/Assets/Integration.h"
#include "Drift/Core/Log.h"
#include <chrono>

namespace Drift::Core::Assets {

// Variáveis estáticas
bool DriftEngineIntegration::s_Initialized = false;
RHI::IDevice* DriftEngineIntegration::s_Device = nullptr;
std::chrono::steady_clock::time_point DriftEngineIntegration::s_LastCleanup = std::chrono::steady_clock::now();

void DriftEngineIntegration::Initialize(RHI::IDevice* device) {
    if (s_Initialized) {
        Log("[DriftEngineIntegration] AVISO: Sistema já inicializado");
        return;
    }
    
    s_Device = device;
    Log("[DriftEngineIntegration] Inicializando sistema de assets...");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Configuração otimizada para o DriftEngine
    AssetCacheConfig config;
    config.maxAssets = 2000;                    // Suporte para muitos assets
    config.maxMemoryUsage = 1024 * 1024 * 1024; // 1GB para assets
    config.enableLazyLoading = true;            // Carregamento sob demanda
    config.enablePreloading = true;             // Pré-carregamento habilitado
    config.enableAsyncLoading = false;          // Síncrono por enquanto
    config.trimThreshold = 0.75f;               // Limpa aos 75%
    
    assetsManager.SetCacheConfig(config);
    
    // Registra loaders específicos
    assetsManager.RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(device));
    assetsManager.RegisterLoader<FontAsset>(std::make_unique<FontLoader>());
    
    SetupCallbacks();
    PreloadCriticalAssets();
    
    s_Initialized = true;
    Log("[DriftEngineIntegration] Sistema de assets inicializado com sucesso!");
}

void DriftEngineIntegration::SetupCallbacks() {
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Callback unificado que redireciona baseado no tipo
    assetsManager.SetAssetLoadedCallback([](const std::string& path, std::type_index type) {
        if (type == std::type_index(typeid(TextureAsset))) {
            OnTextureLoaded(path, type);
        } else if (type == std::type_index(typeid(FontAsset))) {
            OnFontLoaded(path, type);
        }
    });
    
    assetsManager.SetAssetUnloadedCallback(OnAssetUnloaded);
}

void DriftEngineIntegration::PreloadCriticalAssets() {
    Log("[DriftEngineIntegration] Pré-carregando assets críticos...");
    
    // Assets críticos da UI
    std::vector<std::string> criticalAssets = {
        "fonts/Arial-Regular.ttf",  // Fonte padrão
        "textures/grass.png"        // Textura de teste
    };
    
    auto& assetsManager = AssetsManager::GetInstance();
    assetsManager.PreloadAssets(criticalAssets);
    
    // Pré-carrega tamanhos comuns de fonte
    FontLoadParams fontParams;
    fontParams.quality = UI::FontQuality::High;
    fontParams.name = "default";
    
    for (float size : {12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 24.0f, 32.0f}) {
        fontParams.size = size;
        std::string variant = "size_" + std::to_string(static_cast<int>(size));
        assetsManager.PreloadAsset<FontAsset>("fonts/Arial-Regular.ttf", variant, fontParams);
    }
    
    Log("[DriftEngineIntegration] Pré-carregamento concluído");
}

void DriftEngineIntegration::Update() {
    if (!s_Initialized) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - s_LastCleanup).count();
    
    // Limpeza automática a cada 60 segundos
    if (elapsed >= 60) {
        auto& assetsManager = AssetsManager::GetInstance();
        
        // Remove assets não utilizados
        assetsManager.UnloadUnusedAssets();
        
        // Trim do cache se necessário
        auto stats = assetsManager.GetCacheStats();
        float memoryUsage = static_cast<float>(stats.memoryUsage) / stats.maxMemoryUsage;
        
        if (memoryUsage > 0.8f) {
            assetsManager.TrimCache();
            Log("[DriftEngineIntegration] Cache trimmed - uso de memória: " + 
                std::to_string(memoryUsage * 100.0f) + "%");
        }
        
        s_LastCleanup = now;
    }
}

void DriftEngineIntegration::Shutdown() {
    if (!s_Initialized) {
        return;
    }
    
    Log("[DriftEngineIntegration] Finalizando sistema de assets...");
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Log estatísticas finais
    LogSystemStats();
    
    // Limpa todos os assets
    assetsManager.ClearCache();
    
    // Remove loaders
    assetsManager.UnregisterLoader<TextureAsset>();
    assetsManager.UnregisterLoader<FontAsset>();
    
    s_Initialized = false;
    s_Device = nullptr;
    
    Log("[DriftEngineIntegration] Sistema de assets finalizado");
}

std::shared_ptr<RHI::ITexture> DriftEngineIntegration::LoadTexture(const std::string& path, const std::string& variant) {
    if (!s_Initialized) {
        Log("[DriftEngineIntegration] ERRO: Sistema não inicializado");
        return nullptr;
    }
    
    auto& assetsManager = AssetsManager::GetInstance();
    auto textureAsset = assetsManager.GetOrLoadAsset<TextureAsset>(path, variant);
    
    if (textureAsset) {
        return textureAsset->GetTexture();
    }
    
    return nullptr;
}

std::shared_ptr<UI::Font> DriftEngineIntegration::LoadFont(const std::string& path, float size, 
                                                          UI::FontQuality quality, const std::string& name) {
    if (!s_Initialized) {
        Log("[DriftEngineIntegration] ERRO: Sistema não inicializado");
        return nullptr;
    }
    
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Cria parâmetros
    FontLoadParams params;
    params.size = size;
    params.quality = quality;
    params.name = name.empty() ? "" : name;
    
    // Cria variante baseada no tamanho e qualidade
    std::string variant = "size_" + std::to_string(static_cast<int>(size)) + 
                         "_q" + std::to_string(static_cast<int>(quality));
    
    auto fontAsset = assetsManager.GetOrLoadAsset<FontAsset>(path, variant, params);
    
    if (fontAsset) {
        return fontAsset->GetFont();
    }
    
    return nullptr;
}

void DriftEngineIntegration::LogSystemStats() {
    if (!s_Initialized) {
        return;
    }
    
    Log("[DriftEngineIntegration] === Estatísticas do Sistema de Assets ===");
    
    auto& assetsManager = AssetsManager::GetInstance();
    auto stats = assetsManager.GetCacheStats();
    
    Log("[DriftEngineIntegration] Total de Assets: " + std::to_string(stats.totalAssets));
    Log("[DriftEngineIntegration] Assets Carregados: " + std::to_string(stats.loadedAssets));
    Log("[DriftEngineIntegration] Uso de Memória: " + 
        std::to_string(stats.memoryUsage / (1024 * 1024)) + " MB / " + 
        std::to_string(stats.maxMemoryUsage / (1024 * 1024)) + " MB");
    
    float hitRate = stats.cacheHits + stats.cacheMisses > 0 ? 
                   (float)stats.cacheHits / (stats.cacheHits + stats.cacheMisses) * 100.0f : 0.0f;
    Log("[DriftEngineIntegration] Taxa de Acerto do Cache: " + std::to_string(hitRate) + "%");
    
    Log("[DriftEngineIntegration] Carregamentos: " + std::to_string(stats.loadCount));
    Log("[DriftEngineIntegration] Descarregamentos: " + std::to_string(stats.unloadCount));
    Log("[DriftEngineIntegration] Tempo Médio de Carregamento: " + 
        std::to_string(stats.averageLoadTime * 1000.0) + " ms");
    
    // Estatísticas por tipo
    if (!stats.assetsByType.empty()) {
        Log("[DriftEngineIntegration] === Por Tipo ===");
        for (const auto& [type, count] : stats.assetsByType) {
            size_t memory = stats.memoryByType.count(type) ? stats.memoryByType.at(type) : 0;
            Log("[DriftEngineIntegration] " + std::string(type.name()) + ": " + 
                std::to_string(count) + " assets, " + 
                std::to_string(memory / (1024 * 1024)) + " MB");
        }
    }
}

void DriftEngineIntegration::OnTextureLoaded(const std::string& path, std::type_index type) {
    Log("[DriftEngineIntegration] [TEXTURE] Carregada: " + path);
    
    // Aqui poderia notificar outros sistemas do DriftEngine
    // Por exemplo, invalidar caches de renderização, etc.
}

void DriftEngineIntegration::OnFontLoaded(const std::string& path, std::type_index type) {
    Log("[DriftEngineIntegration] [FONT] Carregada: " + path);
    
    // Aqui poderia sincronizar com o FontManager existente
    // ou notificar o sistema de UI sobre nova fonte disponível
}

void DriftEngineIntegration::OnAssetUnloaded(const std::string& path, std::type_index type) {
    Log("[DriftEngineIntegration] [UNLOAD] Asset descarregado: " + path + 
        " (tipo: " + std::string(type.name()) + ")");
    
    // Aqui poderia limpar referências em outros sistemas
}

} // namespace Drift::Core::Assets