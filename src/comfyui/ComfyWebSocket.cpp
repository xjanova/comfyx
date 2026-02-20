#include "ComfyWebSocket.h"
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>

namespace ComfyX {

ComfyWebSocket& ComfyWebSocket::instance() {
    static ComfyWebSocket inst;
    return inst;
}

void ComfyWebSocket::connect(const std::string& url, const std::string& clientId) {
    disconnect();

    std::string wsUrl = url + "/ws?clientId=" + clientId;

    // Replace http with ws
    if (wsUrl.substr(0, 7) == "http://") {
        wsUrl = "ws://" + wsUrl.substr(7);
    } else if (wsUrl.substr(0, 8) == "https://") {
        wsUrl = "wss://" + wsUrl.substr(8);
    }

    m_ws = std::make_unique<ix::WebSocket>();
    m_ws->setUrl(wsUrl);

    m_ws->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        switch (msg->type) {
            case ix::WebSocketMessageType::Open:
                m_connected = true;
                std::cout << "[WS] Connected" << std::endl;
                break;

            case ix::WebSocketMessageType::Close:
                m_connected = false;
                std::cout << "[WS] Disconnected" << std::endl;
                break;

            case ix::WebSocketMessageType::Message:
                handleMessage(msg->str);
                break;

            case ix::WebSocketMessageType::Error:
                m_connected = false;
                if (m_onError) m_onError(msg->errorInfo.reason);
                break;

            default:
                break;
        }
    });

    m_ws->start();
}

void ComfyWebSocket::disconnect() {
    if (m_ws) {
        m_ws->stop();
        m_ws.reset();
    }
    m_connected = false;
}

void ComfyWebSocket::handleMessage(const std::string& msg) {
    try {
        auto j = nlohmann::json::parse(msg);
        std::string type = j.value("type", "");

        if (type == "status") {
            if (m_onStatus) m_onStatus(j["data"]);
        }
        else if (type == "progress") {
            ComfyProgress progress;
            progress.value = j["data"]["value"].get<int>();
            progress.max = j["data"]["max"].get<int>();
            if (j["data"].contains("node")) {
                progress.nodeId = j["data"]["node"].get<std::string>();
            }
            if (m_onProgress) m_onProgress(progress);
        }
        else if (type == "executing") {
            // Currently executing node
            if (m_onStatus) m_onStatus(j["data"]);
        }
        else if (type == "executed") {
            std::string nodeId = j["data"]["node"].get<std::string>();
            if (m_onExecuted) m_onExecuted(nodeId, j["data"]["output"]);
        }
        else if (type == "execution_error") {
            if (m_onError) m_onError(j["data"].dump());
        }
    } catch (const std::exception& e) {
        std::cerr << "[WS] Parse error: " << e.what() << std::endl;
    }
}

} // namespace ComfyX
