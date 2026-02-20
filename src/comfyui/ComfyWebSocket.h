#pragma once
#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <nlohmann/json.hpp>

namespace ix { class WebSocket; }

namespace ComfyX {

struct ComfyProgress {
    std::string nodeId;
    int value = 0;
    int max = 0;
    float percent() const { return max > 0 ? (float)value / max * 100.0f : 0.0f; }
};

class ComfyWebSocket {
public:
    static ComfyWebSocket& instance();

    using StatusCallback = std::function<void(const nlohmann::json&)>;
    using ProgressCallback = std::function<void(const ComfyProgress&)>;
    using ExecutedCallback = std::function<void(const std::string& nodeId, const nlohmann::json& output)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    void connect(const std::string& url, const std::string& clientId);
    void disconnect();
    bool isConnected() const { return m_connected; }

    void onStatus(StatusCallback cb) { m_onStatus = cb; }
    void onProgress(ProgressCallback cb) { m_onProgress = cb; }
    void onExecuted(ExecutedCallback cb) { m_onExecuted = cb; }
    void onError(ErrorCallback cb) { m_onError = cb; }

private:
    ComfyWebSocket() = default;

    void handleMessage(const std::string& msg);

    std::unique_ptr<ix::WebSocket> m_ws;
    std::atomic<bool> m_connected{false};

    StatusCallback m_onStatus;
    ProgressCallback m_onProgress;
    ExecutedCallback m_onExecuted;
    ErrorCallback m_onError;
};

} // namespace ComfyX
