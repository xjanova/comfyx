#pragma once
#include <nlohmann/json.hpp>

namespace ComfyX {

class WorkflowConverter {
public:
    // Convert ComfyUI UI format (litegraph) to API format (flat prompt)
    static nlohmann::json uiToApi(const nlohmann::json& uiWorkflow);

    // Convert API format back to UI format (with auto-layout)
    static nlohmann::json apiToUi(const nlohmann::json& apiWorkflow);

    // Validate API format workflow
    static bool validateApiFormat(const nlohmann::json& workflow, std::string& error);

    // Extract JSON from AI response text (handles markdown code blocks)
    static nlohmann::json extractJsonFromText(const std::string& text);
};

} // namespace ComfyX
