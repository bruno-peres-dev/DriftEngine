#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <vector>

namespace Drift::RHI {

    // Interface base para recursos
    class IResource {
    public:
        virtual ~IResource() = default;
        virtual void* GetBackendHandle() const = 0;
        virtual size_t GetMemoryUsage() const = 0;
        virtual bool IsValid() const = 0;
    };

    // Cache de recursos thread-safe com limpeza automática
    template<typename KeyType, typename ResourceType>
    class ResourceCache {
    public:
        using ResourceFactory = std::function<std::shared_ptr<ResourceType>(const KeyType&)>;
        
        explicit ResourceCache(ResourceFactory factory, size_t maxMemoryMB = 512)
            : _factory(std::move(factory))
            , _maxMemoryBytes(maxMemoryMB * 1024 * 1024)
            , _currentMemoryUsage(0)
        {}
        
        std::shared_ptr<ResourceType> GetOrCreate(const KeyType& key) {
            std::lock_guard<std::mutex> lock(_mutex);
            
            auto it = _resources.find(key);
            if (it != _resources.end()) {
                auto resource = it->second.lock();
                if (resource) {
                    return resource;
                } else {
                    // Resource foi destruído, remove do cache
                    _resources.erase(it);
                }
            }
            
            // Cria novo recurso
            auto resource = _factory(key);
            if (!resource) {
                return nullptr;
            }
            
            // Verifica se há memória suficiente
            size_t resourceMemory = resource->GetMemoryUsage();
            while (_currentMemoryUsage + resourceMemory > _maxMemoryBytes && !_resources.empty()) {
                EvictOldestResource();
            }
            
            // Adiciona ao cache
            _resources[key] = resource;
            _currentMemoryUsage += resourceMemory;
            
            return resource;
        }
        
        void Clear() {
            std::lock_guard<std::mutex> lock(_mutex);
            _resources.clear();
            _currentMemoryUsage = 0;
        }
        
        size_t GetMemoryUsage() const {
            std::lock_guard<std::mutex> lock(_mutex);
            return _currentMemoryUsage;
        }
        
        size_t GetResourceCount() const {
            std::lock_guard<std::mutex> lock(_mutex);
            return _resources.size();
        }
        
    private:
        void EvictOldestResource() {
            if (_resources.empty()) return;
            
            // Remove o primeiro recurso (mais antigo)
            auto it = _resources.begin();
            auto resource = it->second.lock();
            if (resource) {
                _currentMemoryUsage -= resource->GetMemoryUsage();
            }
            _resources.erase(it);
        }
        
        ResourceFactory _factory;
        mutable std::mutex _mutex;
        std::unordered_map<KeyType, std::weak_ptr<ResourceType>> _resources;
        size_t _maxMemoryBytes;
        size_t _currentMemoryUsage;
    };

    // Gerenciador de recursos por dispositivo
    class ResourceManager {
    public:
        explicit ResourceManager(void* device);
        ~ResourceManager();
        
        // Caches específicos por tipo de recurso
        template<typename KeyType, typename ResourceType>
        ResourceCache<KeyType, ResourceType>& GetCache() {
            // Implementação específica por tipo
            static_assert(sizeof(KeyType) == 0, "Specialize GetCache for specific resource types");
        }
        
        // Limpeza de recursos
        void ClearAllCaches();
        void SetMaxMemoryUsage(size_t maxMemoryMB);
        size_t GetTotalMemoryUsage() const;
        
        // Estatísticas
        struct CacheStats {
            size_t shaderCount = 0;
            size_t bufferCount = 0;
            size_t textureCount = 0;
            size_t pipelineCount = 0;
            size_t samplerCount = 0;
            size_t totalMemoryUsage = 0;
        };
        
        CacheStats GetStats() const;
        
    private:
        void* _device;
        mutable std::mutex _statsMutex;
        CacheStats _stats;
    };

} // namespace Drift::RHI 