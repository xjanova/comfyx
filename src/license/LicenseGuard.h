#pragma once
#include <string>

namespace ComfyX {

class LicenseGuard {
public:
    static LicenseGuard& instance();

    enum class Feature {
        CloudAI,
        LocalAI,
        UnlimitedWorkflows,
        CustomThemes,
        ExportWorkflow,
        WorkflowHistory
    };

    // Check if a feature is available
    bool isFeatureAvailable(Feature feature) const;

    // Check usage limits
    bool canGenerateWorkflow() const;
    void recordWorkflowGeneration();

    // License status
    bool isActivated() const;
    bool isTrial() const;
    bool isExpired() const;
    std::string getStatusText() const;

    // Refresh from server
    void refresh();

    // Daily usage counter
    int getDailyUsageCount() const { return m_dailyCount; }
    int getDailyLimit() const;

private:
    LicenseGuard() = default;

    int m_dailyCount = 0;
    std::string m_lastDate;

    void resetDailyCountIfNeeded();
    std::string getCurrentDate() const;
};

} // namespace ComfyX
