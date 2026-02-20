#pragma once
#include <string>

namespace ComfyX {

class MainWindow {
public:
    static MainWindow& instance();

    void initialize();
    void render();
    void shutdown();

private:
    MainWindow() = default;

    void renderMenuBar();
    void renderToolbar();
    void renderStatusBar();
    void renderDockSpace();

    // Panels
    void renderChatPanel();
    void renderNodeGraphPanel();
    void renderPreviewPanel();
    void renderHistoryPanel();
    void renderSettingsPanel();
    void renderLicenseDialog();

    bool m_showSettings = false;
    bool m_showLicense = false;
    bool m_showAbout = false;
    bool m_firstFrame = true;

    // ComfyUI connection status
    std::string m_comfyStatus = "Disconnected";
    bool m_comfyConnected = false;
};

} // namespace ComfyX
