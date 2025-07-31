#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include <fstream>

namespace Drift::Core {

// Configuração do profiler
struct ProfilerConfig {
    bool enableProfiling = true;
    bool enableThreadProfiling = false;
    bool enableMemoryProfiling = false;
    bool enableCallStack = false;
    size_t maxSections = 1000;
    size_t maxDepth = 32;
    std::string outputFile = "";
    std::function<void(const std::string&)> customOutput = nullptr;
};

// Estatísticas detalhadas de uma seção
struct SectionStats {
    // Contadores básicos
    uint64_t callCount = 0;
    uint64_t totalTimeNs = 0;
    uint64_t minTimeNs = UINT64_MAX;
    uint64_t maxTimeNs = 0;
    uint64_t lastTimeNs = 0;
    
    // Estatísticas avançadas
    double averageTimeNs = 0.0;
    double varianceNs = 0.0;
    double standardDeviationNs = 0.0;
    
    // Informações de memória (se habilitado)
    size_t totalMemoryAllocated = 0;
    size_t peakMemoryUsage = 0;
    size_t currentMemoryUsage = 0;
    
    // Informações de thread
    std::thread::id threadId;
    uint32_t threadIndex = 0;
    
    // Timestamps
    std::chrono::steady_clock::time_point firstCall;
    std::chrono::steady_clock::time_point lastCall;
    
    // Hierarquia
    std::string parentSection;
    std::vector<std::string> childSections;
    uint32_t depth = 0;
    
    // Métodos utilitários
    double GetAverageTimeMs() const { return averageTimeNs / 1000000.0; }
    double GetTotalTimeMs() const { return totalTimeNs / 1000000.0; }
    double GetMinTimeMs() const { return minTimeNs / 1000000.0; }
    double GetMaxTimeMs() const { return maxTimeNs / 1000000.0; }
    double GetLastTimeMs() const { return lastTimeNs / 1000000.0; }
    double GetStandardDeviationMs() const { return standardDeviationNs / 1000000.0; }
    
    void UpdateVariance(uint64_t newTimeNs);
    void Reset();
};

// Informações de uma seção ativa
struct ActiveSection {
    std::string name;
    std::chrono::high_resolution_clock::time_point startTime;
    std::string parentSection;
    uint32_t depth;
    std::thread::id threadId;
    size_t memorySnapshot;
    
    ActiveSection(const std::string& n, const std::string& parent = "", uint32_t d = 0)
        : name(n), startTime(std::chrono::high_resolution_clock::now())
        , parentSection(parent), depth(d), threadId(std::this_thread::get_id())
        , memorySnapshot(0) {}
};

// Interface para output do profiler
class IProfilerOutput {
public:
    virtual ~IProfilerOutput() = default;
    virtual void WriteReport(const std::string& report) = 0;
    virtual void WriteSection(const std::string& name, const SectionStats& stats) = 0;
};

// Output padrão para console
class ConsoleProfilerOutput : public IProfilerOutput {
public:
    void WriteReport(const std::string& report) override;
    void WriteSection(const std::string& name, const SectionStats& stats) override;
};

// Output para arquivo
class FileProfilerOutput : public IProfilerOutput {
public:
    explicit FileProfilerOutput(const std::string& filename);
    ~FileProfilerOutput();
    void WriteReport(const std::string& report) override;
    void WriteSection(const std::string& name, const SectionStats& stats) override;

private:
    std::string m_Filename;
    std::ofstream m_File;
};

// Sistema de profiler principal
class Profiler {
public:
    static Profiler& GetInstance();
    
    // Configuração
    void Configure(const ProfilerConfig& config);
    void SetEnabled(bool enabled);
    void AddOutput(std::shared_ptr<IProfilerOutput> output);
    void RemoveOutput(std::shared_ptr<IProfilerOutput> output);
    
    // Controle de seções
    void BeginSection(const std::string& name);
    void EndSection(const std::string& name);
    
    // Controle de seções com contexto
    void BeginSection(const std::string& name, const std::string& parent);
    void EndSection(const std::string& name, const std::string& parent);
    
    // Consulta de estatísticas
    SectionStats GetSectionStats(const std::string& name) const;
    std::vector<std::string> GetSectionNames() const;
    std::vector<std::pair<std::string, SectionStats>> GetAllStats() const;
    
    // Relatórios
    void PrintReport() const;
    void ExportReport(const std::string& filename) const;
    std::string GenerateReport() const;
    
    // Controle de dados
    void Clear();
    void Reset();
    
    // Utilitários
    bool IsEnabled() const { return m_Config.enableProfiling; }
    uint64_t GetCurrentTimeNs() const;
    size_t GetCurrentMemoryUsage() const;
    
    // Thread-local storage para seções ativas
    static thread_local std::vector<ActiveSection> s_ActiveSections;

private:
    Profiler() = default;
    ~Profiler() = default;
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;
    
    void UpdateStats(const std::string& name, uint64_t durationNs, const std::string& parent = "", uint32_t depth = 0);
    void UpdateMemoryStats(const std::string& name, size_t memoryUsage);
    std::string FormatDuration(uint64_t nanoseconds) const;
    std::string FormatMemory(size_t bytes) const;
    std::string GetThreadName(std::thread::id threadId) const;
    
    ProfilerConfig m_Config;
    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, SectionStats> m_Sections;
    std::vector<std::shared_ptr<IProfilerOutput>> m_Outputs;
    std::atomic<uint32_t> m_ThreadCounter{0};
    std::unordered_map<std::thread::id, uint32_t> m_ThreadIndices;
};

// RAII helper para profiling automático
class ScopedProfiler {
public:
    ScopedProfiler(const std::string& name, const std::string& parent = "");
    ~ScopedProfiler();
    
    // Métodos para profiling manual
    void End();
    bool IsActive() const { return m_IsActive; }

private:
    std::string m_Name;
    std::string m_Parent;
    bool m_IsActive;
    std::chrono::high_resolution_clock::time_point m_StartTime;
    size_t m_StartMemory;
};

// Profiler de memória
class MemoryProfiler {
public:
    static MemoryProfiler& GetInstance();
    
    void TrackAllocation(size_t size, const std::string& context = "");
    void TrackDeallocation(size_t size, const std::string& context = "");
    size_t GetCurrentUsage() const { return m_CurrentUsage.load(); }
    size_t GetPeakUsage() const { return m_PeakUsage.load(); }
    void Reset();
    
private:
    MemoryProfiler() = default;
    std::atomic<size_t> m_CurrentUsage{0};
    std::atomic<size_t> m_PeakUsage{0};
    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, size_t> m_AllocationByContext;
};

// Macros para facilitar o uso
#define PROFILE_SCOPE(name) Drift::Core::ScopedProfiler profiler##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)
#define PROFILE_SCOPE_WITH_PARENT(name, parent) Drift::Core::ScopedProfiler profiler##__LINE__(name, parent)

// Macros DRIFT_PROFILE_* para compatibilidade com o sistema de fontes
#define DRIFT_PROFILE_SCOPE(name) Drift::Core::ScopedProfiler driftProfiler##__LINE__(name)
#define DRIFT_PROFILE_FUNCTION() DRIFT_PROFILE_SCOPE(__FUNCTION__)
#define DRIFT_PROFILE_SCOPE_WITH_PARENT(name, parent) Drift::Core::ScopedProfiler driftProfiler##__LINE__(name, parent)

// Macros condicionais
#define PROFILE_SCOPE_IF(condition, name) \
    std::unique_ptr<Drift::Core::ScopedProfiler> profiler##__LINE__; \
    if (condition) profiler##__LINE__ = std::make_unique<Drift::Core::ScopedProfiler>(name)

#define PROFILE_FUNCTION_IF(condition) PROFILE_SCOPE_IF(condition, __FUNCTION__)

// Macros para profiling de memória
#define PROFILE_MEMORY_ALLOC(size) Drift::Core::MemoryProfiler::GetInstance().TrackAllocation(size, __FUNCTION__)
#define PROFILE_MEMORY_DEALLOC(size) Drift::Core::MemoryProfiler::GetInstance().TrackDeallocation(size, __FUNCTION__)

// Macros para profiling de performance específica
#define PROFILE_PERF(name) PROFILE_SCOPE("[PERF]" name)
#define PROFILE_RENDER(name) PROFILE_SCOPE("[RENDER]" name)
#define PROFILE_UPDATE(name) PROFILE_SCOPE("[UPDATE]" name)
#define PROFILE_LOAD(name) PROFILE_SCOPE("[LOAD]" name)

} // namespace Drift::Core 