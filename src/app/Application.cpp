#include "Application.h"
#include "PortablePaths.h"
#include "Config.h"
#include "ui/UIManager.h"
#include "ui/MainWindow.h"
#include "i18n/I18n.h"
#include "license/LicenseClient.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace ComfyX {

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "[GLFW Error " << error << "] " << description << std::endl;
}

Application& Application::instance() {
    static Application inst;
    return inst;
}

bool Application::initialize() {
    std::cout << "[App] ComfyX v1.0.0 - Initializing..." << std::endl;

    // 1. Initialize portable paths
    PortablePaths::instance().initialize();
    std::cout << "[App] Exe dir: " << PortablePaths::instance().exeDir() << std::endl;

    // 2. Load configuration
    Config::instance().load();
    std::cout << "[App] Config loaded" << std::endl;

    // 2.5. Initialize i18n
    I18n::instance().initialize(Config::instance().get().language);
    std::cout << "[App] Language: " << Config::instance().get().language << std::endl;

    // 2.6. Load cached license
    LicenseClient::instance().loadCachedLicense();

    // 3. Create window
    if (!initWindow()) {
        std::cerr << "[App] Failed to create window" << std::endl;
        return false;
    }

    // 4. Initialize UI
    if (!UIManager::instance().initialize(m_window)) {
        std::cerr << "[App] Failed to initialize UI" << std::endl;
        return false;
    }

    // 5. Initialize main window layout
    MainWindow::instance().initialize();

    m_running = true;
    std::cout << "[App] Initialization complete" << std::endl;
    return true;
}

bool Application::initWindow() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        std::cerr << "[App] Failed to initialize GLFW" << std::endl;
        return false;
    }

    // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight,
                                "ComfyX - AI Workflow Studio", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[App] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // VSync

    std::cout << "[App] Window created (" << m_windowWidth << "x" << m_windowHeight << ")" << std::endl;
    return true;
}

void Application::run() {
    mainLoop();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(m_window) && m_running) {
        glfwPollEvents();

        UIManager::instance().beginFrame();
        renderUI();
        UIManager::instance().endFrame();

        glfwSwapBuffers(m_window);
    }
}

void Application::renderUI() {
    MainWindow::instance().render();
}

void Application::shutdown() {
    std::cout << "[App] Shutting down..." << std::endl;

    Config::instance().save();
    MainWindow::instance().shutdown();
    UIManager::instance().shutdown();

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();

    m_running = false;
    std::cout << "[App] Shutdown complete" << std::endl;
}

} // namespace ComfyX
