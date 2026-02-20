#include "ComfyInstaller.h"
#include "app/PortablePaths.h"

#include <filesystem>
#include <iostream>
#include <fstream>

namespace ComfyX {

ComfyInstaller& ComfyInstaller::instance() {
    static ComfyInstaller inst;
    return inst;
}

bool ComfyInstaller::isInstalled() const {
    auto& paths = PortablePaths::instance();
    return std::filesystem::exists(paths.pythonDir() / "python.exe") &&
           std::filesystem::exists(paths.comfyuiDir() / "main.py");
}

void ComfyInstaller::startInstall(ProgressCallback onProgress) {
    if (m_state == State::Downloading || m_state == State::Installing) {
        return; // Already in progress
    }

    m_onProgress = onProgress;
    m_cancelled = false;
    m_error.clear();
    m_state = State::Downloading;

    if (m_installThread.joinable()) {
        m_installThread.join();
    }

    m_installThread = std::thread(&ComfyInstaller::installThread, this);
}

void ComfyInstaller::cancel() {
    m_cancelled = true;
}

void ComfyInstaller::installThread() {
    try {
        auto& paths = PortablePaths::instance();

        // Step 1: Create directories
        setProgress(5.0f, "Creating directories...");
        std::filesystem::create_directories(paths.pythonDir());
        std::filesystem::create_directories(paths.comfyuiDir());
        std::filesystem::create_directories(paths.modelsDir());

        if (m_cancelled) { m_state = State::Idle; return; }

        // Step 2: Download Python Embedded
        setProgress(10.0f, "Downloading Python Embedded...");
        // TODO: Actual download implementation using httplib
        // Download from: https://www.python.org/ftp/python/3.11.x/python-3.11.x-embed-amd64.zip
        // Extract to: runtime/python/

        if (m_cancelled) { m_state = State::Idle; return; }

        // Step 3: Setup pip in embedded Python
        setProgress(30.0f, "Setting up pip...");
        // TODO: Download get-pip.py and run it

        if (m_cancelled) { m_state = State::Idle; return; }

        // Step 4: Install ComfyUI
        setProgress(50.0f, "Installing ComfyUI...");
        // TODO: git clone or download ComfyUI
        // pip install -r requirements.txt

        if (m_cancelled) { m_state = State::Idle; return; }

        // Step 5: Install PyTorch
        setProgress(70.0f, "Installing PyTorch (this may take a while)...");
        // TODO: pip install torch torchvision

        if (m_cancelled) { m_state = State::Idle; return; }

        // Step 6: Verify installation
        setProgress(95.0f, "Verifying installation...");

        if (isInstalled()) {
            setProgress(100.0f, "Installation complete!");
            m_state = State::Complete;
        } else {
            m_error = "Installation verification failed";
            m_state = State::Error;
        }

    } catch (const std::exception& e) {
        m_error = e.what();
        m_state = State::Error;
        std::cerr << "[Installer] Error: " << e.what() << std::endl;
    }
}

bool ComfyInstaller::linkExternalComfyUI(const std::string& comfyuiPath) {
    if (!std::filesystem::exists(comfyuiPath + "/main.py")) {
        m_error = "Invalid ComfyUI directory (main.py not found)";
        return false;
    }

    auto& paths = PortablePaths::instance();

    // Create a symlink or config pointing to external ComfyUI
    try {
        nlohmann::json linkConfig;
        linkConfig["external_path"] = comfyuiPath;
        linkConfig["type"] = "external_link";

        std::ofstream file(paths.runtimeDir() / "comfyui_link.json");
        file << linkConfig.dump(2);

        std::cout << "[Installer] Linked to external ComfyUI: " << comfyuiPath << std::endl;
        return true;
    } catch (const std::exception& e) {
        m_error = e.what();
        return false;
    }
}

void ComfyInstaller::setProgress(float percent, const std::string& status) {
    m_progress = percent;
    m_status = status;
    if (m_onProgress) m_onProgress(percent, status);
    std::cout << "[Installer] " << (int)percent << "% - " << status << std::endl;
}

} // namespace ComfyX
