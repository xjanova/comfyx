#include "CloudAI.h"
#include "app/Config.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>

namespace ComfyX {

std::string CloudAI::generate(const std::string& systemPrompt,
                                const std::string& userPrompt,
                                const std::string& provider,
                                StreamCallback onToken) {
    if (provider == "openai") return generateOpenAI(systemPrompt, userPrompt, onToken);
    if (provider == "claude") return generateClaude(systemPrompt, userPrompt, onToken);
    if (provider == "gemini") return generateGemini(systemPrompt, userPrompt, onToken);
    return "Unknown provider: " + provider;
}

std::string CloudAI::generateOpenAI(const std::string& systemPrompt,
                                      const std::string& userPrompt,
                                      StreamCallback onToken) {
    auto& config = Config::instance().get().ai;

    if (config.openaiApiKey.empty()) {
        return "[Error] OpenAI API key not configured. Set it in Settings > AI Engine.";
    }

    nlohmann::json body;
    body["model"] = config.openaiModel;
    body["messages"] = nlohmann::json::array({
        {{"role", "system"}, {"content", systemPrompt}},
        {{"role", "user"}, {"content", userPrompt}}
    });
    body["temperature"] = 0.7;
    body["max_tokens"] = 4096;
    body["stream"] = false; // Non-streaming for simplicity

    try {
        httplib::Client cli("https://api.openai.com");
        cli.set_connection_timeout(30);
        cli.set_read_timeout(120);

        httplib::Headers headers = {
            {"Authorization", "Bearer " + config.openaiApiKey},
            {"Content-Type", "application/json"}
        };

        auto res = cli.Post("/v1/chat/completions", headers, body.dump(), "application/json");

        if (res && res->status == 200) {
            auto j = nlohmann::json::parse(res->body);
            std::string content = j["choices"][0]["message"]["content"].get<std::string>();
            if (onToken) onToken(content);
            return content;
        } else {
            std::string error = res ? "HTTP " + std::to_string(res->status) + ": " + res->body
                                    : "Connection failed";
            std::cerr << "[OpenAI] " << error << std::endl;
            return "[Error] OpenAI: " + error;
        }
    } catch (const std::exception& e) {
        return "[Error] OpenAI: " + std::string(e.what());
    }
}

std::string CloudAI::generateClaude(const std::string& systemPrompt,
                                      const std::string& userPrompt,
                                      StreamCallback onToken) {
    auto& config = Config::instance().get().ai;

    if (config.claudeApiKey.empty()) {
        return "[Error] Claude API key not configured. Set it in Settings > AI Engine.";
    }

    nlohmann::json body;
    body["model"] = config.claudeModel;
    body["max_tokens"] = 4096;
    body["system"] = systemPrompt;
    body["messages"] = nlohmann::json::array({
        {{"role", "user"}, {"content", userPrompt}}
    });

    try {
        httplib::Client cli("https://api.anthropic.com");
        cli.set_connection_timeout(30);
        cli.set_read_timeout(120);

        httplib::Headers headers = {
            {"x-api-key", config.claudeApiKey},
            {"anthropic-version", "2023-06-01"},
            {"Content-Type", "application/json"}
        };

        auto res = cli.Post("/v1/messages", headers, body.dump(), "application/json");

        if (res && res->status == 200) {
            auto j = nlohmann::json::parse(res->body);
            std::string content;
            for (auto& block : j["content"]) {
                if (block["type"] == "text") {
                    content += block["text"].get<std::string>();
                }
            }
            if (onToken) onToken(content);
            return content;
        } else {
            std::string error = res ? "HTTP " + std::to_string(res->status) + ": " + res->body
                                    : "Connection failed";
            std::cerr << "[Claude] " << error << std::endl;
            return "[Error] Claude: " + error;
        }
    } catch (const std::exception& e) {
        return "[Error] Claude: " + std::string(e.what());
    }
}

std::string CloudAI::generateGemini(const std::string& systemPrompt,
                                      const std::string& userPrompt,
                                      StreamCallback onToken) {
    auto& config = Config::instance().get().ai;

    if (config.geminiApiKey.empty()) {
        return "[Error] Gemini API key not configured. Set it in Settings > AI Engine.";
    }

    nlohmann::json body;
    body["contents"] = nlohmann::json::array({
        {{"role", "user"}, {"parts", nlohmann::json::array({{{"text", userPrompt}}})}},
    });
    body["systemInstruction"] = {{"parts", nlohmann::json::array({{{"text", systemPrompt}}})}};
    body["generationConfig"] = {
        {"temperature", 0.7},
        {"maxOutputTokens", 4096}
    };

    try {
        std::string url = "/v1beta/models/" + config.geminiModel + ":generateContent?key=" + config.geminiApiKey;

        httplib::Client cli("https://generativelanguage.googleapis.com");
        cli.set_connection_timeout(30);
        cli.set_read_timeout(120);

        auto res = cli.Post(url, body.dump(), "application/json");

        if (res && res->status == 200) {
            auto j = nlohmann::json::parse(res->body);
            std::string content;
            if (j.contains("candidates") && !j["candidates"].empty()) {
                for (auto& part : j["candidates"][0]["content"]["parts"]) {
                    content += part["text"].get<std::string>();
                }
            }
            if (onToken) onToken(content);
            return content;
        } else {
            std::string error = res ? "HTTP " + std::to_string(res->status) + ": " + res->body
                                    : "Connection failed";
            std::cerr << "[Gemini] " << error << std::endl;
            return "[Error] Gemini: " + error;
        }
    } catch (const std::exception& e) {
        return "[Error] Gemini: " + std::string(e.what());
    }
}

} // namespace ComfyX
