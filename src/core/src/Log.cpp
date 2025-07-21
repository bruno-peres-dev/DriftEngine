#include "Drift/Core/Log.h"
#include <iostream>
namespace Drift::Core {
    void Log(const std::string& msg) {
        std::cout << "[Drift] " << msg << "\n";
    }
}
