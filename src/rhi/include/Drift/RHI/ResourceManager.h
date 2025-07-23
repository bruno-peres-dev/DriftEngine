#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <cstddef>
#include "Drift/RHI/Resource.h"
#include "Drift/RHI/DepthStencilState.h"
#include "Drift/RHI/Shader.h"
#include "Drift/RHI/Buffer.h"
#include "Drift/RHI/PipelineState.h"
#include "Drift/RHI/Texture.h"

namespace Drift::RHI {



    // Cache genérico para recursos
    template<typename Key, typename Resource>
    class ResourceCache {
    private:
        struct CacheEntry {
            std::shared_ptr<Resource> resource;
            size_t lastAccess = 0;
            size_t accessCount = 0;
            size_t memoryUsage = 0; // Armazena o uso de memória calculado
        };

        std::unordered_map<Key, CacheEntry> _resources;
        mutable std::mutex _mutex;
        size_t _maxSize = 1000;
        size_t _maxMemoryUsage = 1024 * 1024 * 1024; // 1GB default
        size_t _currentMemoryUsage = 0;
        size_t _accessCounter = 0;

    public:
        // Obtém ou cria um recurso
        std::shared_ptr<Resource> GetOrCreate(const Key& key, std::function<std::shared_ptr<Resource>()> factory) {
            std::lock_guard<std::mutex> lock(_mutex);
            
            auto it = _resources.find(key);
            if (it != _resources.end()) {
                // Recurso encontrado - atualiza estatísticas
                it->second.lastAccess = ++_accessCounter;
                it->second.accessCount++;
                return it->second.resource;
            }

            // Recurso não encontrado - cria novo
            auto resource = factory();
            if (!resource) {
                return nullptr;
            }

            // Calcula o uso de memória usando a interface IResource
            size_t resourceMemory = resource->GetMemoryUsage();

            // Verifica se há espaço disponível
            if (_currentMemoryUsage + resourceMemory > _maxMemoryUsage) {
                // Tenta liberar recursos menos usados
                if (!EvictLeastUsed()) {
                    return nullptr; // Não conseguiu liberar espaço
                }
            }

            // Adiciona ao cache
            CacheEntry entry;
            entry.resource = resource;
            entry.lastAccess = ++_accessCounter;
            entry.accessCount = 1;
            entry.memoryUsage = resourceMemory;
            
            _resources[key] = entry;
            _currentMemoryUsage += resourceMemory;

            // Verifica limite de quantidade
            if (_resources.size() > _maxSize) {
                EvictLeastUsed();
            }

            return resource;
        }

        // Remove um recurso específico
        void Remove(const Key& key) {
            std::lock_guard<std::mutex> lock(_mutex);
            
            auto it = _resources.find(key);
            if (it != _resources.end()) {
                _currentMemoryUsage -= it->second.memoryUsage;
                _resources.erase(it);
            }
        }

        // Limpa todos os recursos
        void Clear() {
            std::lock_guard<std::mutex> lock(_mutex);
            _resources.clear();
            _currentMemoryUsage = 0;
        }

        // Define o tamanho máximo do cache
        void SetMaxSize(size_t maxSize) {
            std::lock_guard<std::mutex> lock(_mutex);
            _maxSize = maxSize;
            
            // Remove recursos excedentes
            while (_resources.size() > _maxSize) {
                EvictLeastUsed();
            }
        }

        // Define o uso máximo de memória
        void SetMaxMemoryUsage(size_t maxMemoryUsage) {
            std::lock_guard<std::mutex> lock(_mutex);
            _maxMemoryUsage = maxMemoryUsage;
            
            // Remove recursos excedentes
            while (_currentMemoryUsage > _maxMemoryUsage) {
                if (!EvictLeastUsed()) {
                    break;
                }
            }
        }

        // Obtém estatísticas do cache
        struct CacheStats {
            size_t resourceCount = 0;
            size_t memoryUsage = 0;
            size_t maxSize = 0;
            size_t maxMemoryUsage = 0;
        };

        CacheStats GetStats() const {
            std::lock_guard<std::mutex> lock(_mutex);
            CacheStats stats;
            stats.resourceCount = _resources.size();
            stats.memoryUsage = _currentMemoryUsage;
            stats.maxSize = _maxSize;
            stats.maxMemoryUsage = _maxMemoryUsage;
            return stats;
        }

    private:
        // Remove o recurso menos usado (LRU)
        bool EvictLeastUsed() {
            if (_resources.empty()) {
                return false;
            }

            auto leastUsed = _resources.begin();
            for (auto it = _resources.begin(); it != _resources.end(); ++it) {
                if (it->second.lastAccess < leastUsed->second.lastAccess) {
                    leastUsed = it;
                }
            }

            _currentMemoryUsage -= leastUsed->second.memoryUsage;
            _resources.erase(leastUsed);
            return true;
        }
    };

    // Gerenciador de recursos por dispositivo
    class ResourceManager {
    private:
        struct DeviceCaches {
            ResourceCache<struct ShaderDesc, class IShader> shaderCache;
            ResourceCache<struct BufferDesc, class IBuffer> bufferCache;
            ResourceCache<struct PipelineDesc, class IPipelineState> pipelineCache;
            ResourceCache<struct TextureDesc, class ITexture> textureCache;
            ResourceCache<struct SamplerDesc, class ISampler> samplerCache;
            ResourceCache<struct DepthStencilDesc, class DepthStencilState> depthStencilCache;
        };

        std::unordered_map<void*, std::unique_ptr<DeviceCaches>> _deviceCaches;
        mutable std::mutex _mutex;

    public:
        // Obtém cache para um tipo específico de recurso
        template<typename Key, typename Resource>
        ResourceCache<Key, Resource>& GetCache(void* device) {
            std::lock_guard<std::mutex> lock(_mutex);
            
            auto& deviceCache = _deviceCaches[device];
            if (!deviceCache) {
                deviceCache = std::make_unique<DeviceCaches>();
            }

            // Retorna o cache apropriado baseado no tipo
            if constexpr (std::is_same_v<Resource, class IShader>) {
                return deviceCache->shaderCache;
            } else if constexpr (std::is_same_v<Resource, class IBuffer>) {
                return deviceCache->bufferCache;
            } else if constexpr (std::is_same_v<Resource, class IPipelineState>) {
                return deviceCache->pipelineCache;
            } else if constexpr (std::is_same_v<Resource, class ITexture>) {
                return deviceCache->textureCache;
            } else if constexpr (std::is_same_v<Resource, class ISampler>) {
                return deviceCache->samplerCache;
            } else if constexpr (std::is_same_v<Resource, class DepthStencilState>) {
                return deviceCache->depthStencilCache;
            } else {
                static_assert(false, "Unsupported resource type");
            }
        }

        // Remove todos os caches de um dispositivo
        void RemoveDevice(void* device) {
            std::lock_guard<std::mutex> lock(_mutex);
            _deviceCaches.erase(device);
        }

        // Limpa todos os caches
        void ClearAll() {
            std::lock_guard<std::mutex> lock(_mutex);
            _deviceCaches.clear();
        }

        // Obtém estatísticas de todos os caches
        struct GlobalStats {
            size_t deviceCount = 0;
            size_t totalResources = 0;
            size_t totalMemoryUsage = 0;
        };

        GlobalStats GetGlobalStats() const {
            std::lock_guard<std::mutex> lock(_mutex);
            GlobalStats stats;
            stats.deviceCount = _deviceCaches.size();
            
            for (const auto& [device, caches] : _deviceCaches) {
                auto shaderStats = caches->shaderCache.GetStats();
                auto bufferStats = caches->bufferCache.GetStats();
                auto pipelineStats = caches->pipelineCache.GetStats();
                auto textureStats = caches->textureCache.GetStats();
                auto samplerStats = caches->samplerCache.GetStats();
                auto depthStencilStats = caches->depthStencilCache.GetStats();
                
                stats.totalResources += shaderStats.resourceCount + bufferStats.resourceCount + 
                                       pipelineStats.resourceCount + textureStats.resourceCount + 
                                       samplerStats.resourceCount + depthStencilStats.resourceCount;
                stats.totalMemoryUsage += shaderStats.memoryUsage + bufferStats.memoryUsage + 
                                         pipelineStats.memoryUsage + textureStats.memoryUsage + 
                                         samplerStats.memoryUsage + depthStencilStats.memoryUsage;
            }
            
            return stats;
        }
    };

    // Instância global do Resource Manager
    extern ResourceManager g_resourceManager;

} // namespace Drift::RHI 