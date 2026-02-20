#pragma once

struct GLFWwindow;

namespace ComfyX {

class Application {
public:
    static Application& instance();

    bool initialize();
    void run();
    void shutdown();

    bool isRunning() const { return m_running; }
    GLFWwindow* getWindow() const { return m_window; }

private:
    Application() = default;

    bool initWindow();
    void mainLoop();
    void renderUI();

    GLFWwindow* m_window = nullptr;
    bool m_running = false;
    int m_windowWidth = 1600;
    int m_windowHeight = 900;
};

} // namespace ComfyX
