// Drift/RHI/ResourceCache.h
#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>

namespace Drift::RHI {

    /// Cache genérico de recurso: dado uma chave Key e 
    /// um valor compartilhado T, guarda e reaproveita instâncias.
    template<typename Key, typename T>
    class ResourceCache {
    public:
        /// Factory é uma callable que recebe (Key const&) e devolve std::shared_ptr<T>
        template<typename Factory>
        std::shared_ptr<T> GetOrCreate(Key const& key, Factory factory) {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _map.find(key);
            if (it != _map.end()) {
                return it->second;
            }
            auto resource = factory(key);
            _map.emplace(key, resource);
            return resource;
        }

        /// Remove um recurso do cache (se existir)
        void Invalidate(Key const& key) {
            std::lock_guard<std::mutex> lock(_mutex);
            _map.erase(key);
        }

        /// Esvazia todo o cache
        void Clear() {
            std::lock_guard<std::mutex> lock(_mutex);
            _map.clear();
        }

    private:
        std::unordered_map<Key, std::shared_ptr<T>> _map;
        std::mutex                                 _mutex;
    };

} // namespace Drift::RHI
