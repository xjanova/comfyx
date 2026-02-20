#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace ComfyX {

struct LicenseInfo {
    bool valid = false;
    std::string status;      // "active", "expired", "revoked", "trial"
    std::string licenseType; // "demo", "monthly", "yearly", "lifetime"
    std::string licenseKey;
    std::string expiresAt;
    int daysRemaining = 0;
    int activations = 0;
    int maxActivations = 1;
};

class LicenseClient {
public:
    static LicenseClient& instance();

    void setServerUrl(const std::string& url);

    // Device registration
    bool registerDevice();

    // License operations
    LicenseInfo activate(const std::string& licenseKey);
    LicenseInfo validate();
    bool deactivate();

    // Demo/Trial
    LicenseInfo startDemo();
    LicenseInfo checkDemo();

    // Status
    LicenseInfo getStatus(const std::string& licenseKey);

    // Cached license info
    const LicenseInfo& getCachedInfo() const { return m_cachedInfo; }
    bool hasCachedLicense() const;
    void loadCachedLicense();
    void saveCachedLicense();

private:
    LicenseClient() = default;

    std::string post(const std::string& endpoint, const nlohmann::json& data);
    std::string get(const std::string& endpoint);

    std::string m_serverUrl = "https://xmanstudio.com";
    std::string m_productSlug = "comfyx";
    std::string m_appVersion = "1.0.0";

    LicenseInfo m_cachedInfo;

    LicenseInfo parseLicenseResponse(const std::string& response);
};

} // namespace ComfyX
