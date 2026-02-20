#include "I18n.h"
#include "app/PortablePaths.h"

#include <fstream>
#include <iostream>
#include <filesystem>

namespace ComfyX {

const std::string I18n::s_missing = "[???]";

I18n& I18n::instance() {
    static I18n inst;
    return inst;
}

void I18n::initialize(const std::string& language) {
    m_language = language;
    loadDefaults();
    loadTranslations();
}

void I18n::setLanguage(const std::string& language) {
    m_language = language;
    loadDefaults();
    loadTranslations();
}

void I18n::loadDefaults() {
    // English defaults (always loaded as fallback)
    m_translations = {
        // Menu
        {"menu.file", "File"},
        {"menu.new_workflow", "New Workflow"},
        {"menu.open_workflow", "Open Workflow..."},
        {"menu.save_workflow", "Save Workflow"},
        {"menu.settings", "Settings"},
        {"menu.exit", "Exit"},
        {"menu.comfyui", "ComfyUI"},
        {"menu.connect", "Connect"},
        {"menu.disconnect", "Disconnect"},
        {"menu.start_embedded", "Start Embedded Server"},
        {"menu.stop_embedded", "Stop Embedded Server"},
        {"menu.ai", "AI Engine"},
        {"menu.help", "Help"},
        {"menu.license", "License"},
        {"menu.about", "About"},

        // Chat Panel
        {"chat.welcome", "Welcome to ComfyX! Describe the workflow you want and AI will generate it for you."},
        {"chat.placeholder", "No messages yet. Type a prompt below to start generating workflows."},
        {"chat.send", "Send"},
        {"chat.input_hint", "Describe your workflow... (e.g., 'Generate a portrait with SDXL')"},

        // Node Graph
        {"graph.empty", "No workflow loaded. Generate one using the AI Chat panel."},
        {"graph.title", "Node Graph"},

        // Preview
        {"preview.empty", "No preview available. Execute a workflow to see results here."},
        {"preview.title", "Preview"},

        // History
        {"history.empty", "No workflow history yet."},
        {"history.title", "History"},

        // Settings
        {"settings.title", "Settings"},
        {"settings.comfyui", "ComfyUI"},
        {"settings.ai", "AI Engine"},
        {"settings.appearance", "Appearance"},
        {"settings.mode", "Mode"},
        {"settings.port", "Port"},
        {"settings.autostart", "Auto-start on launch"},
        {"settings.theme", "Theme"},
        {"settings.scale", "UI Scale"},
        {"settings.language", "Language"},
        {"settings.save", "Save"},
        {"settings.cancel", "Cancel"},

        // License
        {"license.title", "License"},
        {"license.info", "Enter your license key to unlock all features, or start a free trial."},
        {"license.key", "License Key"},
        {"license.activate", "Activate"},
        {"license.trial", "Start Trial"},

        // Status
        {"status.connected", "Connected"},
        {"status.disconnected", "Disconnected"},
        {"status.running", "Running"},
        {"status.generating", "Generating..."},
    };
}

void I18n::loadTranslations() {
    if (m_language == "en") return; // Defaults are English

    // Load language file from assets/i18n/ directory
    auto langPath = PortablePaths::instance().assetsDir() / "i18n" / (m_language + ".json");

    std::filesystem::path pathToLoad;
    if (std::filesystem::exists(langPath)) {
        pathToLoad = langPath;
    } else {
        std::cout << "[I18n] Translation file not found for: " << m_language << std::endl;
        return;
    }

    try {
        std::ifstream file(pathToLoad);
        nlohmann::json j;
        file >> j;

        for (auto& [key, value] : j.items()) {
            if (value.is_string()) {
                m_translations[key] = value.get<std::string>();
            }
        }
        std::cout << "[I18n] Loaded translations for: " << m_language << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[I18n] Failed to load translations: " << e.what() << std::endl;
    }
}

const std::string& I18n::translate(const std::string& key) const {
    auto it = m_translations.find(key);
    if (it != m_translations.end()) {
        return it->second;
    }
    return s_missing;
}

} // namespace ComfyX
