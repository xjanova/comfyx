#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <thread>

namespace ComfyX {

class ComfyInstaller {
public:
    static ComfyInstaller& instance();

    enum class State { Idle, Downloading, Installing, Complete, Error };

    using ProgressCallback = std::function<void(float percent, const std::string& status)>;

    bool isInstalled() const;
    void startInstall(ProgressCallback onProgress = nullptr);
    void cancel();

    State getState() const { return m_state; }
    float getProgress() const { return m_progress; }
    std::string getStatus() const { return m_status; }
    std::string getError() const { return m_error; }

    // Use existing ComfyUI installation
    bool linkExternalComfyUI(const std::string& comfyuiPath);

private:
    ComfyInstaller() = default;

    void installThread();

    std::atomic<State> m_state{State::Idle};
    std::atomic<float> m_progress{0.0f};
    std::string m_status;
    std::string m_error;
    std::atomic<bool> m_cancelled{false};
    std::thread m_installThread;

    ProgressCallback m_onProgress;

    void setProgress(float percent, const std::string& status);
};

} // namespace ComfyX
