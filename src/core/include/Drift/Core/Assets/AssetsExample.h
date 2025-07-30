#pragma once

#include "AssetsSystem.h"
#include <memory>
#include <string>

namespace Drift::Core::Assets {

/**
 * @brief Exemplo de asset simples
 */
class SimpleAsset : public IAsset {
public:
    SimpleAsset(const std::string& path, const std::string& name);
    
    // IAsset interface
    const std::string& GetPath() const override { return m_Path; }
    const std::string& GetName() const override { return m_Name; }
    size_t GetMemoryUsage() const override { return m_MemoryUsage; }
    AssetStatus GetStatus() const override { return m_Status; }
    
    bool Load() override;
    void Unload() override;
    bool IsLoaded() const override { return m_Status == AssetStatus::Loaded; }
    
    std::chrono::steady_clock::time_point GetLoadTime() const override { return m_LoadTime; }
    size_t GetAccessCount() const override { return m_AccessCount; }
    void UpdateAccess() override { m_AccessCount++; }

private:
    std::string m_Path;
    std::string m_Name;
    size_t m_MemoryUsage = 0;
    AssetStatus m_Status = AssetStatus::NotLoaded;
    std::chrono::steady_clock::time_point m_LoadTime;
    size_t m_AccessCount = 0;
};

/**
 * @brief Loader para SimpleAsset
 */
class SimpleAssetLoader : public IAssetLoader<SimpleAsset> {
public:
    std::shared_ptr<SimpleAsset> Load(const std::string& path, const std::any& params = {}) override;
    bool CanLoad(const std::string& path) const override;
    std::vector<std::string> GetSupportedExtensions() const override;
    std::string GetLoaderName() const override { return "SimpleAssetLoader"; }
    size_t EstimateMemoryUsage(const std::string& path) const override;
};

/**
 * @brief Exemplo de uso do sistema de Assets
 */
class AssetsExample {
public:
    static void RunBasicExample();
    static void RunAsyncLoadingExample();
    static void RunPreloadingExample();
    static void RunCacheManagementExample();
    static void RunPerformanceTest();
    static void RunCompleteExample();

private:
    static std::vector<std::string> GenerateAssetPaths(size_t count);
    static void OnAssetLoaded(const std::string& path, std::type_index type);
    static void OnAssetUnloaded(const std::string& path, std::type_index type);
    static void OnAssetFailed(const std::string& path, std::type_index type, const std::string& error);
};

} // namespace Drift::Core::Assets 