#include "Drift/Core/Profiler.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace Drift::Core {

Profiler& Profiler::GetInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::BeginSection(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Sections[name].startTime = std::chrono::high_resolution_clock::now();
}

void Profiler::EndSection(const std::string& name) {
    auto endTime = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Sections.find(name);
    if (it != m_Sections.end()) {
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - it->second.startTime);
        uint64_t timeNs = duration.count();
        
        auto& stats = it->second.stats;
        stats.callCount++;
        stats.totalTimeNs += timeNs;
        stats.minTimeNs = std::min(stats.minTimeNs, timeNs);
        stats.maxTimeNs = std::max(stats.maxTimeNs, timeNs);
        stats.lastTimeNs = timeNs;
    }
}

Profiler::SectionStats Profiler::GetSectionStats(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Sections.find(name);
    if (it != m_Sections.end()) {
        return it->second.stats;
    }
    return SectionStats{};
}

std::vector<std::string> Profiler::GetSectionNames() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<std::string> names;
    names.reserve(m_Sections.size());
    
    for (const auto& pair : m_Sections) {
        names.push_back(pair.first);
    }
    
    return names;
}

void Profiler::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Sections.clear();
}

void Profiler::PrintReport() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (m_Sections.empty()) {
        Log("Profiler: Nenhuma seção registrada");
        return;
    }
    
    Log("=== RELATÓRIO DE PERFORMANCE ===");
    
    // Converte para vector para ordenar
    std::vector<std::pair<std::string, SectionStats>> sortedSections;
    sortedSections.reserve(m_Sections.size());
    
    for (const auto& pair : m_Sections) {
        sortedSections.emplace_back(pair.first, pair.second.stats);
    }
    
    // Ordena por tempo total (mais lento primeiro)
    std::sort(sortedSections.begin(), sortedSections.end(),
        [](const auto& a, const auto& b) {
            return a.second.totalTimeNs > b.second.totalTimeNs;
        });
    
    for (const auto& [name, stats] : sortedSections) {
        if (stats.callCount == 0) continue;
        
        double avgTimeMs = (stats.totalTimeNs / stats.callCount) / 1000000.0;
        double totalTimeMs = stats.totalTimeNs / 1000000.0;
        double minTimeMs = stats.minTimeNs / 1000000.0;
        double maxTimeMs = stats.maxTimeNs / 1000000.0;
        
        std::stringstream ss;
        ss << std::fixed << std::setprecision(3)
           << name << ": "
           << stats.callCount << " chamadas, "
           << "média: " << avgTimeMs << "ms, "
           << "total: " << totalTimeMs << "ms, "
           << "min: " << minTimeMs << "ms, "
           << "max: " << maxTimeMs << "ms";
        
        Log(ss.str());
    }
    
    Log("================================");
}

} // namespace Drift::Core 