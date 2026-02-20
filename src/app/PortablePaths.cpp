#include "PortablePaths.h"
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace ComfyX {

PortablePaths& PortablePaths::instance() {
    static PortablePaths inst;
    return inst;
}

void PortablePaths::initialize() {
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    m_exeDir = std::filesystem::path(path).parent_path();
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    m_exeDir = std::filesystem::path(std::string(path, count > 0 ? count : 0)).parent_path();
#endif

    m_assetsDir = m_exeDir / "assets";
    m_dataDir = m_exeDir / "data";
    m_runtimeDir = m_exeDir / "runtime";

    ensureDirectories();
}

std::filesystem::path PortablePaths::configFile() const { return m_dataDir / "config.json"; }
std::filesystem::path PortablePaths::licenseFile() const { return m_dataDir / "license.dat"; }
std::filesystem::path PortablePaths::workflowsDir() const { return m_dataDir / "workflows"; }
std::filesystem::path PortablePaths::historyDir() const { return m_dataDir / "history"; }
std::filesystem::path PortablePaths::cacheDir() const { return m_dataDir / "cache"; }
std::filesystem::path PortablePaths::aiModelsDir() const { return m_dataDir / "ai_models"; }

std::filesystem::path PortablePaths::fontsDir() const { return m_assetsDir / "fonts"; }
std::filesystem::path PortablePaths::iconsDir() const { return m_assetsDir / "icons"; }
std::filesystem::path PortablePaths::themesDir() const { return m_assetsDir / "themes"; }
std::filesystem::path PortablePaths::promptsDir() const { return m_assetsDir / "prompts"; }

std::filesystem::path PortablePaths::pythonDir() const { return m_runtimeDir / "python"; }
std::filesystem::path PortablePaths::comfyuiDir() const { return m_runtimeDir / "comfyui"; }
std::filesystem::path PortablePaths::modelsDir() const { return m_runtimeDir / "models"; }
std::filesystem::path PortablePaths::outputDir() const { return m_runtimeDir / "output"; }

void PortablePaths::ensureDirectories() const {
    const std::filesystem::path dirs[] = {
        m_dataDir, workflowsDir(), historyDir(), cacheDir(), aiModelsDir(),
        m_runtimeDir, m_assetsDir, fontsDir(), iconsDir(), themesDir(), promptsDir()
    };
    for (const auto& dir : dirs) {
        std::filesystem::create_directories(dir);
    }
}

} // namespace ComfyX
