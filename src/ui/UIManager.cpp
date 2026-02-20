#include "UIManager.h"
#include "app/PortablePaths.h"
#include "app/Config.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>

namespace ComfyX {

UIManager& UIManager::instance() {
    static UIManager inst;
    return inst;
}

bool UIManager::initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = nullptr; // We manage layout ourselves (portable)

    // Platform/Renderer setup
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load fonts
    loadFonts();

    // Apply theme
    auto& config = Config::instance().get();
    if (config.theme == "light") {
        setTheme(Theme::Style::Light);
    } else if (config.theme == "midnight") {
        setTheme(Theme::Style::Midnight);
    } else {
        setTheme(Theme::Style::Modern);
    }

    m_scale = config.uiScale;
    m_initialized = true;
    return true;
}

bool UIManager::loadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    auto& paths = PortablePaths::instance();

    float fontSize = 16.0f * m_scale;

    // Try to load custom fonts
    auto notoSansPath = paths.fontsDir() / "NotoSans-Regular.ttf";
    auto notoSansThaiPath = paths.fontsDir() / "NotoSansThai-Regular.ttf";
    auto jetBrainsPath = paths.fontsDir() / "JetBrainsMono-Regular.ttf";

    // Default font (English)
    if (std::filesystem::exists(notoSansPath)) {
        ImFontConfig config;
        config.SizePixels = fontSize;
        io.Fonts->AddFontFromFileTTF(notoSansPath.string().c_str(), fontSize, &config,
                                     io.Fonts->GetGlyphRangesDefault());
        std::cout << "[UI] Loaded NotoSans font" << std::endl;
    } else {
        io.Fonts->AddFontDefault();
        std::cout << "[UI] Using default font (NotoSans not found)" << std::endl;
    }

    // Thai font (merged into default)
    if (std::filesystem::exists(notoSansThaiPath)) {
        ImFontConfig thaiConfig;
        thaiConfig.MergeMode = true;

        // Thai Unicode range: U+0E00 to U+0E7F
        static const ImWchar thaiRanges[] = { 0x0E00, 0x0E7F, 0 };
        io.Fonts->AddFontFromFileTTF(notoSansThaiPath.string().c_str(), fontSize, &thaiConfig, thaiRanges);
        std::cout << "[UI] Loaded NotoSansThai font (merged)" << std::endl;
    }

    // Monospace font for code display
    if (std::filesystem::exists(jetBrainsPath)) {
        ImFontConfig monoConfig;
        monoConfig.SizePixels = 14.0f * m_scale;
        io.Fonts->AddFontFromFileTTF(jetBrainsPath.string().c_str(), 14.0f * m_scale, &monoConfig);
        std::cout << "[UI] Loaded JetBrainsMono font" << std::endl;
    }

    io.Fonts->Build();
    return true;
}

void UIManager::shutdown() {
    if (!m_initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

void UIManager::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}

void UIManager::setTheme(Theme::Style style) {
    m_currentTheme = style;
    Theme::apply(style);
}

void UIManager::setLanguage(const std::string& lang) {
    Config::instance().get().language = lang;
    Config::instance().save();
}

void UIManager::setScale(float scale) {
    m_scale = scale;
    Config::instance().get().uiScale = scale;
}

} // namespace ComfyX
