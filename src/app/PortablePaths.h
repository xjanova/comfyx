#pragma once
#include <string>
#include <filesystem>

namespace ComfyX {

class PortablePaths {
public:
    static PortablePaths& instance();

    void initialize();

    // Base directories
    const std::filesystem::path& exeDir() const { return m_exeDir; }
    const std::filesystem::path& assetsDir() const { return m_assetsDir; }
    const std::filesystem::path& dataDir() const { return m_dataDir; }
    const std::filesystem::path& runtimeDir() const { return m_runtimeDir; }

    // Specific paths
    std::filesystem::path configFile() const;
    std::filesystem::path licenseFile() const;
    std::filesystem::path workflowsDir() const;
    std::filesystem::path historyDir() const;
    std::filesystem::path cacheDir() const;
    std::filesystem::path aiModelsDir() const;

    // Assets
    std::filesystem::path fontsDir() const;
    std::filesystem::path iconsDir() const;
    std::filesystem::path themesDir() const;
    std::filesystem::path promptsDir() const;

    // Runtime (embedded ComfyUI)
    std::filesystem::path pythonDir() const;
    std::filesystem::path comfyuiDir() const;
    std::filesystem::path modelsDir() const;
    std::filesystem::path outputDir() const;

    // Ensure directories exist
    void ensureDirectories() const;

private:
    PortablePaths() = default;

    std::filesystem::path m_exeDir;
    std::filesystem::path m_assetsDir;
    std::filesystem::path m_dataDir;
    std::filesystem::path m_runtimeDir;
};

} // namespace ComfyX
