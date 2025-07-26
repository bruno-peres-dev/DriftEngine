#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <vector>

namespace Drift::Core {

class Profiler {
public:
    static Profiler& GetInstance();
    
    // Inicia uma seção de profiling
    void BeginSection(const std::string& name);
    
    // Finaliza uma seção de profiling
    void EndSection(const std::string& name);
    
    // Obtém estatísticas de uma seção
    struct SectionStats {
        uint64_t callCount = 0;
        uint64_t totalTimeNs = 0;
        uint64_t minTimeNs = UINT64_MAX;
        uint64_t maxTimeNs = 0;
        uint64_t lastTimeNs = 0;
    };
    
    SectionStats GetSectionStats(const std::string& name) const;
    
    // Lista todas as seções
    std::vector<std::string> GetSectionNames() const;
    
    // Limpa todas as estatísticas
    void Clear();
    
    // Imprime relatório de performance
    void PrintReport() const;

private:
    Profiler() = default;
    
    struct SectionData {
        std::chrono::high_resolution_clock::time_point startTime;
        SectionStats stats;
    };
    
    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, SectionData> m_Sections;
};

// RAII helper para profiling automático
class ScopedProfiler {
public:
    ScopedProfiler(const std::string& name) : m_Name(name) {
        Profiler::GetInstance().BeginSection(name);
    }
    
    ~ScopedProfiler() {
        Profiler::GetInstance().EndSection(m_Name);
    }
    
private:
    std::string m_Name;
};

// Macros para facilitar o uso
#define PROFILE_SCOPE(name) ScopedProfiler profiler##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

} // namespace Drift::Core 