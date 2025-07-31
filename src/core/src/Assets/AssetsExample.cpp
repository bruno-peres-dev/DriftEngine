#include "Drift/Core/Assets/AssetsExample.h"
#include "Drift/Core/Log.h"
#include <random>
#include <algorithm>
#include <thread>
#include <filesystem>

namespace Drift::Core::Assets {

// SimpleAsset implementation
SimpleAsset::SimpleAsset(const std::string& path, const std::string& name)
    : m_Path(path), m_Name(name) {
}

bool SimpleAsset::Load() {
    if (m_Status == AssetStatus::Loaded) {
        return true;
    }
    
    m_Status = AssetStatus::Loading;
    
    // Simula carregamento
    std::this_thread::sleep_for(std::chrono::milliseconds(100 + (rand() % 200))); // 100-300ms
    
    // Simula falha ocasional (5% de chance)
    if (rand() % 100 < 5) {
        m_Status = AssetStatus::Failed;
        return false;
    }
    
    // Simula uso de memória baseado no tamanho do nome
    m_MemoryUsage = m_Name.length() * 1024; // 1KB por caractere
    
    m_Status = AssetStatus::Loaded;
    m_LoadTime = std::chrono::steady_clock::now();
    
    return true;
}

void SimpleAsset::Unload() {
    if (m_Status == AssetStatus::Loaded) {
        m_Status = AssetStatus::NotLoaded;
        m_MemoryUsage = 0;
        m_AccessCount = 0;
    }
}

// SimpleAssetLoader implementation
std::shared_ptr<SimpleAsset> SimpleAssetLoader::Load(const std::string& path, const std::any& params) {
    // Extrai nome do arquivo do path
    std::filesystem::path fsPath(path);
    std::string name = fsPath.stem().string();
    
    auto asset = std::make_shared<SimpleAsset>(path, name);
    if (asset->Load()) {
        return asset;
    }
    
    return nullptr;
}

bool SimpleAssetLoader::CanLoad(const std::string& path) const {
    std::filesystem::path fsPath(path);
    std::string extension = fsPath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    return extension == ".asset" || extension == ".data";
}

std::vector<std::string> SimpleAssetLoader::GetSupportedExtensions() const {
    return {".asset", ".data"};
}

size_t SimpleAssetLoader::EstimateMemoryUsage(const std::string& path) const {
    std::filesystem::path fsPath(path);
    std::string name = fsPath.stem().string();
    return name.length() * 1024; // 1KB por caractere
}

// AssetsExample implementation
void AssetsExample::RunBasicExample() {
    LOG_INFO("[AssetsExample] Iniciando exemplo básico...");
    
    auto& assetsSystem = AssetsSystem::GetInstance();
    assetsSystem.Initialize();
    
    // Registra loader
    auto loader = std::make_unique<SimpleAssetLoader>();
    assetsSystem.RegisterLoader<SimpleAsset>(std::move(loader));
    
    // Carrega assets
    auto asset1 = DRIFT_LOAD_ASSET(SimpleAsset, "textures/grass.asset");
    auto asset2 = DRIFT_LOAD_ASSET(SimpleAsset, "models/tree.asset");
    auto asset3 = DRIFT_LOAD_ASSET(SimpleAsset, "sounds/ambient.asset");
    
    if (asset1) {
        DRIFT_LOG_INFO("[AssetsExample] Asset 1 carregado: ", asset1->GetName());
    }
    if (asset2) {
        DRIFT_LOG_INFO("[AssetsExample] Asset 2 carregado: ", asset2->GetName());
    }
    if (asset3) {
        DRIFT_LOG_INFO("[AssetsExample] Asset 3 carregado: ", asset3->GetName());
    }
    
    // Testa cache
    auto cachedAsset = DRIFT_GET_ASSET(SimpleAsset, "textures/grass.asset");
    if (cachedAsset) {
        DRIFT_LOG_INFO("[AssetsExample] Asset encontrado no cache: ", cachedAsset->GetName());
    }
    
    assetsSystem.LogStats();
    LOG_INFO("[AssetsExample] Exemplo básico concluído!");
}

void AssetsExample::RunAsyncLoadingExample() {
    LOG_INFO("[AssetsExample] Iniciando exemplo de carregamento assíncrono...");
    
    auto& assetsSystem = AssetsSystem::GetInstance();
    assetsSystem.Initialize();
    
    // Registra loader
    auto loader = std::make_unique<SimpleAssetLoader>();
    assetsSystem.RegisterLoader<SimpleAsset>(std::move(loader));
    
    // Configura callbacks
    assetsSystem.SetAssetLoadedCallback(OnAssetLoaded);
    assetsSystem.SetAssetFailedCallback(OnAssetFailed);
    
    // Carrega assets assincronamente
    std::vector<std::future<std::shared_ptr<SimpleAsset>>> futures;
    
    for (int i = 0; i < 10; ++i) {
        std::string path = "async_asset_" + std::to_string(i) + ".asset";
        auto future = DRIFT_LOAD_ASSET_ASYNC(SimpleAsset, path);
        futures.push_back(std::move(future));
    }
    
    LOG_INFO("[AssetsExample] Assets submetidos para carregamento assíncrono...");
    
    // Aguarda alguns assets
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            auto asset = futures[i].get();
            if (asset) {
                DRIFT_LOG_INFO("[AssetsExample] Asset ", i, " carregado: ", asset->GetName());
            }
        } catch (const std::exception& e) {
            DRIFT_LOG_ERROR("[AssetsExample] Falha ao carregar asset {}: {}", i, e.what());
        }
    }
    
    assetsSystem.LogStats();
    LOG_INFO("[AssetsExample] Exemplo de carregamento assíncrono concluído!");
}

void AssetsExample::RunPreloadingExample() {
    LOG_INFO("[AssetsExample] Iniciando exemplo de pré-carregamento...");
    
    auto& assetsSystem = AssetsSystem::GetInstance();
    assetsSystem.Initialize();
    
    // Registra loader
    auto loader = std::make_unique<SimpleAssetLoader>();
    assetsSystem.RegisterLoader<SimpleAsset>(std::move(loader));
    
    // Lista de assets para pré-carregar
    std::vector<std::string> preloadPaths = {
        "textures/grass.asset",
        "textures/stone.asset",
        "textures/wood.asset",
        "models/tree.asset",
        "models/rock.asset",
        "sounds/ambient.asset",
        "sounds/footstep.asset"
    };
    
            DRIFT_LOG_INFO("[AssetsExample] Iniciando pré-carregamento de ", preloadPaths.size(), " assets...");
    
    // Pré-carrega assets
    for (const auto& path : preloadPaths) {
        DRIFT_PRELOAD_ASSET(SimpleAsset, path);
    }
    
    // Aguarda um pouco para os assets carregarem
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Verifica quais assets foram carregados
    for (const auto& path : preloadPaths) {
        auto asset = DRIFT_GET_ASSET(SimpleAsset, path);
        if (asset && asset->IsLoaded()) {
            DRIFT_LOG_INFO("[AssetsExample] Asset pré-carregado: ", asset->GetName());
        }
    }
    
    assetsSystem.LogStats();
    LOG_INFO("[AssetsExample] Exemplo de pré-carregamento concluído!");
}

void AssetsExample::RunCacheManagementExample() {
    LOG_INFO("[AssetsExample] Iniciando exemplo de gerenciamento de cache...");
    
    auto& assetsSystem = AssetsSystem::GetInstance();
    
    // Configuração com limites baixos para testar cache
    AssetsConfig config;
    config.maxAssets = 5;
    config.maxMemoryUsage = 1024 * 1024; // 1MB
    config.enableLazyUnloading = true;
    
    assetsSystem.Initialize(config);
    
    // Registra loader
    auto loader = std::make_unique<SimpleAssetLoader>();
    assetsSystem.RegisterLoader<SimpleAsset>(std::move(loader));
    
    // Carrega mais assets que o limite
    std::vector<std::shared_ptr<SimpleAsset>> assets;
    
    for (int i = 0; i < 10; ++i) {
        std::string path = "cache_test_" + std::to_string(i) + ".asset";
        auto asset = DRIFT_LOAD_ASSET(SimpleAsset, path);
        if (asset) {
            assets.push_back(asset);
            DRIFT_LOG_INFO("[AssetsExample] Asset carregado: ", asset->GetName());
        }
    }
    
    LOG_INFO("[AssetsExample] Cache após carregamento:");
    assetsSystem.LogStats();
    
    // Força limpeza de cache
    LOG_INFO("[AssetsExample] Forçando limpeza de cache...");
    assetsSystem.TrimCache();
    
    LOG_INFO("[AssetsExample] Cache após limpeza:");
    assetsSystem.LogStats();
    
    // Descarta referências para testar descarregamento automático
    assets.clear();
    
    LOG_INFO("[AssetsExample] Descarregando assets não utilizados...");
    assetsSystem.UnloadUnusedAssets();
    
    LOG_INFO("[AssetsExample] Cache final:");
    assetsSystem.LogStats();
    
    LOG_INFO("[AssetsExample] Exemplo de gerenciamento de cache concluído!");
}

void AssetsExample::RunPerformanceTest() {
    LOG_INFO("[AssetsExample] Iniciando teste de performance...");
    
    auto& assetsSystem = AssetsSystem::GetInstance();
    assetsSystem.Initialize();
    
    // Registra loader
    auto loader = std::make_unique<SimpleAssetLoader>();
    assetsSystem.RegisterLoader<SimpleAsset>(std::move(loader));
    
    // Gera lista de assets
    auto assetPaths = GenerateAssetPaths(100);
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Carrega assets em paralelo
    std::vector<std::future<std::shared_ptr<SimpleAsset>>> futures;
    
    for (const auto& path : assetPaths) {
        auto future = DRIFT_LOAD_ASSET_ASYNC(SimpleAsset, path);
        futures.push_back(std::move(future));
    }
    
    // Aguarda todos terminarem
    for (auto& future : futures) {
        future.get();
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    DRIFT_LOG_INFO("[AssetsExample] Performance: ", assetPaths.size(), " assets em ", duration.count(), "ms");
    DRIFT_LOG_INFO("[AssetsExample] Taxa: ", assetPaths.size() * 1000 / duration.count(), " assets/segundo");
    
    assetsSystem.LogStats();
    LOG_INFO("[AssetsExample] Teste de performance concluído!");
}

void AssetsExample::RunCompleteExample() {
    LOG_INFO("[AssetsExample] Iniciando exemplo completo...");
    
    auto& assetsSystem = AssetsSystem::GetInstance();
    
    // Configuração completa
    AssetsConfig config;
    config.maxAssets = 50;
    config.maxMemoryUsage = 10 * 1024 * 1024; // 10MB
    config.enableAsyncLoading = true;
    config.enablePreloading = true;
    config.enableLazyUnloading = true;
    config.maxConcurrentLoads = 8;
    
    assetsSystem.Initialize(config);
    
    // Configura callbacks
    assetsSystem.SetAssetLoadedCallback(OnAssetLoaded);
    assetsSystem.SetAssetUnloadedCallback(OnAssetUnloaded);
    assetsSystem.SetAssetFailedCallback(OnAssetFailed);
    
    // Registra loader
    auto loader = std::make_unique<SimpleAssetLoader>();
    assetsSystem.RegisterLoader<SimpleAsset>(std::move(loader));
    
    // Fase 1: Pré-carregamento
    LOG_INFO("[AssetsExample] Fase 1: Pré-carregamento");
    std::vector<std::string> preloadPaths = {
        "textures/grass.asset", "textures/stone.asset", "textures/wood.asset",
        "models/tree.asset", "models/rock.asset", "models/house.asset"
    };
    
    for (const auto& path : preloadPaths) {
        DRIFT_PRELOAD_ASSET(SimpleAsset, path);
    }
    
    // Fase 2: Carregamento sob demanda
    LOG_INFO("[AssetsExample] Fase 2: Carregamento sob demanda");
    std::vector<std::shared_ptr<SimpleAsset>> onDemandAssets;
    
    for (int i = 0; i < 20; ++i) {
        std::string path = "on_demand_" + std::to_string(i) + ".asset";
        auto asset = DRIFT_GET_OR_LOAD_ASSET(SimpleAsset, path);
        if (asset) {
            onDemandAssets.push_back(asset);
        }
    }
    
    // Fase 3: Carregamento assíncrono
    LOG_INFO("[AssetsExample] Fase 3: Carregamento assíncrono");
    std::vector<std::future<std::shared_ptr<SimpleAsset>>> asyncAssets;
    
    for (int i = 0; i < 15; ++i) {
        std::string path = "async_" + std::to_string(i) + ".asset";
        auto future = DRIFT_LOAD_ASSET_ASYNC(SimpleAsset, path);
        asyncAssets.push_back(std::move(future));
    }
    
    // Aguarda assets assíncronos
    for (auto& future : asyncAssets) {
        future.get();
    }
    
    // Fase 4: Gerenciamento de cache
    LOG_INFO("[AssetsExample] Fase 4: Gerenciamento de cache");
    assetsSystem.LogStats();
    
    // Força limpeza
    assetsSystem.TrimCache();
    
    LOG_INFO("[AssetsExample] Cache após limpeza:");
    assetsSystem.LogStats();
    
    LOG_INFO("[AssetsExample] Exemplo completo concluído!");
}

std::vector<std::string> AssetsExample::GenerateAssetPaths(size_t count) {
    std::vector<std::string> paths;
    paths.reserve(count);
    
    std::vector<std::string> categories = {"textures", "models", "sounds", "shaders", "data"};
    std::vector<std::string> names = {"grass", "stone", "wood", "metal", "water", "fire", "tree", "rock", "house", "car"};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> catDist(0, categories.size() - 1);
    std::uniform_int_distribution<> nameDist(0, names.size() - 1);
    
    for (size_t i = 0; i < count; ++i) {
        std::string category = categories[catDist(gen)];
        std::string name = names[nameDist(gen)];
        std::string path = category + "/" + name + "_" + std::to_string(i) + ".asset";
        paths.push_back(path);
    }
    
    return paths;
}

void AssetsExample::OnAssetLoaded(const std::string& path, std::type_index type) {
            DRIFT_LOG_INFO("[AssetsExample] Asset carregado: ", path, " (", std::string(type.name()), ")");
}

void AssetsExample::OnAssetUnloaded(const std::string& path, std::type_index type) {
            DRIFT_LOG_INFO("[AssetsExample] Asset descarregado: ", path, " (", std::string(type.name()), ")");
}

void AssetsExample::OnAssetFailed(const std::string& path, std::type_index type, const std::string& error) {
    DRIFT_LOG_ERROR("[AssetsExample] Falha ao carregar asset: {} - {}", path, error);
}

} // namespace Drift::Core::Assets 
