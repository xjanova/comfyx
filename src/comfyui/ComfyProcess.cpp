#include "ComfyProcess.h"
#include "app/PortablePaths.h"

#include <iostream>
#include <filesystem>

namespace ComfyX {

ComfyProcess& ComfyProcess::instance() {
    static ComfyProcess inst;
    return inst;
}

ComfyProcess::~ComfyProcess() {
    stop();
}

bool ComfyProcess::isRuntimeInstalled() const {
    auto& paths = PortablePaths::instance();
#ifdef _WIN32
    const char* pythonExe = "python.exe";
#else
    const char* pythonExe = "python3";
#endif
    return std::filesystem::exists(paths.pythonDir() / pythonExe) &&
           std::filesystem::exists(paths.comfyuiDir() / "main.py");
}

std::string ComfyProcess::getPythonPath() const {
#ifdef _WIN32
    return (PortablePaths::instance().pythonDir() / "python.exe").string();
#else
    return (PortablePaths::instance().pythonDir() / "python3").string();
#endif
}

std::string ComfyProcess::getComfyUIPath() const {
    return (PortablePaths::instance().comfyuiDir() / "main.py").string();
}

bool ComfyProcess::start(int port) {
    if (m_state == State::Running) {
        appendLog("[ComfyProcess] Already running");
        return true;
    }

    if (!isRuntimeInstalled()) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_error = "ComfyUI runtime not installed. Run the installer first.";
        }
        m_state = State::Error;
        appendLog("[ComfyProcess] ComfyUI runtime not installed.");
        return false;
    }

    m_port = port;
    m_state = State::Starting;
    m_shouldStop = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_error.clear();
    }

    appendLog("[ComfyProcess] Starting ComfyUI on port " + std::to_string(port) + "...");

#ifdef _WIN32
    // Create pipe for stdout/stderr capture
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE pipeWrite;
    if (!CreatePipe(&m_pipeRead, &pipeWrite, &sa, 0)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_error = "Failed to create pipe";
        m_state = State::Error;
        return false;
    }
    SetHandleInformation(m_pipeRead, HANDLE_FLAG_INHERIT, 0);

    // Build command line
    std::string cmd = "\"" + getPythonPath() + "\" \"" + getComfyUIPath() + "\""
                      + " --port " + std::to_string(port)
                      + " --dont-print-server";

    // Set working directory to ComfyUI directory
    std::string workDir = PortablePaths::instance().comfyuiDir().string();

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.hStdOutput = pipeWrite;
    si.hStdError = pipeWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};

    BOOL success = CreateProcessA(
        nullptr,
        const_cast<char*>(cmd.c_str()),
        nullptr, nullptr,
        TRUE, // Inherit handles
        CREATE_NO_WINDOW,
        nullptr,
        workDir.c_str(),
        &si, &pi
    );

    CloseHandle(pipeWrite);

    if (!success) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_error = "Failed to start ComfyUI process (error " + std::to_string(GetLastError()) + ")";
        }
        m_state = State::Error;
        CloseHandle(m_pipeRead);
        m_pipeRead = INVALID_HANDLE_VALUE;
        appendLog("[ComfyProcess] " + m_error);
        return false;
    }

    m_processHandle = pi.hProcess;
    CloseHandle(pi.hThread);

    // Start monitor thread
    m_monitorThread = std::thread(&ComfyProcess::monitorProcess, this);

    appendLog("[ComfyProcess] Process started (PID: " + std::to_string(pi.dwProcessId) + ")");
    m_state = State::Running;
    return true;
#else
    // Linux/Mac implementation would use fork/exec
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_error = "Embedded mode not yet supported on this platform";
    }
    m_state = State::Error;
    return false;
#endif
}

void ComfyProcess::stop() {
    m_shouldStop = true;

#ifdef _WIN32
    if (m_processHandle != INVALID_HANDLE_VALUE) {
        appendLog("[ComfyProcess] Stopping ComfyUI...");
        TerminateProcess(m_processHandle, 0);
        WaitForSingleObject(m_processHandle, 5000);
        CloseHandle(m_processHandle);
        m_processHandle = INVALID_HANDLE_VALUE;
    }

    if (m_pipeRead != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipeRead);
        m_pipeRead = INVALID_HANDLE_VALUE;
    }
#endif

    if (m_monitorThread.joinable()) {
        m_monitorThread.join();
    }

    m_state = State::Stopped;
    appendLog("[ComfyProcess] Stopped");
}

void ComfyProcess::monitorProcess() {
#ifdef _WIN32
    char buffer[4096];
    DWORD bytesRead;

    while (!m_shouldStop) {
        // Read process output
        if (m_pipeRead != INVALID_HANDLE_VALUE) {
            DWORD available;
            if (PeekNamedPipe(m_pipeRead, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
                if (ReadFile(m_pipeRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    appendLog(std::string(buffer, bytesRead));
                }
            }
        }

        // Check if process is still running
        if (m_processHandle != INVALID_HANDLE_VALUE) {
            DWORD exitCode;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode != STILL_ACTIVE) {
                if (!m_shouldStop) {
                    std::string errMsg = "ComfyUI process exited with code " + std::to_string(exitCode);
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_error = errMsg;
                    }
                    m_state = State::Error;
                    appendLog("[ComfyProcess] " + errMsg);
                }
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#endif
}

void ComfyProcess::appendLog(const std::string& line) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logBuffer.push_back(line);
        if (m_logBuffer.size() > MAX_LOG_LINES) {
            m_logBuffer.erase(m_logBuffer.begin());
        }
    }
    if (m_onLog) m_onLog(line);
    std::cout << line << std::endl;
}

} // namespace ComfyX
