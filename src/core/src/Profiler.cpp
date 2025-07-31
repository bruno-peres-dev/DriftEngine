#include "Drift/Core/Profiler.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <cmath>
#include <ctime>

namespace Drift::Core {

// Thread-local storage para seções ativas
thread_local std::vector<ActiveSection> Profiler::s_ActiveSections;

// Implementação do ConsoleProfilerOutput
void ConsoleProfilerOutput::WriteReport(const std::string& report) {
    std::cout << report << std::endl;
}

void ConsoleProfilerOutput::WriteSection(const std::string& name, const SectionStats& stats) {
    std::cout << "  " << name << ": " << stats.callCount << " calls, avg: " 
              << std::fixed << std::setprecision(3) << stats.GetAverageTimeMs() 
              << "ms, total: " << stats.GetTotalTimeMs() << "ms" << std::endl;
}

// Implementação do FileProfilerOutput
FileProfilerOutput::FileProfilerOutput(const std::string& filename) : m_Filename(filename) {
    m_File.open(filename, std::ios::app);
    if (m_File.is_open()) {
        m_File << "\n=== Profiler Report iniciado em " << Drift::Core::GetTimestamp() << " ===" << std::endl;
    }
}

FileProfilerOutput::~FileProfilerOutput() {
    if (m_File.is_open()) {
        m_File.close();
    }
}

void FileProfilerOutput::WriteReport(const std::string& report) {
    if (m_File.is_open()) {
        m_File << report << std::endl;
        m_File.flush();
    }
}

void FileProfilerOutput::WriteSection(const std::string& name, const SectionStats& stats) {
    if (m_File.is_open()) {
        m_File << "  " << name << ": " << stats.callCount << " calls, avg: " 
               << std::fixed << std::setprecision(3) << stats.GetAverageTimeMs() 
               << "ms, total: " << stats.GetTotalTimeMs() << "ms" << std::endl;
        m_File.flush();
    }
}

// Implementação do Profiler
Profiler& Profiler::GetInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::Configure(const ProfilerConfig& config) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Config = config;
    
    // Adicionar output padrão se não houver nenhum
    if (m_Outputs.empty()) {
        m_Outputs.push_back(std::make_shared<ConsoleProfilerOutput>());
    }
    
    // Adicionar output de arquivo se especificado
    if (!config.outputFile.empty()) {
        m_Outputs.push_back(std::make_shared<FileProfilerOutput>(config.outputFile));
    }
}

void Profiler::SetEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Config.enableProfiling = enabled;
}

void Profiler::AddOutput(std::shared_ptr<IProfilerOutput> output) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Outputs.push_back(output);
}

void Profiler::RemoveOutput(std::shared_ptr<IProfilerOutput> output) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Outputs.erase(
        std::remove(m_Outputs.begin(), m_Outputs.end(), output),
        m_Outputs.end()
    );
}

void Profiler::BeginSection(const std::string& name) {
    if (!m_Config.enableProfiling) return;
    
    std::string parent = "";
    uint32_t depth = 0;
    
    if (!s_ActiveSections.empty()) {
        parent = s_ActiveSections.back().name;
        depth = s_ActiveSections.back().depth + 1;
    }
    
    BeginSection(name, parent);
}

void Profiler::EndSection(const std::string& name) {
    if (!m_Config.enableProfiling) return;
    
    if (s_ActiveSections.empty() || s_ActiveSections.back().name != name) {
        return; // Seção não encontrada ou não iniciada
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto& activeSection = s_ActiveSections.back();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - activeSection.startTime);
    uint64_t timeNs = duration.count();
    
    UpdateStats(name, timeNs, activeSection.parentSection, activeSection.depth);
    
    if (m_Config.enableMemoryProfiling) {
        size_t currentMemory = GetCurrentMemoryUsage();
        size_t memoryUsed = currentMemory - activeSection.memorySnapshot;
        UpdateMemoryStats(name, memoryUsed);
    }
    
    s_ActiveSections.pop_back();
}

void Profiler::BeginSection(const std::string& name, const std::string& parent) {
    if (!m_Config.enableProfiling) return;
    
    uint32_t depth = 0;
    if (!parent.empty()) {
        depth = 1;
        for (const auto& section : s_ActiveSections) {
            if (section.name == parent) {
                depth = section.depth + 1;
                break;
            }
        }
    }
    
    if (depth >= m_Config.maxDepth) {
        return; // Profundidade máxima atingida
    }
    
    ActiveSection newSection(name, parent, depth);
    
    if (m_Config.enableMemoryProfiling) {
        newSection.memorySnapshot = GetCurrentMemoryUsage();
    }
    
    s_ActiveSections.push_back(newSection);
}

void Profiler::EndSection(const std::string& name, const std::string& parent) {
    EndSection(name); // A implementação atual não usa o parent
}

SectionStats Profiler::GetSectionStats(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Sections.find(name);
    if (it != m_Sections.end()) {
        return it->second;
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

std::vector<std::pair<std::string, SectionStats>> Profiler::GetAllStats() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<std::pair<std::string, SectionStats>> result;
    result.reserve(m_Sections.size());
    
    for (const auto& pair : m_Sections) {
        result.emplace_back(pair.first, pair.second);
    }
    
    return result;
}

void Profiler::PrintReport() const {
    std::string report = GenerateReport();
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& output : m_Outputs) {
        output->WriteReport(report);
    }
}

void Profiler::ExportReport(const std::string& filename) const {
    std::string report = GenerateReport();
    
    std::ofstream file(filename);
    if (file.is_open()) {
        file << report << std::endl;
        file.close();
    }
}

std::string Profiler::GenerateReport() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (m_Sections.empty()) {
        return "Profiler: Nenhuma seção registrada";
    }
    
    std::stringstream ss;
    ss << "\n=== RELATÓRIO DE PERFORMANCE ===" << std::endl;
    ss << "Gerado em: " << Drift::Core::GetTimestamp() << std::endl;
    ss << "Total de seções: " << m_Sections.size() << std::endl;
    ss << std::endl;
    
    // Converte para vector para ordenar
    std::vector<std::pair<std::string, SectionStats>> sortedSections;
    sortedSections.reserve(m_Sections.size());
    
    for (const auto& pair : m_Sections) {
        sortedSections.emplace_back(pair.first, pair.second);
    }
    
    // Ordena por tempo total (mais lento primeiro)
    std::sort(sortedSections.begin(), sortedSections.end(),
        [](const auto& a, const auto& b) {
            return a.second.totalTimeNs > b.second.totalTimeNs;
        });
    
    ss << std::left << std::setw(30) << "Seção" 
       << std::setw(8) << "Calls" 
       << std::setw(12) << "Avg (ms)" 
       << std::setw(12) << "Total (ms)" 
       << std::setw(12) << "Min (ms)" 
       << std::setw(12) << "Max (ms)" 
       << std::setw(8) << "Depth" << std::endl;
    
    ss << std::string(100, '-') << std::endl;
    
    for (const auto& [name, stats] : sortedSections) {
        if (stats.callCount == 0) continue;
        
        ss << std::left << std::setw(30) << name
           << std::setw(8) << stats.callCount
           << std::setw(12) << std::fixed << std::setprecision(3) << stats.GetAverageTimeMs()
           << std::setw(12) << std::fixed << std::setprecision(3) << stats.GetTotalTimeMs()
           << std::setw(12) << std::fixed << std::setprecision(3) << stats.GetMinTimeMs()
           << std::setw(12) << std::fixed << std::setprecision(3) << stats.GetMaxTimeMs()
           << std::setw(8) << stats.depth << std::endl;
    }
    
    ss << std::string(100, '-') << std::endl;
    ss << "================================" << std::endl;
    
    return ss.str();
}

void Profiler::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Sections.clear();
}

void Profiler::Reset() {
    Clear();
    m_ThreadCounter = 0;
    m_ThreadIndices.clear();
}

uint64_t Profiler::GetCurrentTimeNs() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

size_t Profiler::GetCurrentMemoryUsage() const {
    // Implementação básica - pode ser expandida para usar APIs específicas do sistema
    return 0;
}

void Profiler::UpdateStats(const std::string& name, uint64_t durationNs, const std::string& parent, uint32_t depth) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto& stats = m_Sections[name];
    
    if (stats.callCount == 0) {
        stats.firstCall = std::chrono::steady_clock::now();
        stats.threadId = std::this_thread::get_id();
        
        // Atribuir índice de thread se não existir
        auto threadIt = m_ThreadIndices.find(stats.threadId);
        if (threadIt == m_ThreadIndices.end()) {
            stats.threadIndex = m_ThreadCounter++;
            m_ThreadIndices[stats.threadId] = stats.threadIndex;
        } else {
            stats.threadIndex = threadIt->second;
        }
    }
    
    stats.callCount++;
    stats.totalTimeNs += durationNs;
    stats.minTimeNs = std::min(stats.minTimeNs, durationNs);
    stats.maxTimeNs = std::max(stats.maxTimeNs, durationNs);
    stats.lastTimeNs = durationNs;
    stats.lastCall = std::chrono::steady_clock::now();
    stats.depth = depth;
    stats.parentSection = parent;
    
    // Atualizar estatísticas avançadas
    stats.UpdateVariance(durationNs);
}

void Profiler::UpdateMemoryStats(const std::string& name, size_t memoryUsage) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto& stats = m_Sections[name];
    stats.totalMemoryAllocated += memoryUsage;
    stats.peakMemoryUsage = std::max(stats.peakMemoryUsage, memoryUsage);
    stats.currentMemoryUsage = memoryUsage;
}

std::string Profiler::FormatDuration(uint64_t nanoseconds) const {
    if (nanoseconds < 1000) {
        return std::to_string(nanoseconds) + " ns";
    } else if (nanoseconds < 1000000) {
        return std::to_string(nanoseconds / 1000.0) + " μs";
    } else if (nanoseconds < 1000000000) {
        return std::to_string(nanoseconds / 1000000.0) + " ms";
    } else {
        return std::to_string(nanoseconds / 1000000000.0) + " s";
    }
}

std::string Profiler::FormatMemory(size_t bytes) const {
    if (bytes < 1024) {
        return std::to_string(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return std::to_string(bytes / 1024.0) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return std::to_string(bytes / (1024.0 * 1024.0)) + " MB";
    } else {
        return std::to_string(bytes / (1024.0 * 1024.0 * 1024.0)) + " GB";
    }
}

std::string Profiler::GetThreadName(std::thread::id threadId) const {
    auto it = m_ThreadIndices.find(threadId);
    if (it != m_ThreadIndices.end()) {
        return "Thread-" + std::to_string(it->second);
    }
    return "Thread-Unknown";
}

// Implementação do ScopedProfiler
ScopedProfiler::ScopedProfiler(const std::string& name, const std::string& parent)
    : m_Name(name), m_Parent(parent), m_IsActive(true) {
    Profiler::GetInstance().BeginSection(name, parent);
    m_StartTime = std::chrono::high_resolution_clock::now();
    m_StartMemory = Profiler::GetInstance().GetCurrentMemoryUsage();
}

ScopedProfiler::~ScopedProfiler() {
    if (m_IsActive) {
        End();
    }
}

void ScopedProfiler::End() {
    if (m_IsActive) {
        Profiler::GetInstance().EndSection(m_Name);
        m_IsActive = false;
    }
}

// Implementação do MemoryProfiler
MemoryProfiler& MemoryProfiler::GetInstance() {
    static MemoryProfiler instance;
    return instance;
}

void MemoryProfiler::TrackAllocation(size_t size, const std::string& context) {
    m_CurrentUsage.fetch_add(size);
    
    size_t current = m_CurrentUsage.load();
    size_t peak = m_PeakUsage.load();
    
    while (current > peak && !m_PeakUsage.compare_exchange_weak(peak, current)) {
        // Loop até conseguir atualizar o peak
    }
    
    if (!context.empty()) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_AllocationByContext[context] += size;
    }
}

void MemoryProfiler::TrackDeallocation(size_t size, const std::string& context) {
    m_CurrentUsage.fetch_sub(size);
    
    if (!context.empty()) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_AllocationByContext.find(context);
        if (it != m_AllocationByContext.end()) {
            if (it->second <= size) {
                m_AllocationByContext.erase(it);
            } else {
                it->second -= size;
            }
        }
    }
}

void MemoryProfiler::Reset() {
    m_CurrentUsage = 0;
    m_PeakUsage = 0;
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_AllocationByContext.clear();
}

// Implementação dos métodos da SectionStats
void SectionStats::UpdateVariance(uint64_t newTimeNs) {
    if (callCount == 1) {
        averageTimeNs = static_cast<double>(newTimeNs);
        varianceNs = 0.0;
    } else {
        double oldAverage = averageTimeNs;
        averageTimeNs = (averageTimeNs * (callCount - 1) + newTimeNs) / callCount;
        varianceNs = ((varianceNs * (callCount - 2) + (newTimeNs - oldAverage) * (newTimeNs - averageTimeNs)) / (callCount - 1));
    }
    standardDeviationNs = std::sqrt(varianceNs);
}

void SectionStats::Reset() {
    callCount = 0;
    totalTimeNs = 0;
    minTimeNs = UINT64_MAX;
    maxTimeNs = 0;
    lastTimeNs = 0;
    averageTimeNs = 0.0;
    varianceNs = 0.0;
    standardDeviationNs = 0.0;
    totalMemoryAllocated = 0;
    peakMemoryUsage = 0;
    currentMemoryUsage = 0;
    threadIndex = 0;
    depth = 0;
    parentSection.clear();
    childSections.clear();
}

} // namespace Drift::Core 