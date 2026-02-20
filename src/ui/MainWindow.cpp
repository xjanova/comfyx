#include "MainWindow.h"
#include "app/Config.h"
#include "i18n/I18n.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace ComfyX {

MainWindow& MainWindow::instance() {
    static MainWindow inst;
    return inst;
}

void MainWindow::initialize() {
    // Initial setup done in first render frame
}

void MainWindow::render() {
    renderMenuBar();
    renderDockSpace();

    if (m_showSettings) renderSettingsPanel();
    if (m_showLicense)  renderLicenseDialog();
}

void MainWindow::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(I18n::t("menu.file").c_str())) {
            if (ImGui::MenuItem(I18n::t("menu.new_workflow").c_str(), "Ctrl+N")) {}
            if (ImGui::MenuItem(I18n::t("menu.open_workflow").c_str(), "Ctrl+O")) {}
            if (ImGui::MenuItem(I18n::t("menu.save_workflow").c_str(), "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem(I18n::t("menu.settings").c_str())) { m_showSettings = true; }
            ImGui::Separator();
            if (ImGui::MenuItem(I18n::t("menu.exit").c_str(), "Alt+F4")) {
                // Will be handled by GLFW close
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(I18n::t("menu.comfyui").c_str())) {
            if (ImGui::MenuItem(I18n::t("menu.connect").c_str())) {}
            if (ImGui::MenuItem(I18n::t("menu.disconnect").c_str())) {}
            ImGui::Separator();
            if (ImGui::MenuItem(I18n::t("menu.start_embedded").c_str())) {}
            if (ImGui::MenuItem(I18n::t("menu.stop_embedded").c_str())) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(I18n::t("menu.ai").c_str())) {
            auto& config = Config::instance().get();

            bool isOpenAI = config.ai.activeProvider == "openai";
            bool isClaude = config.ai.activeProvider == "claude";
            bool isGemini = config.ai.activeProvider == "gemini";
            bool isLocal  = config.ai.activeProvider == "local";

            if (ImGui::MenuItem("OpenAI (GPT-4o)", nullptr, isOpenAI))
                config.ai.activeProvider = "openai";
            if (ImGui::MenuItem("Claude (Sonnet)", nullptr, isClaude))
                config.ai.activeProvider = "claude";
            if (ImGui::MenuItem("Gemini (Flash)", nullptr, isGemini))
                config.ai.activeProvider = "gemini";
            if (ImGui::MenuItem("Local AI (Qwen)", nullptr, isLocal))
                config.ai.activeProvider = "local";

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(I18n::t("menu.help").c_str())) {
            if (ImGui::MenuItem(I18n::t("menu.license").c_str())) { m_showLicense = true; }
            if (ImGui::MenuItem(I18n::t("menu.about").c_str())) { m_showAbout = true; }
            ImGui::EndMenu();
        }

        // Right-aligned status
        float statusWidth = 300.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - statusWidth);

        // Language toggle
        auto& config = Config::instance().get();
        if (ImGui::SmallButton(config.language == "th" ? "EN" : "TH")) {
            config.language = (config.language == "th") ? "en" : "th";
            I18n::instance().setLanguage(config.language);
            Config::instance().save();
        }

        ImGui::SameLine();

        // ComfyUI status indicator
        ImVec4 statusColor = m_comfyConnected
            ? ImVec4(0.29f, 1.00f, 0.49f, 1.00f)  // Green
            : ImVec4(0.91f, 0.27f, 0.38f, 1.00f);  // Red
        ImGui::TextColored(statusColor, "ComfyUI: %s", m_comfyStatus.c_str());

        ImGui::EndMainMenuBar();
    }
}

void MainWindow::renderDockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Full-screen dock space
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpaceWindow", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceId = ImGui::GetID("ComfyXDockSpace");

    // Setup default layout on first frame
    if (m_firstFrame) {
        m_firstFrame = false;
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

        ImGuiID dockLeft, dockCenter, dockRight, dockBottom;

        // Split: left (25%) | center+right (75%)
        ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.25f, &dockLeft, &dockCenter);
        // Split center: center (60%) | right (40%)
        ImGuiID dockCenterTop;
        ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Right, 0.35f, &dockRight, &dockCenterTop);
        // Split center: top (70%) | bottom (30%)
        ImGui::DockBuilderSplitNode(dockCenterTop, ImGuiDir_Down, 0.30f, &dockBottom, &dockCenterTop);

        ImGui::DockBuilderDockWindow("AI Chat", dockLeft);
        ImGui::DockBuilderDockWindow("Node Graph", dockCenterTop);
        ImGui::DockBuilderDockWindow("Preview", dockRight);
        ImGui::DockBuilderDockWindow("History", dockBottom);

        ImGui::DockBuilderFinish(dockspaceId);
    }

    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    // Render panels
    renderChatPanel();
    renderNodeGraphPanel();
    renderPreviewPanel();
    renderHistoryPanel();
}

void MainWindow::renderChatPanel() {
    ImGui::Begin("AI Chat");

    ImGui::TextWrapped("%s", I18n::t("chat.welcome").c_str());
    ImGui::Separator();

    // Chat messages area
    ImGui::BeginChild("ChatMessages", ImVec2(0, -60), true);
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s",
                       I18n::t("chat.placeholder").c_str());
    ImGui::EndChild();

    // Input area
    static char inputBuf[2048] = "";
    ImGui::PushItemWidth(-80);
    bool enterPressed = ImGui::InputText("##ChatInput", inputBuf, sizeof(inputBuf),
                                         ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    if (ImGui::Button(I18n::t("chat.send").c_str(), ImVec2(-1, 0)) || enterPressed) {
        if (strlen(inputBuf) > 0) {
            // TODO: Send to AI engine
            inputBuf[0] = '\0';
        }
    }

    ImGui::End();
}

void MainWindow::renderNodeGraphPanel() {
    ImGui::Begin("Node Graph");

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s",
                       I18n::t("graph.empty").c_str());

    // TODO: imgui-node-editor integration

    ImGui::End();
}

void MainWindow::renderPreviewPanel() {
    ImGui::Begin("Preview");

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s",
                       I18n::t("preview.empty").c_str());

    // TODO: Image preview from ComfyUI

    ImGui::End();
}

void MainWindow::renderHistoryPanel() {
    ImGui::Begin("History");

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s",
                       I18n::t("history.empty").c_str());

    // TODO: Workflow history list

    ImGui::End();
}

void MainWindow::renderSettingsPanel() {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(I18n::t("settings.title").c_str(), &m_showSettings)) {
        auto& config = Config::instance().get();

        if (ImGui::BeginTabBar("SettingsTabs")) {
            // ComfyUI Settings
            if (ImGui::BeginTabItem(I18n::t("settings.comfyui").c_str())) {
                const char* modes[] = { "Embedded", "External" };
                int currentMode = config.comfyui.mode == "embedded" ? 0 : 1;
                if (ImGui::Combo(I18n::t("settings.mode").c_str(), &currentMode, modes, 2)) {
                    config.comfyui.mode = currentMode == 0 ? "embedded" : "external";
                }

                if (config.comfyui.mode == "external") {
                    char urlBuf[256];
                    strncpy(urlBuf, config.comfyui.externalUrl.c_str(), sizeof(urlBuf) - 1);
                    if (ImGui::InputText("URL", urlBuf, sizeof(urlBuf))) {
                        config.comfyui.externalUrl = urlBuf;
                    }
                } else {
                    ImGui::InputInt(I18n::t("settings.port").c_str(), &config.comfyui.embeddedPort);
                    ImGui::Checkbox(I18n::t("settings.autostart").c_str(), &config.comfyui.autoStart);
                }

                ImGui::EndTabItem();
            }

            // AI Settings
            if (ImGui::BeginTabItem(I18n::t("settings.ai").c_str())) {
                // OpenAI
                if (ImGui::CollapsingHeader("OpenAI")) {
                    char buf[256];
                    strncpy(buf, config.ai.openaiApiKey.c_str(), sizeof(buf) - 1);
                    if (ImGui::InputText("API Key##openai", buf, sizeof(buf), ImGuiInputTextFlags_Password)) {
                        config.ai.openaiApiKey = buf;
                    }
                }
                // Claude
                if (ImGui::CollapsingHeader("Anthropic Claude")) {
                    char buf[256];
                    strncpy(buf, config.ai.claudeApiKey.c_str(), sizeof(buf) - 1);
                    if (ImGui::InputText("API Key##claude", buf, sizeof(buf), ImGuiInputTextFlags_Password)) {
                        config.ai.claudeApiKey = buf;
                    }
                }
                // Gemini
                if (ImGui::CollapsingHeader("Google Gemini")) {
                    char buf[256];
                    strncpy(buf, config.ai.geminiApiKey.c_str(), sizeof(buf) - 1);
                    if (ImGui::InputText("API Key##gemini", buf, sizeof(buf), ImGuiInputTextFlags_Password)) {
                        config.ai.geminiApiKey = buf;
                    }
                }
                // Local AI
                if (ImGui::CollapsingHeader("Local AI (llama.cpp)")) {
                    char buf[512];
                    strncpy(buf, config.ai.localModelPath.c_str(), sizeof(buf) - 1);
                    if (ImGui::InputText("Model Path", buf, sizeof(buf))) {
                        config.ai.localModelPath = buf;
                    }
                    ImGui::SliderInt("GPU Layers", &config.ai.localGpuLayers, 0, 100);
                    ImGui::InputInt("Context Size", &config.ai.localContextSize);
                }
                ImGui::EndTabItem();
            }

            // Appearance
            if (ImGui::BeginTabItem(I18n::t("settings.appearance").c_str())) {
                const char* themes[] = { "Midnight Studio", "Light" };
                int currentTheme = config.theme == "light" ? 1 : 0;
                if (ImGui::Combo(I18n::t("settings.theme").c_str(), &currentTheme, themes, 2)) {
                    config.theme = currentTheme == 1 ? "light" : "midnight";
                }

                ImGui::SliderFloat(I18n::t("settings.scale").c_str(), &config.uiScale, 0.8f, 2.0f, "%.1f");

                const char* languages[] = { "English", "Thai" };
                int currentLang = config.language == "th" ? 1 : 0;
                if (ImGui::Combo(I18n::t("settings.language").c_str(), &currentLang, languages, 2)) {
                    config.language = currentLang == 1 ? "th" : "en";
                    I18n::instance().setLanguage(config.language);
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        if (ImGui::Button(I18n::t("settings.save").c_str(), ImVec2(120, 0))) {
            Config::instance().save();
            m_showSettings = false;
        }
        ImGui::SameLine();
        if (ImGui::Button(I18n::t("settings.cancel").c_str(), ImVec2(120, 0))) {
            m_showSettings = false;
        }
    }
    ImGui::End();
}

void MainWindow::renderLicenseDialog() {
    ImGui::SetNextWindowSize(ImVec2(450, 350), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(I18n::t("license.title").c_str(), &m_showLicense)) {
        ImGui::TextWrapped("%s", I18n::t("license.info").c_str());
        ImGui::Separator();

        static char licenseKey[64] = "";
        ImGui::InputText(I18n::t("license.key").c_str(), licenseKey, sizeof(licenseKey));

        ImGui::Spacing();
        if (ImGui::Button(I18n::t("license.activate").c_str(), ImVec2(140, 0))) {
            // TODO: Activate via xmanstudio API
        }
        ImGui::SameLine();
        if (ImGui::Button(I18n::t("license.trial").c_str(), ImVec2(140, 0))) {
            // TODO: Start trial via xmanstudio API
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Status: Trial (7 days remaining)");
    }
    ImGui::End();
}

void MainWindow::shutdown() {
    // Cleanup
}

} // namespace ComfyX
