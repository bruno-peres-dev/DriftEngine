#pragma once

#include <cstddef>

namespace Drift::RHI {

    // Interface base para recursos
    class IResource {
    public:
        virtual ~IResource() = default;
        virtual void* GetBackendHandle() const = 0;
        virtual size_t GetMemoryUsage() const = 0;
    };

} // namespace Drift::RHI 