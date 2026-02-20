#pragma once
#include "Theme.h"

struct GLFWwindow;

namespace ComfyX {

class UIManager {
public:
    static UIManager& instance();

    bool initialize(GLFWwindow* window);
    void shutdown();

    void beginFrame();
    void endFrame();

    void setTheme(Theme::Style style);
    void setLanguage(const std::string& lang);
    void setScale(float scale);

    float getScale() const { return m_scale; }
    bool isInitialized() const { return m_initialized; }

private:
    UIManager() = default;

    bool loadFonts();

    bool m_initialized = false;
    float m_scale = 1.0f;
    Theme::Style m_currentTheme = Theme::Style::Midnight;
};

} // namespace ComfyX
