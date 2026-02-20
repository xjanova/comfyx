#include "MainWindow.h"
#include "Theme.h"
#include "app/Config.h"
#include "i18n/I18n.h"
#include "comfyui/ComfyProcess.h"
#include "ai/AIManager.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <ctime>

namespace ComfyX {

// ═══════════════════════════════════════════════════════════════════════════════
// HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

static ImU32 toU32(const ImVec4& c) {
    return ImGui::ColorConvertFloat4ToU32(c);
}

static ImVec4 withAlpha(const ImVec4& c, float a) {
    return ImVec4(c.x, c.y, c.z, a);
}

// ═══════════════════════════════════════════════════════════════════════════════
// CORE
// ═══════════════════════════════════════════════════════════════════════════════

MainWindow& MainWindow::instance() {
    static MainWindow inst;
    return inst;
}

void MainWindow::initialize() {
    addLog(LogEntry::Info, "ComfyX initialized");
}

void MainWindow::addLog(LogEntry::Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.timestamp = std::chrono::system_clock::now();
    m_appLog.push_back(std::move(entry));
    if (m_appLog.size() > MAX_APP_LOG) {
        m_appLog.erase(m_appLog.begin());
    }
}

void MainWindow::drawNeonRect(ImDrawList* dl, ImVec2 min, ImVec2 max,
                               ImU32 color, float rounding,
                               float glowSize, ImU32 glowColor) {
    dl->AddRectFilled(min, max, color, rounding);
    if (glowSize > 0.0f && glowColor != 0) {
        // Soft outer glow
        dl->AddRect(
            ImVec2(min.x - 1, min.y - 1),
            ImVec2(max.x + 1, max.y + 1),
            glowColor, rounding + 1, 0, glowSize);
    }
}

void MainWindow::render() {
    renderTopBar();
    renderSidebar();
    renderContentArea();
    renderStatusBar();

    if (m_showLicense) renderLicenseDialog();
}

// ═══════════════════════════════════════════════════════════════════════════════
// TOP BAR — Neon-styled header with gradient brand icon
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderTopBar() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const auto& p = Theme::getPalette();

    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, m_topBarHeight));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, withAlpha(p.bgPrimary, 0.95f));

    ImGui::Begin("##TopBar", nullptr, flags);

    float barH = m_topBarHeight;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // ─── Brand Icon (gradient square like X-Repass) ───
    float iconSize = 26.0f;
    float iconY = (barH - iconSize) * 0.5f;
    ImVec2 iconPos = ImGui::GetCursorScreenPos();
    iconPos.y = vp->WorkPos.y + iconY;

    // Gradient icon background (cyan → pink)
    ImVec2 iconMin(iconPos.x, iconPos.y);
    ImVec2 iconMax(iconPos.x + iconSize, iconPos.y + iconSize);
    dl->AddRectFilledMultiColor(iconMin, iconMax,
        toU32(p.accent),        // top-left: cyan
        toU32(p.accentPink),    // top-right: pink
        toU32(p.accentPurple),  // bottom-right: purple
        toU32(p.accent));       // bottom-left: cyan

    // Round corners overlay (draw filled rounded rect with gradient behind, then letter)
    // For simplicity in ImGui, just draw the rect with rounding
    dl->AddRect(iconMin, iconMax, toU32(withAlpha(p.accent, 0.5f)), 7.0f, 0, 1.5f);

    // Letter "C" on the icon
    ImVec2 letterPos(iconPos.x + 7, iconPos.y + 3);
    dl->AddText(ImGui::GetFont(), 18.0f, letterPos,
                IM_COL32(255, 255, 255, 240), "C");

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconSize + 10);

    // ─── App Name ───
    float textY = (barH - ImGui::GetTextLineHeight()) * 0.5f;
    ImGui::SetCursorPosY(textY);
    ImGui::PushStyleColor(ImGuiCol_Text, p.textPrimary);
    ImGui::Text("ComfyX");
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 4);
    ImGui::TextColored(p.textMuted, "- AI Workflow Studio");
    ImGui::SameLine(0, 28);

    // ─── File buttons (transparent, neon hover) ───
    ImGui::SetCursorPosY((barH - ImGui::GetFrameHeight()) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.accent, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, withAlpha(p.accent, 0.25f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));

    if (ImGui::Button(I18n::t("menu.new_workflow").c_str())) {}
    ImGui::SameLine(0, 4);
    if (ImGui::Button(I18n::t("menu.open_workflow").c_str())) {}
    ImGui::SameLine(0, 4);
    if (ImGui::Button(I18n::t("menu.save_workflow").c_str())) {}

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, 36);

    // ─── ComfyUI Start/Stop (prominent, center-ish) ───
    ImGui::SetCursorPosY((barH - ImGui::GetFrameHeight()) * 0.5f);
    renderComfyToggleButton();

    // ─── Right section ───
    float rightWidth = 260.0f;
    float rightStart = vp->WorkSize.x - rightWidth - 16;
    ImGui::SameLine(rightStart);
    ImGui::SetCursorPosY((barH - ImGui::GetFrameHeight()) * 0.5f);

    // Connection badge (styled like ZipCrackerUI status badges)
    ImVec4 connColor = m_comfyConnected ? p.success : p.textMuted;
    ImVec2 badgePos = ImGui::GetCursorScreenPos();
    float badgeY = vp->WorkPos.y + (barH - 22) * 0.5f;

    // Badge background
    ImVec2 badgeMin(badgePos.x, badgeY);
    ImVec2 badgeMax(badgePos.x + 90, badgeY + 22);
    dl->AddRectFilled(badgeMin, badgeMax, toU32(withAlpha(connColor, 0.10f)), 6.0f);
    dl->AddRect(badgeMin, badgeMax, toU32(withAlpha(connColor, 0.35f)), 6.0f);

    // LED dot inside badge
    ImVec2 dotCenter(badgeMin.x + 12, badgeY + 11);
    dl->AddCircleFilled(dotCenter, 3.5f, toU32(connColor));

    // Badge text
    const char* connStr = m_comfyConnected ? "Online" : "Offline";
    dl->AddText(ImVec2(badgeMin.x + 22, badgeY + 3),
                toU32(connColor), connStr);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 100);
    ImGui::SameLine(0, 16);

    // Language toggle (neon pill)
    auto& config = Config::instance().get();
    ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(p.accentPurple, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.accentPurple, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_Text, p.accentPurple);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 4));

    if (ImGui::Button(config.language == "th" ? "EN" : "TH")) {
        config.language = (config.language == "th") ? "en" : "th";
        I18n::instance().setLanguage(config.language);
        Config::instance().save();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, 8);

    // License button (small, subtle)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.accent, 0.12f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button("Lic")) {
        m_showLicense = true;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    // Bottom gradient line (neon cyan → pink)
    ImVec2 lineMin(vp->WorkPos.x, vp->WorkPos.y + barH - 1);
    ImVec2 lineMid(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + barH);
    ImVec2 lineMax(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + barH);
    dl->AddRectFilledMultiColor(
        lineMin, lineMid,
        toU32(withAlpha(p.accent, 0.6f)),
        toU32(withAlpha(p.accentPink, 0.6f)),
        toU32(withAlpha(p.accentPink, 0.6f)),
        toU32(withAlpha(p.accent, 0.6f)));
    dl->AddRectFilledMultiColor(
        ImVec2(lineMid.x, lineMin.y), lineMax,
        toU32(withAlpha(p.accentPink, 0.6f)),
        toU32(withAlpha(p.accentPurple, 0.6f)),
        toU32(withAlpha(p.accentPurple, 0.6f)),
        toU32(withAlpha(p.accentPink, 0.6f)));

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// ═══════════════════════════════════════════════════════════════════════════════
// COMFYUI START/STOP — Neon glow toggle
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderComfyToggleButton() {
    auto& process = ComfyProcess::instance();
    auto state = process.getState();
    const auto& p = Theme::getPalette();

    ImVec4 btnBg, btnText, ledColor;
    const char* label;
    bool canClick = true;

    switch (state) {
        case ComfyProcess::State::Stopped:
            btnBg   = ImVec4(0.0f, 0.4f, 0.3f, 0.25f);  // dark green tint
            btnText = p.success;
            ledColor = p.textMuted;
            label = I18n::t("topbar.start").c_str();
            break;
        case ComfyProcess::State::Starting:
            btnBg   = ImVec4(0.4f, 0.2f, 0.0f, 0.25f);  // dark orange tint
            btnText = p.warning;
            ledColor = p.warning;
            label = I18n::t("topbar.starting").c_str();
            canClick = false;
            break;
        case ComfyProcess::State::Running:
            btnBg   = ImVec4(0.4f, 0.0f, 0.1f, 0.20f);  // dark red tint
            btnText = p.error;
            ledColor = p.success;
            label = I18n::t("topbar.stop").c_str();
            break;
        case ComfyProcess::State::Error:
            btnBg   = ImVec4(0.4f, 0.0f, 0.1f, 0.25f);
            btnText = p.error;
            ledColor = p.error;
            label = I18n::t("topbar.retry").c_str();
            break;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // LED circle with glow
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float ledRadius = 5.0f;
    float btnH = ImGui::GetFrameHeight();
    ImVec2 ledCenter(cursor.x + ledRadius + 2, cursor.y + btnH * 0.5f);

    // Outer glow for LED
    if (state == ComfyProcess::State::Running || state == ComfyProcess::State::Starting) {
        float glowAlpha = (state == ComfyProcess::State::Starting)
            ? (sinf(static_cast<float>(ImGui::GetTime()) * 5.0f) + 1.0f) * 0.25f
            : 0.3f;
        ImVec4 glow = ledColor;
        glow.w = glowAlpha;
        dl->AddCircleFilled(ledCenter, ledRadius + 6.0f, toU32(glow));
        dl->AddCircleFilled(ledCenter, ledRadius + 3.0f, toU32(withAlpha(ledColor, glowAlpha + 0.1f)));
    }

    dl->AddCircleFilled(ledCenter, ledRadius, toU32(ledColor));

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ledRadius * 2 + 12);

    // Button with neon border on hover
    ImGui::PushStyleColor(ImGuiCol_Button, btnBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImVec4(btnBg.x * 1.3f, btnBg.y * 1.3f, btnBg.z * 1.3f, btnBg.w + 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        ImVec4(btnBg.x * 1.5f, btnBg.y * 1.5f, btnBg.z * 1.5f, btnBg.w + 0.30f));
    ImGui::PushStyleColor(ImGuiCol_Text, btnText);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 6));

    if (ImGui::Button(label) && canClick) {
        if (state == ComfyProcess::State::Running) {
            process.stop();
            addLog(LogEntry::Info, "ComfyUI stop requested");
        } else {
            auto& cfg = Config::instance().get();
            process.start(cfg.comfyui.embeddedPort);
            addLog(LogEntry::Info, "ComfyUI start requested on port " +
                   std::to_string(cfg.comfyui.embeddedPort));
        }
    }

    // Neon border glow on hover
    if (ImGui::IsItemHovered()) {
        ImVec2 bMin = ImGui::GetItemRectMin();
        ImVec2 bMax = ImGui::GetItemRectMax();
        dl->AddRect(bMin, bMax, toU32(withAlpha(btnText, 0.5f)), 8.0f, 0, 1.5f);
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

    if (state == ComfyProcess::State::Error && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Error: %s", process.getError().c_str());
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SIDEBAR — Neon icon navigation
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderSidebar() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const auto& p = Theme::getPalette();

    ImVec2 pos(vp->WorkPos.x, vp->WorkPos.y + m_topBarHeight);
    ImVec2 size(m_sidebarWidth, vp->WorkSize.y - m_topBarHeight - m_statusBarHeight);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 14));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, withAlpha(p.bgPrimary, 0.95f));

    ImGui::Begin("##Sidebar", nullptr, flags);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    struct NavItem {
        const char* label;
        const char* tooltip;
        Page page;
        ImVec4 activeColor;  // Unique neon color per icon
    };

    NavItem items[] = {
        { "Ai",  "AI Chat",     Page::Chat,      p.accent },       // Cyan
        { "Nd",  "Node Graph",  Page::NodeGraph,  p.success },      // Green
        { "Pv",  "Preview",     Page::Preview,    p.accentPink },   // Pink
        { "Lg",  "Log",         Page::Log,        p.warning },      // Orange
    };

    float btnSize = 40.0f;
    float centerX = (m_sidebarWidth - btnSize) * 0.5f;

    for (auto& item : items) {
        ImGui::SetCursorPosX(centerX);

        bool isActive = (m_activePage == item.page);
        ImVec4 itemColor = isActive ? item.activeColor : p.textMuted;

        // Active indicator bar (left edge, neon colored)
        if (isActive) {
            ImVec2 barMin = ImGui::GetCursorScreenPos();
            barMin.x = pos.x;
            ImVec2 barMax(barMin.x + 3.0f, barMin.y + btnSize);
            dl->AddRectFilled(barMin, barMax, toU32(item.activeColor), 2.0f);

            // Subtle glow behind active button
            ImVec2 glowMin(pos.x + 4, barMin.y - 2);
            ImVec2 glowMax(pos.x + m_sidebarWidth, barMin.y + btnSize + 2);
            dl->AddRectFilled(glowMin, glowMax,
                toU32(withAlpha(item.activeColor, 0.06f)), 8.0f);
        }

        // Button style
        ImVec4 bg = isActive ? withAlpha(item.activeColor, 0.15f) : ImVec4(0, 0, 0, 0);
        ImGui::PushStyleColor(ImGuiCol_Button, bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(item.activeColor, 0.12f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, withAlpha(item.activeColor, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_Text, itemColor);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

        if (ImGui::Button(item.label, ImVec2(btnSize, btnSize))) {
            m_activePage = item.page;
        }

        // Neon hover glow
        if (ImGui::IsItemHovered() && !isActive) {
            ImVec2 hMin = ImGui::GetItemRectMin();
            ImVec2 hMax = ImGui::GetItemRectMax();
            dl->AddRect(hMin, hMax, toU32(withAlpha(item.activeColor, 0.3f)), 10.0f, 0, 1.0f);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", item.tooltip);
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        ImGui::Spacing();
    }

    // ─── Settings button at bottom ───
    float bottomY = size.y - btnSize - 16;
    if (ImGui::GetCursorPosY() < bottomY) {
        ImGui::SetCursorPosY(bottomY);
    }

    ImGui::SetCursorPosX(centerX);
    bool isSett = (m_activePage == Page::Settings);
    ImVec4 settColor = p.accentPurple;

    if (isSett) {
        ImVec2 barMin = ImGui::GetCursorScreenPos();
        barMin.x = pos.x;
        ImVec2 barMax(barMin.x + 3.0f, barMin.y + btnSize);
        dl->AddRectFilled(barMin, barMax, toU32(settColor), 2.0f);

        ImVec2 glowMin(pos.x + 4, barMin.y - 2);
        ImVec2 glowMax(pos.x + m_sidebarWidth, barMin.y + btnSize + 2);
        dl->AddRectFilled(glowMin, glowMax, toU32(withAlpha(settColor, 0.06f)), 8.0f);
    }

    ImVec4 settBg = isSett ? withAlpha(settColor, 0.15f) : ImVec4(0, 0, 0, 0);
    ImGui::PushStyleColor(ImGuiCol_Button, settBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(settColor, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_Text, isSett ? settColor : p.textMuted);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

    if (ImGui::Button("Set", ImVec2(btnSize, btnSize))) {
        m_activePage = Page::Settings;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Settings");

    if (ImGui::IsItemHovered() && !isSett) {
        ImVec2 hMin = ImGui::GetItemRectMin();
        ImVec2 hMax = ImGui::GetItemRectMax();
        dl->AddRect(hMin, hMax, toU32(withAlpha(settColor, 0.3f)), 10.0f, 0, 1.0f);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // Right border (subtle)
    ImVec2 bMin(pos.x + m_sidebarWidth - 1, pos.y);
    ImVec2 bMax(pos.x + m_sidebarWidth, pos.y + size.y);
    dl->AddRectFilled(bMin, bMax, toU32(withAlpha(p.border, 0.25f)));

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONTENT AREA
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderContentArea() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const auto& p = Theme::getPalette();

    ImVec2 pos(vp->WorkPos.x + m_sidebarWidth, vp->WorkPos.y + m_topBarHeight);
    ImVec2 size(vp->WorkSize.x - m_sidebarWidth,
                vp->WorkSize.y - m_topBarHeight - m_statusBarHeight);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, p.bgDeep);

    ImGui::Begin("##ContentArea", nullptr, flags);

    switch (m_activePage) {
        case Page::Chat:      renderChatPage(); break;
        case Page::NodeGraph: renderNodeGraphPage(); break;
        case Page::Preview:   renderPreviewPage(); break;
        case Page::Log:       renderLogPage(); break;
        case Page::Settings:  renderSettingsPage(); break;
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// ═══════════════════════════════════════════════════════════════════════════════
// STATUS BAR — Neon badges, LED, version info
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderStatusBar() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const auto& p = Theme::getPalette();

    ImVec2 pos(vp->WorkPos.x,
               vp->WorkPos.y + vp->WorkSize.y - m_statusBarHeight);
    ImVec2 size(vp->WorkSize.x, m_statusBarHeight);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, withAlpha(p.bgPrimary, 0.92f));

    ImGui::Begin("##StatusBar", nullptr, flags);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float textY = (m_statusBarHeight - ImGui::GetTextLineHeight()) * 0.5f;
    ImGui::SetCursorPosY(textY);

    // Top gradient line (pink → cyan → purple)
    ImVec2 lMin(pos.x, pos.y);
    ImVec2 lMid(pos.x + size.x * 0.5f, pos.y + 1);
    ImVec2 lMax(pos.x + size.x, pos.y + 1);
    dl->AddRectFilledMultiColor(
        lMin, lMid,
        toU32(withAlpha(p.accentPink, 0.5f)),
        toU32(withAlpha(p.accent, 0.5f)),
        toU32(withAlpha(p.accent, 0.5f)),
        toU32(withAlpha(p.accentPink, 0.5f)));
    dl->AddRectFilledMultiColor(
        ImVec2(lMid.x, lMin.y), lMax,
        toU32(withAlpha(p.accent, 0.5f)),
        toU32(withAlpha(p.accentPurple, 0.5f)),
        toU32(withAlpha(p.accentPurple, 0.5f)),
        toU32(withAlpha(p.accent, 0.5f)));

    // ─── LED + ComfyUI Status ───
    auto state = ComfyProcess::instance().getState();
    const char* stateText;
    ImVec4 stateColor;

    switch (state) {
        case ComfyProcess::State::Stopped:
            stateText = "ComfyUI: Stopped"; stateColor = p.textMuted; break;
        case ComfyProcess::State::Starting:
            stateText = "ComfyUI: Starting..."; stateColor = p.warning; break;
        case ComfyProcess::State::Running:
            stateText = "ComfyUI: Running"; stateColor = p.success; break;
        case ComfyProcess::State::Error:
            stateText = "ComfyUI: Error"; stateColor = p.error; break;
    }

    // Status LED dot
    ImVec2 ledPos = ImGui::GetCursorScreenPos();
    ImVec2 ledCenter(ledPos.x + 5, pos.y + m_statusBarHeight * 0.5f);
    dl->AddCircleFilled(ledCenter, 3.5f, toU32(stateColor));
    if (state == ComfyProcess::State::Running) {
        dl->AddCircleFilled(ledCenter, 6.0f, toU32(withAlpha(stateColor, 0.2f)));
    }

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 14);
    ImGui::TextColored(stateColor, "%s", stateText);

    ImGui::SameLine(0, 20);

    // Version badge (like ZipCrackerUI)
    ImVec2 verPos = ImGui::GetCursorScreenPos();
    float verY = pos.y + (m_statusBarHeight - 18) * 0.5f;
    ImVec2 verMin(verPos.x, verY);
    ImVec2 verMax(verPos.x + 60, verY + 18);
    dl->AddRectFilled(verMin, verMax, toU32(withAlpha(p.accent, 0.08f)), 4.0f);
    dl->AddText(ImVec2(verMin.x + 6, verY + 2), toU32(withAlpha(p.accent, 0.7f)), "v0.1.0");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 68);

    // Right side: AI provider
    float rightX = vp->WorkSize.x - 200;
    ImGui::SameLine(rightX);
    ImGui::TextColored(p.textSecondary, "AI: %s",
        AIManager::instance().getActiveProvider().c_str());

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// ═══════════════════════════════════════════════════════════════════════════════
// CHAT PAGE — Neon card style
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderChatPage() {
    const auto& p = Theme::getPalette();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 16));
    ImGui::BeginChild("ChatPage", avail, false);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Header with neon accent
    ImGui::PushStyleColor(ImGuiCol_Text, p.accent);
    ImGui::Text("AI Chat");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextColored(p.textMuted, "- %s", I18n::t("chat.welcome").c_str());
    ImGui::Spacing();
    ImGui::Spacing();

    // Messages area (dark card)
    float inputAreaH = 54.0f;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::hex(0x040408));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::BeginChild("ChatMessages", ImVec2(0, avail.y - inputAreaH - 44), true);

    // Neon border on message area
    ImVec2 msgMin = ImGui::GetWindowPos();
    ImVec2 msgMax(msgMin.x + ImGui::GetWindowSize().x, msgMin.y + ImGui::GetWindowSize().y);
    dl->AddRect(msgMin, msgMax, toU32(withAlpha(p.border, 0.3f)), 10.0f);

    auto history = AIManager::instance().getHistory();
    if (history.empty()) {
        float centerY = ImGui::GetContentRegionAvail().y * 0.4f;
        ImGui::SetCursorPosY(centerY);

        const char* placeholder = I18n::t("chat.placeholder").c_str();
        float textW = ImGui::CalcTextSize(placeholder).x;
        float winW = ImGui::GetContentRegionAvail().x;
        if (textW < winW) ImGui::SetCursorPosX((winW - textW) * 0.5f);

        ImGui::TextColored(p.textMuted, "%s", placeholder);
    } else {
        for (const auto& msg : history) {
            if (msg.role == ChatMessage::User) {
                // User message with cyan accent
                ImGui::PushStyleColor(ImGuiCol_Text, p.accent);
                ImGui::TextWrapped("> %s", msg.content.c_str());
                ImGui::PopStyleColor();
            } else {
                // AI message with neon green tint
                ImGui::PushStyleColor(ImGuiCol_Text, p.success);
                ImGui::TextWrapped("%s", msg.content.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::Spacing();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Input area (neon-styled input)
    static char inputBuf[2048] = "";
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Theme::hex(0x0d0d1a));
    ImGui::PushStyleColor(ImGuiCol_Text, p.accent);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14, 12));
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 100);

    bool enter = ImGui::InputText("##ChatInput", inputBuf, sizeof(inputBuf),
                                   ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 8);

    // Send button (gradient-styled, neon cyan)
    ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(p.accent, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.accent, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, withAlpha(p.accent, 0.55f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

    if ((ImGui::Button(I18n::t("chat.send").c_str(), ImVec2(82, 0)) || enter)
        && strlen(inputBuf) > 0) {
        addLog(LogEntry::Info, std::string("Chat: ") + inputBuf);
        inputBuf[0] = '\0';
    }

    // Glow on hover
    if (ImGui::IsItemHovered()) {
        ImVec2 bMin = ImGui::GetItemRectMin();
        ImVec2 bMax = ImGui::GetItemRectMax();
        dl->AddRect(bMin, bMax, toU32(withAlpha(p.accent, 0.5f)), 10.0f, 0, 1.5f);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ═══════════════════════════════════════════════════════════════════════════════
// NODE GRAPH PAGE (Full-screen dark canvas)
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderNodeGraphPage() {
    const auto& p = Theme::getPalette();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("NodeGraphPage", avail, false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Grid background (subtle)
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 canvasMin = ImGui::GetWindowPos();
    ImVec2 canvasMax(canvasMin.x + avail.x, canvasMin.y + avail.y);

    // Dark canvas background
    dl->AddRectFilled(canvasMin, canvasMax, toU32(Theme::hex(0x060610)));

    // Grid dots
    float gridStep = 30.0f;
    for (float x = canvasMin.x; x < canvasMax.x; x += gridStep) {
        for (float y = canvasMin.y; y < canvasMax.y; y += gridStep) {
            dl->AddCircleFilled(ImVec2(x, y), 1.0f, toU32(withAlpha(p.textMuted, 0.15f)));
        }
    }

    // Center placeholder text
    float centerY = avail.y * 0.45f;
    ImGui::SetCursorPosY(centerY);

    const char* text = I18n::t("graph.empty").c_str();
    float textW = ImGui::CalcTextSize(text).x;
    if (textW < avail.x)
        ImGui::SetCursorPosX((avail.x - textW) * 0.5f);

    ImGui::TextColored(p.textMuted, "%s", text);

    ImGui::EndChild();
}

// ═══════════════════════════════════════════════════════════════════════════════
// PREVIEW PAGE
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderPreviewPage() {
    const auto& p = Theme::getPalette();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 16));
    ImGui::BeginChild("PreviewPage", avail, false);

    ImGui::PushStyleColor(ImGuiCol_Text, p.accentPink);
    ImGui::Text("Preview");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    // Center placeholder with neon styling
    float centerY = (avail.y - ImGui::GetTextLineHeight()) * 0.45f;
    ImGui::SetCursorPosY(centerY);

    const char* text = I18n::t("preview.empty").c_str();
    float textW = ImGui::CalcTextSize(text).x;
    float winW = ImGui::GetContentRegionAvail().x;
    if (textW < winW) ImGui::SetCursorPosX((winW - textW) * 0.5f);

    ImGui::TextColored(p.textMuted, "%s", text);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ═══════════════════════════════════════════════════════════════════════════════
// LOG PAGE — Neon-styled console (like ZipCrackerUI's log panels)
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderLogPage() {
    const auto& p = Theme::getPalette();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 16));
    ImGui::BeginChild("LogPage", avail, false);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Header row
    ImGui::PushStyleColor(ImGuiCol_Text, p.warning); // Orange like ZipCrackerUI's log
    ImGui::Text("Activity Log");
    ImGui::PopStyleColor();

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 150);

    // Clear button (neon styled)
    ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(p.error, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.error, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_Text, p.error);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (ImGui::Button(I18n::t("log.clear").c_str())) {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_appLog.clear();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_CheckMark, p.accent);
    ImGui::Checkbox("Auto", &m_logAutoScroll);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Log content (dark terminal-like card)
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::hex(0x040408));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::BeginChild("LogContent", ImVec2(0, 0), true,
        ImGuiWindowFlags_HorizontalScrollbar);

    // Neon border
    ImVec2 logMin = ImGui::GetWindowPos();
    ImVec2 logMax(logMin.x + ImGui::GetWindowSize().x, logMin.y + ImGui::GetWindowSize().y);
    dl->AddRect(logMin, logMax, toU32(withAlpha(p.warning, 0.15f)), 10.0f);

    // Use monospace font if available
    ImGuiIO& io = ImGui::GetIO();
    bool usedMono = false;
    if (io.Fonts->Fonts.Size > 1) {
        ImGui::PushFont(io.Fonts->Fonts[1]);
        usedMono = true;
    }

    // Application logs (color-coded like ZipCrackerUI)
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        for (const auto& entry : m_appLog) {
            ImVec4 color;

            switch (entry.level) {
                case LogEntry::Error: color = p.error; break;      // Neon red
                case LogEntry::Warn:  color = p.warning; break;    // Neon orange
                case LogEntry::Info:  color = p.success; break;    // Neon green (like ZipCrackerUI)
                case LogEntry::Debug: color = p.textMuted; break;
            }

            // Format timestamp
            auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
            char timeBuf[16];
            std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", std::localtime(&time));

            ImGui::TextColored(p.textMuted, "[%s]", timeBuf);
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", entry.message.c_str());
        }
    }

    // ComfyUI process logs
    auto processLogs = ComfyProcess::instance().getLog();
    for (const auto& line : processLogs) {
        ImVec4 color = p.success; // Default neon green (like terminal)

        if (line.find("ERROR") != std::string::npos ||
            line.find("error") != std::string::npos) {
            color = p.error;
        } else if (line.find("WARN") != std::string::npos ||
                   line.find("warn") != std::string::npos) {
            color = p.warning;
        }

        ImGui::TextColored(color, "%s", line.c_str());
    }

    if (usedMono) ImGui::PopFont();

    // Auto-scroll
    if (m_logAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ═══════════════════════════════════════════════════════════════════════════════
// SETTINGS PAGE — Neon cards
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderSettingsPage() {
    const auto& p = Theme::getPalette();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 20));
    ImGui::BeginChild("SettingsPage", avail, false);

    ImGui::PushStyleColor(ImGuiCol_Text, p.accentPurple);
    ImGui::Text("%s", I18n::t("settings.title").c_str());
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    auto& config = Config::instance().get();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    // Tab bar with neon styling
    ImGui::PushStyleColor(ImGuiCol_Tab, Theme::hex(0x0d0d1a));
    ImGui::PushStyleColor(ImGuiCol_TabHovered, withAlpha(p.accent, 0.22f));
    ImGui::PushStyleColor(ImGuiCol_TabSelected, withAlpha(p.accent, 0.30f));

    if (ImGui::BeginTabBar("SettingsTabs")) {
        // ComfyUI Settings
        if (ImGui::BeginTabItem(I18n::t("settings.comfyui").c_str())) {
            ImGui::Spacing();
            const char* modes[] = { "Embedded", "External" };
            int currentMode = config.comfyui.mode == "embedded" ? 0 : 1;
            if (ImGui::Combo(I18n::t("settings.mode").c_str(), &currentMode, modes, 2)) {
                config.comfyui.mode = currentMode == 0 ? "embedded" : "external";
            }

            ImGui::Spacing();
            if (config.comfyui.mode == "external") {
                char urlBuf[256] = {};
                snprintf(urlBuf, sizeof(urlBuf), "%s", config.comfyui.externalUrl.c_str());
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
            ImGui::Spacing();

            const char* providers[] = { "OpenAI (GPT-4o)", "Claude (Sonnet)", "Gemini (Flash)", "Local AI (Qwen)" };
            const char* providerKeys[] = { "openai", "claude", "gemini", "local" };
            int currentProvider = 0;
            for (int i = 0; i < 4; i++) {
                if (config.ai.activeProvider == providerKeys[i]) { currentProvider = i; break; }
            }
            if (ImGui::Combo("Provider", &currentProvider, providers, 4)) {
                config.ai.activeProvider = providerKeys[currentProvider];
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("OpenAI")) {
                char buf[256] = {};
                snprintf(buf, sizeof(buf), "%s", config.ai.openaiApiKey.c_str());
                if (ImGui::InputText("API Key##openai", buf, sizeof(buf), ImGuiInputTextFlags_Password)) {
                    config.ai.openaiApiKey = buf;
                }
            }
            if (ImGui::CollapsingHeader("Anthropic Claude")) {
                char buf[256] = {};
                snprintf(buf, sizeof(buf), "%s", config.ai.claudeApiKey.c_str());
                if (ImGui::InputText("API Key##claude", buf, sizeof(buf), ImGuiInputTextFlags_Password)) {
                    config.ai.claudeApiKey = buf;
                }
            }
            if (ImGui::CollapsingHeader("Google Gemini")) {
                char buf[256] = {};
                snprintf(buf, sizeof(buf), "%s", config.ai.geminiApiKey.c_str());
                if (ImGui::InputText("API Key##gemini", buf, sizeof(buf), ImGuiInputTextFlags_Password)) {
                    config.ai.geminiApiKey = buf;
                }
            }
            if (ImGui::CollapsingHeader("Local AI (llama.cpp)")) {
                char buf[512] = {};
                snprintf(buf, sizeof(buf), "%s", config.ai.localModelPath.c_str());
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
            ImGui::Spacing();

            const char* themes[] = { "Neon Dark", "Midnight Studio", "Light" };
            int currentTheme = 0;
            if (config.theme == "midnight") currentTheme = 1;
            else if (config.theme == "light") currentTheme = 2;

            if (ImGui::Combo(I18n::t("settings.theme").c_str(), &currentTheme, themes, 3)) {
                switch (currentTheme) {
                    case 0: config.theme = "modern"; break;
                    case 1: config.theme = "midnight"; break;
                    case 2: config.theme = "light"; break;
                }
                Theme::Style s = Theme::Style::Modern;
                if (config.theme == "midnight") s = Theme::Style::Midnight;
                else if (config.theme == "light") s = Theme::Style::Light;
                Theme::apply(s);
            }

            ImGui::SliderFloat(I18n::t("settings.scale").c_str(),
                &config.uiScale, 0.8f, 2.0f, "%.1f");

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

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::Spacing();
    ImGui::Spacing();

    // Save button (neon cyan)
    ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(p.accent, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.accent, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    if (ImGui::Button(I18n::t("settings.save").c_str(), ImVec2(120, 36))) {
        Config::instance().save();
        addLog(LogEntry::Info, "Settings saved");
    }

    // Glow on hover
    if (ImGui::IsItemHovered()) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 bMin = ImGui::GetItemRectMin();
        ImVec2 bMax = ImGui::GetItemRectMax();
        dl->AddRect(bMin, bMax, toU32(withAlpha(p.accent, 0.5f)), 8.0f, 0, 1.5f);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, 8);

    // Cancel button (subtle)
    ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(p.textMuted, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.textMuted, 0.22f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    if (ImGui::Button(I18n::t("settings.cancel").c_str(), ImVec2(120, 36))) {
        m_activePage = Page::Chat;
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ═══════════════════════════════════════════════════════════════════════════════
// LICENSE DIALOG — Neon-styled popup
// ═══════════════════════════════════════════════════════════════════════════════

void MainWindow::renderLicenseDialog() {
    const auto& p = Theme::getPalette();

    ImGui::SetNextWindowSize(ImVec2(460, 360), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, p.bgElevated);

    if (ImGui::Begin(I18n::t("license.title").c_str(), &m_showLicense)) {
        ImGui::TextWrapped("%s", I18n::t("license.info").c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        static char licenseKey[64] = "";
        ImGui::PushStyleColor(ImGuiCol_FrameBg, Theme::hex(0x0d0d1a));
        ImGui::PushStyleColor(ImGuiCol_Text, p.accent);
        ImGui::InputText(I18n::t("license.key").c_str(), licenseKey, sizeof(licenseKey));
        ImGui::PopStyleColor(2);

        ImGui::Spacing();

        // Activate button (neon cyan)
        ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(p.accent, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.accent, 0.40f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        if (ImGui::Button(I18n::t("license.activate").c_str(), ImVec2(140, 34))) {
            // TODO: Activate via xmanstudio API
        }
        ImGui::SameLine();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        // Trial button (neon pink)
        ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(p.accentPink, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(p.accentPink, 0.30f));
        ImGui::PushStyleColor(ImGuiCol_Text, p.accentPink);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        if (ImGui::Button(I18n::t("license.trial").c_str(), ImVec2(140, 34))) {
            // TODO: Start trial via xmanstudio API
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(p.textMuted, "Status: Trial (7 days remaining)");
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void MainWindow::shutdown() {
    addLog(LogEntry::Info, "ComfyX shutting down");
}

} // namespace ComfyX
