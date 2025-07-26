// Drift/RHI/ResourceCache.h
#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include <list>
#include <optional>

namespace Drift::RHI {

    // Cache genérico de recursos compartilhados com LRU (Least Recently Used)
    // Dado uma chave Key e um valor T, armazena instâncias compartilhadas e reaproveita recursos.
    template<typename Key, typename T>
    class ResourceCache {
    public:
        explicit ResourceCache(size_t maxSize = 100) : m_MaxSize(maxSize) {}
        
        // Factory: callable que recebe (Key const&) e retorna std::shared_ptr<T>
        template<typename Factory>
        std::shared_ptr<T> GetOrCreate(Key const& key, Factory factory) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            auto it = m_Map.find(key);
            if (it != m_Map.end()) {
                // Move para o final da lista (mais recentemente usado)
                m_LRUList.splice(m_LRUList.end(), m_LRUList, it->second.lruIt);
                return it->second.resource;
            }
            
            // Cria novo recurso
            auto resource = factory(key);
            if (!resource) {
                return nullptr;
            }
            
            // Verifica se precisa remover recursos antigos
            if (m_Map.size() >= m_MaxSize) {
                EvictLRU();
            }
            
            // Adiciona à lista LRU
            m_LRUList.push_back(key);
            auto lruIt = std::prev(m_LRUList.end());
            
            // Armazena no mapa
            m_Map.emplace(key, CacheEntry{resource, lruIt});
            
            return resource;
        }

        // Remove um recurso do cache, se existir
        void Invalidate(Key const& key) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            auto it = m_Map.find(key);
            if (it != m_Map.end()) {
                m_LRUList.erase(it->second.lruIt);
                m_Map.erase(it);
            }
        }

        // Esvazia todo o cache
        void Clear() {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Map.clear();
            m_LRUList.clear();
        }
        
        // Obtém estatísticas do cache
        size_t GetSize() const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Map.size();
        }
        
        size_t GetMaxSize() const { return m_MaxSize; }
        
        // Verifica se uma chave existe no cache
        bool Contains(Key const& key) const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Map.find(key) != m_Map.end();
        }

    private:
        struct CacheEntry {
            std::shared_ptr<T> resource;
            typename std::list<Key>::iterator lruIt;
        };
        
        void EvictLRU() {
            if (m_LRUList.empty()) return;
            
            // Remove o elemento menos recentemente usado (primeiro da lista)
            auto keyToRemove = m_LRUList.front();
            m_LRUList.pop_front();
            m_Map.erase(keyToRemove);
        }
        
        std::unordered_map<Key, CacheEntry> m_Map;
        std::list<Key> m_LRUList; // Lista para LRU
        mutable std::mutex m_Mutex;
        size_t m_MaxSize;
    };

} // namespace Drift::RHI
