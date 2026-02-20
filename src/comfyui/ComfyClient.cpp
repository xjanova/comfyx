#include "ComfyClient.h"
#include <httplib.h>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>

namespace ComfyX {

static std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << dist(gen) << "-";
    ss << std::setw(4) << (dist(gen) & 0xFFFF) << "-";
    ss << std::setw(4) << ((dist(gen) & 0x0FFF) | 0x4000) << "-";
    ss << std::setw(4) << ((dist(gen) & 0x3FFF) | 0x8000) << "-";
    ss << std::setw(8) << dist(gen) << std::setw(4) << (dist(gen) & 0xFFFF);
    return ss.str();
}

ComfyClient& ComfyClient::instance() {
    static ComfyClient inst;
    return inst;
}

void ComfyClient::setBaseUrl(const std::string& url) {
    m_baseUrl = url;
    m_connected = false;
}

bool ComfyClient::connect() {
    m_clientId = generateUUID();

    try {
        auto stats = httpGet("/system_stats");
        if (!stats.empty()) {
            auto j = nlohmann::json::parse(stats);
            m_connected = true;
            std::cout << "[ComfyUI] Connected to " << m_baseUrl << std::endl;
            return true;
        }
    } catch (...) {}

    m_connected = false;
    std::cerr << "[ComfyUI] Failed to connect to " << m_baseUrl << std::endl;
    return false;
}

nlohmann::json ComfyClient::getObjectInfo() {
    auto response = httpGet("/object_info");
    if (response.empty()) return {};
    try { return nlohmann::json::parse(response); }
    catch (...) { return {}; }
}

nlohmann::json ComfyClient::getObjectInfo(const std::string& nodeClass) {
    auto response = httpGet("/object_info/" + nodeClass);
    if (response.empty()) return {};
    try { return nlohmann::json::parse(response); }
    catch (...) { return {}; }
}

nlohmann::json ComfyClient::getSystemStats() {
    auto response = httpGet("/system_stats");
    if (response.empty()) return {};
    try { return nlohmann::json::parse(response); }
    catch (...) { return {}; }
}

ComfyClient::PromptResult ComfyClient::queuePrompt(const nlohmann::json& workflow,
                                                     const std::string& clientId) {
    PromptResult result;
    nlohmann::json body;
    body["prompt"] = workflow;
    body["client_id"] = clientId.empty() ? m_clientId : clientId;

    auto response = httpPost("/prompt", body.dump());
    if (response.empty()) {
        result.success = false;
        result.error = "No response from server";
        return result;
    }

    try {
        auto j = nlohmann::json::parse(response);
        if (j.contains("prompt_id")) {
            result.success = true;
            result.promptId = j["prompt_id"].get<std::string>();
        } else if (j.contains("error")) {
            result.success = false;
            result.error = j["error"].dump();
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.error = e.what();
    }

    return result;
}

nlohmann::json ComfyClient::getHistory(const std::string& promptId) {
    auto response = httpGet("/history/" + promptId);
    if (response.empty()) return {};
    try { return nlohmann::json::parse(response); }
    catch (...) { return {}; }
}

nlohmann::json ComfyClient::getQueue() {
    auto response = httpGet("/queue");
    if (response.empty()) return {};
    try { return nlohmann::json::parse(response); }
    catch (...) { return {}; }
}

std::vector<uint8_t> ComfyClient::getImage(const std::string& filename,
                                             const std::string& subfolder,
                                             const std::string& type) {
    std::string path = "/view?filename=" + filename;
    if (!subfolder.empty()) path += "&subfolder=" + subfolder;
    path += "&type=" + type;

    try {
        httplib::Client cli(m_baseUrl);
        cli.set_connection_timeout(10);
        auto res = cli.Get(path);
        if (res && res->status == 200) {
            return std::vector<uint8_t>(res->body.begin(), res->body.end());
        }
    } catch (...) {}

    return {};
}

bool ComfyClient::uploadImage(const std::string& filepath, const std::string& subfolder) {
    try {
        httplib::Client cli(m_baseUrl);
        cli.set_connection_timeout(30);

        httplib::UploadFormDataItems items;
        std::ifstream file(filepath, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

        items.push_back({"image", content, std::filesystem::path(filepath).filename().string(), "image/png"});
        if (!subfolder.empty()) {
            items.push_back({"subfolder", subfolder, "", ""});
        }

        auto res = cli.Post("/upload/image", items);
        return res && res->status == 200;
    } catch (...) {
        return false;
    }
}

bool ComfyClient::interrupt() {
    auto response = httpPost("/interrupt", "");
    return !response.empty();
}

bool ComfyClient::freeMemory(bool unloadModels) {
    nlohmann::json body;
    body["unload_models"] = unloadModels;
    body["free_memory"] = true;
    auto response = httpPost("/free", body.dump());
    return !response.empty();
}

nlohmann::json ComfyClient::getModels(const std::string& folder) {
    std::string path = folder.empty() ? "/models" : "/models/" + folder;
    auto response = httpGet(path);
    if (response.empty()) return {};
    try { return nlohmann::json::parse(response); }
    catch (...) { return {}; }
}

nlohmann::json ComfyClient::getEmbeddings() {
    auto response = httpGet("/embeddings");
    if (response.empty()) return {};
    try { return nlohmann::json::parse(response); }
    catch (...) { return {}; }
}

std::string ComfyClient::httpGet(const std::string& path) {
    try {
        httplib::Client cli(m_baseUrl);
        cli.set_connection_timeout(10);
        cli.set_read_timeout(60);

        auto res = cli.Get(path);
        if (res && res->status == 200) {
            return res->body;
        }
    } catch (const std::exception& e) {
        std::cerr << "[ComfyUI] GET " << path << " failed: " << e.what() << std::endl;
    }
    return "";
}

std::string ComfyClient::httpPost(const std::string& path, const std::string& body,
                                    const std::string& contentType) {
    try {
        httplib::Client cli(m_baseUrl);
        cli.set_connection_timeout(10);
        cli.set_read_timeout(60);

        auto res = cli.Post(path, body, contentType);
        if (res && (res->status == 200 || res->status == 201)) {
            return res->body;
        }
    } catch (const std::exception& e) {
        std::cerr << "[ComfyUI] POST " << path << " failed: " << e.what() << std::endl;
    }
    return "";
}

} // namespace ComfyX
