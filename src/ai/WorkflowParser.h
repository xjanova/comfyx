#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace ComfyX {

class WorkflowParser {
public:
    struct ParseResult {
        bool success;
        nlohmann::json workflow;
        std::string error;
        int nodeCount = 0;
    };

    // Parse AI response and extract valid ComfyUI workflow
    static ParseResult parse(const std::string& aiResponse);

    // Validate workflow against node registry
    static bool validate(const nlohmann::json& workflow, std::string& error);

    // Fix common AI mistakes in generated workflows
    static nlohmann::json autoFix(const nlohmann::json& workflow);
};

} // namespace ComfyX
