#include "WorkflowParser.h"
#include "comfyui/WorkflowConverter.h"
#include "comfyui/NodeRegistry.h"

#include <iostream>

namespace ComfyX {

WorkflowParser::ParseResult WorkflowParser::parse(const std::string& aiResponse) {
    ParseResult result;

    // Extract JSON from AI response
    auto json = WorkflowConverter::extractJsonFromText(aiResponse);

    if (json.is_null() || json.empty()) {
        result.success = false;
        result.error = "Could not find valid JSON in AI response";
        return result;
    }

    // Check if it's API format (flat object with numbered keys)
    bool isApiFormat = true;
    for (auto& [key, value] : json.items()) {
        if (!value.contains("class_type")) {
            isApiFormat = false;
            break;
        }
    }

    if (!isApiFormat) {
        // Maybe it's UI format? Try to convert
        if (json.contains("nodes")) {
            json = WorkflowConverter::uiToApi(json);
        } else {
            result.success = false;
            result.error = "JSON is not a valid ComfyUI workflow format";
            return result;
        }
    }

    // Validate structure
    std::string validationError;
    if (!WorkflowConverter::validateApiFormat(json, validationError)) {
        result.success = false;
        result.error = validationError;
        return result;
    }

    // Auto-fix common issues
    json = autoFix(json);

    result.success = true;
    result.workflow = json;
    result.nodeCount = json.size();

    return result;
}

bool WorkflowParser::validate(const nlohmann::json& workflow, std::string& error) {
    return NodeRegistry::instance().validateWorkflow(workflow, error);
}

nlohmann::json WorkflowParser::autoFix(const nlohmann::json& workflow) {
    auto fixed = workflow;

    for (auto& [nodeId, nodeData] : fixed.items()) {
        // Ensure inputs exist
        if (!nodeData.contains("inputs")) {
            nodeData["inputs"] = nlohmann::json::object();
        }

        // Fix common class_type typos
        std::string classType = nodeData["class_type"].get<std::string>();

        // Common fixes
        if (classType == "CheckpointLoader") {
            nodeData["class_type"] = "CheckpointLoaderSimple";
        }
        if (classType == "TextEncode" || classType == "CLIPEncode") {
            nodeData["class_type"] = "CLIPTextEncode";
        }

        // Ensure link references are string format
        for (auto& [inputName, inputValue] : nodeData["inputs"].items()) {
            if (inputValue.is_array() && inputValue.size() == 2) {
                // Make sure the node reference is a string
                if (inputValue[0].is_number()) {
                    inputValue[0] = std::to_string(inputValue[0].get<int>());
                }
            }
        }
    }

    return fixed;
}

} // namespace ComfyX
