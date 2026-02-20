#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <imgui.h>

namespace ComfyX {

struct LogEntry {
    enum Level { Info, Warn, Error, Debug };
    Level level = Info;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};

class MainWindow {
public:
    static MainWindow& instance();

    void initialize();
    void render();
    void shutdown();

    // Application-level logging
    void addLog(LogEntry::Level level, const std::string& message);

    enum class Page {
        Chat,
        NodeGraph,
        Preview,
        Log,
        Settings
    };

private:
    MainWindow() = default;

    // Layout components
    void renderTopBar();
    void renderSidebar();
    void renderContentArea();
    void renderStatusBar();

    // Top bar widgets
    void renderComfyToggleButton();

    // Pages
    void renderChatPage();
    void renderNodeGraphPage();
    void renderPreviewPage();
    void renderLogPage();
    void renderSettingsPage();

    // Dialogs
    void renderLicenseDialog();

    // Drawing helpers (neon effects)
    void drawNeonRect(ImDrawList* dl, ImVec2 min, ImVec2 max, ImU32 color,
                      float rounding, float glowSize = 0.0f, ImU32 glowColor = 0);

    // State
    Page m_activePage = Page::Chat;
    bool m_showLicense = false;

    // Layout dimensions
    float m_sidebarWidth  = 60.0f;
    float m_topBarHeight  = 52.0f;
    float m_statusBarHeight = 30.0f;

    // Log
    mutable std::mutex m_logMutex;
    std::vector<LogEntry> m_appLog;
    bool m_logAutoScroll = true;
    static constexpr size_t MAX_APP_LOG = 1000;

    // ComfyUI status
    std::string m_comfyStatus = "Disconnected";
    bool m_comfyConnected = false;
};

} // namespace ComfyX
