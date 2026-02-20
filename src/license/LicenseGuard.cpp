#include "LicenseGuard.h"
#include "LicenseClient.h"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace ComfyX {

LicenseGuard& LicenseGuard::instance() {
    static LicenseGuard inst;
    return inst;
}

bool LicenseGuard::isFeatureAvailable(Feature feature) const {
    auto& info = LicenseClient::instance().getCachedInfo();

    if (!info.valid) return false;

    bool isPaid = (info.licenseType == "monthly" || info.licenseType == "yearly" || info.licenseType == "lifetime");
    bool isDemo = (info.licenseType == "demo");

    switch (feature) {
        case Feature::CloudAI:
            return isPaid || isDemo; // Demo has limited calls
        case Feature::LocalAI:
            return isPaid;
        case Feature::UnlimitedWorkflows:
            return isPaid;
        case Feature::CustomThemes:
            return isPaid;
        case Feature::ExportWorkflow:
            return isPaid;
        case Feature::WorkflowHistory:
            return true; // Always available, but limited for demo
    }

    return false;
}

bool LicenseGuard::canGenerateWorkflow() const {
    auto& info = LicenseClient::instance().getCachedInfo();

    if (!info.valid) return false;

    bool isPaid = (info.licenseType == "monthly" || info.licenseType == "yearly" || info.licenseType == "lifetime");

    if (isPaid) return true;

    // Demo: 3 calls per day
    return m_dailyCount < getDailyLimit();
}

void LicenseGuard::recordWorkflowGeneration() {
    resetDailyCountIfNeeded();
    m_dailyCount++;
}

bool LicenseGuard::isActivated() const {
    auto& info = LicenseClient::instance().getCachedInfo();
    return info.valid && info.status == "active";
}

bool LicenseGuard::isTrial() const {
    auto& info = LicenseClient::instance().getCachedInfo();
    return info.licenseType == "demo";
}

bool LicenseGuard::isExpired() const {
    auto& info = LicenseClient::instance().getCachedInfo();
    return info.status == "expired";
}

std::string LicenseGuard::getStatusText() const {
    auto& info = LicenseClient::instance().getCachedInfo();

    if (!info.valid && info.status.empty()) {
        return "Not activated";
    }

    if (info.licenseType == "demo") {
        return "Trial (" + std::to_string(info.daysRemaining) + " days remaining)";
    }

    if (info.status == "active") {
        if (info.licenseType == "lifetime") {
            return "Lifetime License (Active)";
        }
        return info.licenseType + " License (" + std::to_string(info.daysRemaining) + " days remaining)";
    }

    if (info.status == "expired") {
        return "License Expired";
    }

    return info.status;
}

void LicenseGuard::refresh() {
    LicenseClient::instance().validate();
}

int LicenseGuard::getDailyLimit() const {
    auto& info = LicenseClient::instance().getCachedInfo();

    if (info.licenseType == "monthly" || info.licenseType == "yearly" || info.licenseType == "lifetime") {
        return 99999; // Effectively unlimited
    }

    return 3; // Demo limit
}

void LicenseGuard::resetDailyCountIfNeeded() {
    std::string today = getCurrentDate();
    if (m_lastDate != today) {
        m_dailyCount = 0;
        m_lastDate = today;
    }
}

std::string LicenseGuard::getCurrentDate() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = {};
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");
    return ss.str();
}

} // namespace ComfyX
