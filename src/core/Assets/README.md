# Sistema de AssetsManager - DriftEngine

O **AssetsManager** é um sistema genérico de gerenciamento de assets que permite carregar, cachear e gerenciar diferentes tipos de recursos como texturas, fontes, modelos, áudios, etc.

## Características Principais

- **Genérico**: Funciona com qualquer tipo de asset através de templates
- **Cache Inteligente**: Sistema LRU com limites de memória e quantidade
- **Lazy Loading**: Carregamento sob demanda
- **Pré-carregamento**: Suporte a pré-carregamento de assets críticos
- **Thread-Safe**: Operações seguras para threads
- **Estatísticas**: Sistema completo de métricas e debug
- **Callbacks**: Eventos de carregamento/descarregamento
- **Extensível**: Fácil adição de novos tipos de assets

## Arquitetura

### Componentes Principais

1. **AssetsManager**: Gerenciador principal (singleton)
2. **IAsset**: Interface base para todos os assets
3. **IAssetLoader<T>**: Interface para loaders específicos
4. **AssetKey**: Chave única para identificação (path + tipo + variante)
5. **AssetCacheEntry**: Entrada no cache com metadados

### Fluxo de Carregamento

```
LoadAsset<T>() -> Verifica Cache -> Não existe? -> Loader -> Cache -> Retorna Asset
                     ↓
                 Existe? -> Atualiza Stats -> Retorna Asset
```

## Como Usar

### 1. Configuração Inicial

```cpp
#include "Drift/Core/AssetsManager.h"
#include "Drift/Core/Assets/TextureAsset.h"
#include "Drift/Core/Assets/FontAsset.h"

// Obter instância singleton
auto& assetsManager = AssetsManager::GetInstance();

// Configurar cache
AssetCacheConfig config;
config.maxAssets = 1000;                    // Máximo de assets
config.maxMemoryUsage = 1024 * 1024 * 1024; // 1GB
config.enableLazyLoading = true;
config.enablePreloading = true;
config.trimThreshold = 0.8f;                // Limpa aos 80%

assetsManager.SetCacheConfig(config);

// Registrar loaders
assetsManager.RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(device));
assetsManager.RegisterLoader<FontAsset>(std::make_unique<FontLoader>());
```

### 2. Carregamento de Assets

#### Texturas

```cpp
// Carregamento básico
auto texture = assetsManager.LoadAsset<TextureAsset>("textures/grass.png");
if (texture) {
    auto rhiTexture = texture->GetTexture();
    // Usar textura...
}

// Com parâmetros específicos
TextureLoadParams params;
params.format = RHI::Format::R8G8B8A8_UNorm_sRGB;
params.generateMips = true;
params.sRGB = true;

auto logo = assetsManager.LoadAsset<TextureAsset>("textures/logo.png", "", params);

// Diferentes variantes (qualidades)
auto highQuality = assetsManager.LoadAsset<TextureAsset>("textures/icon.png", "high", params);
auto lowQuality = assetsManager.LoadAsset<TextureAsset>("textures/icon.png", "low", params);
```

#### Fontes

```cpp
// Carregamento básico (16pt, qualidade alta)
auto font = assetsManager.LoadAsset<FontAsset>("fonts/Arial-Regular.ttf");

// Diferentes tamanhos
FontLoadParams params24;
params24.size = 24.0f;
params24.quality = UI::FontQuality::High;
params24.name = "arial";

auto arial24 = assetsManager.LoadAsset<FontAsset>("fonts/Arial-Regular.ttf", "size_24", params24);

FontLoadParams params32;
params32.size = 32.0f;
params32.quality = UI::FontQuality::Ultra;

auto arial32 = assetsManager.LoadAsset<FontAsset>("fonts/Arial-Regular.ttf", "size_32", params32);
```

### 3. Operações de Cache

```cpp
// Obter asset existente (sem carregar)
auto existing = assetsManager.GetAsset<TextureAsset>("textures/grass.png");

// Lazy loading (carrega se não existir)
auto texture = assetsManager.GetOrLoadAsset<TextureAsset>("textures/lazy.png");

// Verificar se está carregado
bool loaded = assetsManager.IsAssetLoaded("textures/grass.png", 
                                         std::type_index(typeid(TextureAsset)));

// Pré-carregamento
assetsManager.PreloadAsset<TextureAsset>("textures/ui/button.png");

// Pré-carregamento em lote
std::vector<std::string> assets = {
    "textures/ui/panel.png",
    "textures/ui/cursor.png",
    "fonts/UI-Regular.ttf"
};
assetsManager.PreloadAssets(assets);
```

### 4. Gerenciamento de Memória

```cpp
// Remover assets não utilizados (ref_count == 1)
assetsManager.UnloadUnusedAssets();

// Remover todos os assets de um tipo
assetsManager.UnloadAssets(std::type_index(typeid(TextureAsset)));

// Remover asset específico
assetsManager.UnloadAsset("textures/temp.png", 
                         std::type_index(typeid(TextureAsset)));

// Limpeza automática (trim)
assetsManager.TrimCache();

// Limpar tudo
assetsManager.ClearCache();
```

### 5. Estatísticas e Debug

```cpp
// Log das estatísticas
assetsManager.LogCacheStats();

// Obter estatísticas programaticamente
auto stats = assetsManager.GetCacheStats();
std::cout << "Total assets: " << stats.totalAssets << std::endl;
std::cout << "Memory usage: " << stats.memoryUsage / (1024*1024) << " MB" << std::endl;
std::cout << "Cache hits: " << stats.cacheHits << std::endl;
std::cout << "Cache misses: " << stats.cacheMisses << std::endl;
std::cout << "Average load time: " << stats.averageLoadTime * 1000.0 << " ms" << std::endl;
```

### 6. Callbacks de Eventos

```cpp
// Configurar callbacks
assetsManager.SetAssetLoadedCallback([](const std::string& path, std::type_index type) {
    std::cout << "Asset loaded: " << path << " (" << type.name() << ")" << std::endl;
});

assetsManager.SetAssetUnloadedCallback([](const std::string& path, std::type_index type) {
    std::cout << "Asset unloaded: " << path << " (" << type.name() << ")" << std::endl;
});
```

## Criando Novos Tipos de Assets

### 1. Implementar IAsset

```cpp
class ModelAsset : public IAsset {
public:
    ModelAsset(const std::string& path) : m_Path(path) {}
    
    // IAsset implementation
    size_t GetMemoryUsage() const override { return m_MemoryUsage; }
    const std::string& GetPath() const override { return m_Path; }
    bool IsLoaded() const override { return m_Model != nullptr; }
    bool Load() override { /* implementar */ }
    void Unload() override { /* implementar */ }
    
    // Model-specific methods
    std::shared_ptr<Model> GetModel() const { return m_Model; }
    
private:
    std::string m_Path;
    std::shared_ptr<Model> m_Model;
    size_t m_MemoryUsage = 0;
};
```

### 2. Implementar IAssetLoader

```cpp
class ModelLoader : public IAssetLoader<ModelAsset> {
public:
    std::shared_ptr<ModelAsset> Load(const std::string& path, const std::any& params) override {
        // Implementar carregamento
        auto model = LoadModelFromFile(path);
        return std::make_shared<ModelAsset>(path, model);
    }
    
    bool CanLoad(const std::string& path) const override {
        // Verificar extensão
        return path.ends_with(".fbx") || path.ends_with(".obj") || path.ends_with(".gltf");
    }
    
    std::vector<std::string> GetSupportedExtensions() const override {
        return {".fbx", ".obj", ".gltf", ".dae"};
    }
};
```

### 3. Registrar o Loader

```cpp
assetsManager.RegisterLoader<ModelAsset>(std::make_unique<ModelLoader>());
```

### 4. Usar o Novo Asset

```cpp
auto model = assetsManager.LoadAsset<ModelAsset>("models/character.fbx");
if (model) {
    auto mesh = model->GetModel();
    // Usar modelo...
}
```

## Padrões de Uso Recomendados

### 1. Inicialização da Aplicação

```cpp
void InitializeAssets(RHI::IDevice* device) {
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Configurar cache
    AssetCacheConfig config;
    config.maxMemoryUsage = GetAvailableVideoMemory() * 0.7f; // 70% da VRAM
    assetsManager.SetCacheConfig(config);
    
    // Registrar loaders
    assetsManager.RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(device));
    assetsManager.RegisterLoader<FontAsset>(std::make_unique<FontLoader>());
    
    // Pré-carregar assets críticos
    std::vector<std::string> criticalAssets = {
        "textures/ui/loading.png",
        "fonts/UI-Regular.ttf"
    };
    assetsManager.PreloadAssets(criticalAssets);
}
```

### 2. Carregamento de Nível

```cpp
void LoadLevel(const std::string& levelName) {
    auto& assetsManager = AssetsManager::GetInstance();
    
    // Limpar assets do nível anterior
    assetsManager.UnloadUnusedAssets();
    
    // Carregar assets do novo nível
    auto levelConfig = LoadLevelConfig(levelName);
    for (const auto& assetPath : levelConfig.requiredAssets) {
        if (assetPath.ends_with(".png") || assetPath.ends_with(".jpg")) {
            assetsManager.PreloadAsset<TextureAsset>(assetPath);
        }
    }
}
```

### 3. Limpeza Periódica

```cpp
void UpdateAssets() {
    static auto lastCleanup = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    
    // Limpeza a cada 30 segundos
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastCleanup).count() > 30) {
        AssetsManager::GetInstance().UnloadUnusedAssets();
        lastCleanup = now;
    }
}
```

## Considerações de Performance

1. **Pré-carregamento**: Use para assets críticos que serão usados imediatamente
2. **Variantes**: Use para diferentes qualidades/configurações do mesmo asset
3. **Lazy Loading**: Deixe habilitado para assets não críticos
4. **Trim Threshold**: Configure baseado no uso de memória da aplicação
5. **Cache Size**: Balance entre velocidade de acesso e uso de memória

## Exemplo Completo

Veja `AssetsManagerExample.cpp` para um exemplo completo de uso do sistema.

## Limitações Atuais

- Carregamento assíncrono não implementado (TODO)
- Alguns métodos de loader não funcionam com std::any (limitação técnica)
- Serialização de cache não implementada
- Compressão de assets não suportada

## Extensões Futuras

- Carregamento assíncrono com thread pool
- Streaming de assets grandes
- Compressão automática
- Serialização de cache para disco
- Hot-reloading de assets em desenvolvimento
- Métricas avançadas de performance