#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ComfyX {

class ComfyProcess {
public:
    static ComfyProcess& instance();

    enum class State { Stopped, Starting, Running, Error };

    using LogCallback = std::function<void(const std::string&)>;

    bool start(int port = 8188);
    void stop();
    bool isRunning() const { return m_state == State::Running; }
    State getState() const { return m_state; }
    std::string getError() const { return m_error; }

    bool isRuntimeInstalled() const;
    std::string getPythonPath() const;
    std::string getComfyUIPath() const;

    void onLog(LogCallback cb) { m_onLog = cb; }

    // Get recent log lines
    const std::vector<std::string>& getLog() const { return m_logBuffer; }

private:
    ComfyProcess() = default;
    ~ComfyProcess();

    void monitorProcess();
    void appendLog(const std::string& line);

    std::atomic<State> m_state{State::Stopped};
    std::string m_error;
    int m_port = 8188;

#ifdef _WIN32
    HANDLE m_processHandle = INVALID_HANDLE_VALUE;
    HANDLE m_pipeRead = INVALID_HANDLE_VALUE;
#else
    pid_t m_pid = -1;
    int m_pipeRead = -1;
#endif

    std::thread m_monitorThread;
    std::atomic<bool> m_shouldStop{false};

    LogCallback m_onLog;
    std::vector<std::string> m_logBuffer;
    static constexpr size_t MAX_LOG_LINES = 500;
};

} // namespace ComfyX
