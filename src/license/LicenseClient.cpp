#include "LicenseClient.h"
#include "MachineFingerprint.h"
#include "app/PortablePaths.h"

#include <httplib.h>
#include <fstream>
#include <iostream>

namespace ComfyX {

LicenseClient& LicenseClient::instance() {
    static LicenseClient inst;
    return inst;
}

void LicenseClient::setServerUrl(const std::string& url) {
    m_serverUrl = url;
}

bool LicenseClient::registerDevice() {
    nlohmann::json data;
    data["machine_id"] = MachineFingerprint::getMachineId();
    data["os"] = MachineFingerprint::getOSInfo();
    data["app_version"] = m_appVersion;
    data["hardware_hash"] = MachineFingerprint::getFingerprint();

    auto response = post("/api/v1/product/" + m_productSlug + "/device/register", data);
    if (response.empty()) return false;

    try {
        auto j = nlohmann::json::parse(response);
        return j.value("success", false);
    } catch (...) {
        return false;
    }
}

LicenseInfo LicenseClient::activate(const std::string& licenseKey) {
    nlohmann::json data;
    data["license_key"] = licenseKey;
    data["machine_id"] = MachineFingerprint::getMachineId();
    data["machine_fingerprint"] = MachineFingerprint::getFingerprint();
    data["app_version"] = m_appVersion;

    auto response = post("/api/v1/product/" + m_productSlug + "/activate", data);
    auto info = parseLicenseResponse(response);

    if (info.valid) {
        info.licenseKey = licenseKey;
        m_cachedInfo = info;
        saveCachedLicense();
    }

    return info;
}

LicenseInfo LicenseClient::validate() {
    if (m_cachedInfo.licenseKey.empty()) {
        loadCachedLicense();
    }

    if (m_cachedInfo.licenseKey.empty()) {
        return m_cachedInfo;
    }

    nlohmann::json data;
    data["license_key"] = m_cachedInfo.licenseKey;
    data["machine_id"] = MachineFingerprint::getMachineId();

    auto response = post("/api/v1/product/" + m_productSlug + "/validate", data);
    auto info = parseLicenseResponse(response);
    info.licenseKey = m_cachedInfo.licenseKey;

    m_cachedInfo = info;
    saveCachedLicense();

    return info;
}

bool LicenseClient::deactivate() {
    nlohmann::json data;
    data["license_key"] = m_cachedInfo.licenseKey;
    data["machine_id"] = MachineFingerprint::getMachineId();

    auto response = post("/api/v1/product/" + m_productSlug + "/deactivate", data);

    try {
        auto j = nlohmann::json::parse(response);
        if (j.value("success", false)) {
            m_cachedInfo = LicenseInfo{};
            saveCachedLicense();
            return true;
        }
    } catch (...) {}

    return false;
}

LicenseInfo LicenseClient::startDemo() {
    nlohmann::json data;
    data["machine_id"] = MachineFingerprint::getMachineId();
    data["machine_fingerprint"] = MachineFingerprint::getFingerprint();

    auto response = post("/api/v1/product/" + m_productSlug + "/demo/start", data);
    auto info = parseLicenseResponse(response);

    if (info.valid) {
        info.licenseType = "demo";
        m_cachedInfo = info;
        saveCachedLicense();
    }

    return info;
}

LicenseInfo LicenseClient::checkDemo() {
    nlohmann::json data;
    data["machine_id"] = MachineFingerprint::getMachineId();

    auto response = post("/api/v1/product/" + m_productSlug + "/demo/check", data);
    return parseLicenseResponse(response);
}

LicenseInfo LicenseClient::getStatus(const std::string& licenseKey) {
    auto response = get("/api/v1/product/" + m_productSlug + "/status/" + licenseKey);
    return parseLicenseResponse(response);
}

bool LicenseClient::hasCachedLicense() const {
    return !m_cachedInfo.licenseKey.empty() || m_cachedInfo.licenseType == "demo";
}

void LicenseClient::loadCachedLicense() {
    auto path = PortablePaths::instance().licenseFile();
    if (!std::filesystem::exists(path)) return;

    try {
        std::ifstream file(path);
        nlohmann::json j;
        file >> j;

        m_cachedInfo.licenseKey = j.value("license_key", "");
        m_cachedInfo.status = j.value("status", "");
        m_cachedInfo.licenseType = j.value("license_type", "");
        m_cachedInfo.expiresAt = j.value("expires_at", "");
        m_cachedInfo.daysRemaining = j.value("days_remaining", 0);
        m_cachedInfo.valid = j.value("valid", false);
    } catch (const std::exception& e) {
        std::cerr << "[License] Failed to load cached license: " << e.what() << std::endl;
    }
}

void LicenseClient::saveCachedLicense() {
    auto path = PortablePaths::instance().licenseFile();

    try {
        nlohmann::json j;
        j["license_key"] = m_cachedInfo.licenseKey;
        j["status"] = m_cachedInfo.status;
        j["license_type"] = m_cachedInfo.licenseType;
        j["expires_at"] = m_cachedInfo.expiresAt;
        j["days_remaining"] = m_cachedInfo.daysRemaining;
        j["valid"] = m_cachedInfo.valid;

        std::ofstream file(path);
        file << j.dump(2);
    } catch (const std::exception& e) {
        std::cerr << "[License] Failed to save cached license: " << e.what() << std::endl;
    }
}

LicenseInfo LicenseClient::parseLicenseResponse(const std::string& response) {
    LicenseInfo info;

    if (response.empty()) {
        info.status = "offline";
        return info;
    }

    try {
        auto j = nlohmann::json::parse(response);
        info.valid = j.value("success", false);

        if (j.contains("data")) {
            auto& data = j["data"];
            info.status = data.value("status", "unknown");
            info.licenseType = data.value("license_type", "");
            info.expiresAt = data.value("expires_at", "");
            info.daysRemaining = data.value("days_remaining", 0);
            info.activations = data.value("activations", 0);
            info.maxActivations = data.value("max_activations", 1);
        } else if (j.contains("license")) {
            auto& lic = j["license"];
            info.status = lic.value("status", "unknown");
            info.licenseType = lic.value("license_type", "");
            info.expiresAt = lic.value("expires_at", "");
        }
    } catch (const std::exception& e) {
        info.valid = false;
        info.status = "parse_error";
        std::cerr << "[License] Parse error: " << e.what() << std::endl;
    }

    return info;
}

std::string LicenseClient::post(const std::string& endpoint, const nlohmann::json& data) {
    try {
        httplib::Client cli(m_serverUrl);
        cli.set_connection_timeout(10);
        cli.set_read_timeout(15);

        httplib::Headers headers = {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"}
        };

        auto res = cli.Post(endpoint, headers, data.dump(), "application/json");

        if (res && (res->status == 200 || res->status == 201)) {
            return res->body;
        } else if (res) {
            std::cerr << "[License] POST " << endpoint << " = " << res->status << std::endl;
            return res->body;
        }
    } catch (const std::exception& e) {
        std::cerr << "[License] POST " << endpoint << " failed: " << e.what() << std::endl;
    }

    return "";
}

std::string LicenseClient::get(const std::string& endpoint) {
    try {
        httplib::Client cli(m_serverUrl);
        cli.set_connection_timeout(10);
        cli.set_read_timeout(15);

        auto res = cli.Get(endpoint);
        if (res && res->status == 200) {
            return res->body;
        }
    } catch (const std::exception& e) {
        std::cerr << "[License] GET " << endpoint << " failed: " << e.what() << std::endl;
    }

    return "";
}

} // namespace ComfyX
