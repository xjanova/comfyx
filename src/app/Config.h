#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace ComfyX {

struct AIConfig {
    // Cloud AI
    std::string openaiApiKey;
    std::string openaiModel = "gpt-4o";
    std::string claudeApiKey;
    std::string claudeModel = "claude-sonnet-4-20250514";
    std::string geminiApiKey;
    std::string geminiModel = "gemini-2.0-flash";
    std::string activeProvider = "openai"; // openai, claude, gemini, local

    // Local AI
    std::string localModelPath;
    int localGpuLayers = 35;
    int localContextSize = 4096;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AIConfig,
        openaiApiKey, openaiModel,
        claudeApiKey, claudeModel,
        geminiApiKey, geminiModel,
        activeProvider,
        localModelPath, localGpuLayers, localContextSize)
};

struct ComfyUIConfig {
    std::string mode = "external"; // "embedded" or "external"
    std::string externalUrl = "http://127.0.0.1:8188";
    int embeddedPort = 8188;
    bool autoStart = true;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ComfyUIConfig,
        mode, externalUrl, embeddedPort, autoStart)
};

struct AppConfig {
    std::string language = "en"; // "en" or "th"
    float uiScale = 1.0f;
    std::string theme = "modern"; // "modern", "midnight", "light"
    bool showWelcome = true;

    AIConfig ai;
    ComfyUIConfig comfyui;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AppConfig,
        language, uiScale, theme, showWelcome, ai, comfyui)
};

class Config {
public:
    static Config& instance();

    void load();
    void save();

    AppConfig& get() { return m_config; }
    const AppConfig& get() const { return m_config; }

private:
    Config() = default;
    AppConfig m_config;
};

} // namespace ComfyX
