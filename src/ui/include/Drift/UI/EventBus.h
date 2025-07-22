#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <cstdint>
#include <typeindex>
#include <algorithm>

namespace Drift::UI {

// Identificador genérico de callback
using HandlerId = std::uint64_t;

class EventBus {
public:
    EventBus() = default;
    ~EventBus() = default;

    template <typename EventT>
    HandlerId Subscribe(std::function<void(const EventT&)> handler)
    {
        const std::type_index type = typeid(EventT);
        auto& vector = m_Handlers[type];
        HandlerId id = ++m_NextId;
        vector.emplace_back(id, [func = std::move(handler)](const void* evt)
        {
            func(*static_cast<const EventT*>(evt));
        });
        return id;
    }

    template <typename EventT>
    void Unsubscribe(HandlerId id)
    {
        const std::type_index type = typeid(EventT);
        auto it = m_Handlers.find(type);
        if (it == m_Handlers.end())
            return;
        auto& vector = it->second;
        vector.erase(std::remove_if(vector.begin(), vector.end(), [id](auto& pair) { return pair.first == id; }), vector.end());
    }

    template <typename EventT>
    void Publish(const EventT& event)
    {
        const std::type_index type = typeid(EventT);
        auto it = m_Handlers.find(type);
        if (it == m_Handlers.end())
            return;
        for (auto& [id, fn] : it->second)
            fn(&event);
    }

private:
    using HandlerWrapper = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<std::pair<HandlerId, HandlerWrapper>>> m_Handlers;
    HandlerId m_NextId{0};
};

} // namespace Drift::UI 