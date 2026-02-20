#pragma once
#include <string>
#include <functional>
#include <vector>
#include <nlohmann/json.hpp>

namespace ComfyX {

class ComfyClient {
public:
    static ComfyClient& instance();

    void setBaseUrl(const std::string& url);
    bool connect();
    bool isConnected() const { return m_connected; }

    // API Methods
    nlohmann::json getObjectInfo();
    nlohmann::json getObjectInfo(const std::string& nodeClass);
    nlohmann::json getSystemStats();

    // Workflow execution
    struct PromptResult {
        bool success;
        std::string promptId;
        std::string error;
    };
    PromptResult queuePrompt(const nlohmann::json& workflow, const std::string& clientId = "");

    nlohmann::json getHistory(const std::string& promptId);
    nlohmann::json getQueue();

    // File operations
    std::vector<uint8_t> getImage(const std::string& filename,
                                   const std::string& subfolder = "",
                                   const std::string& type = "output");
    bool uploadImage(const std::string& filepath, const std::string& subfolder = "");

    // Control
    bool interrupt();
    bool freeMemory(bool unloadModels = true);

    // Models
    nlohmann::json getModels(const std::string& folder = "");
    nlohmann::json getEmbeddings();

private:
    ComfyClient() = default;

    std::string m_baseUrl = "http://127.0.0.1:8188";
    bool m_connected = false;
    std::string m_clientId;

    std::string httpGet(const std::string& path);
    std::string httpPost(const std::string& path, const std::string& body,
                         const std::string& contentType = "application/json");
};

} // namespace ComfyX
