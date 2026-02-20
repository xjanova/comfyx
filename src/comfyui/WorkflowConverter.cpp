#include "WorkflowConverter.h"
#include <iostream>
#include <regex>

namespace ComfyX {

nlohmann::json WorkflowConverter::uiToApi(const nlohmann::json& uiWorkflow) {
    nlohmann::json api;

    if (!uiWorkflow.contains("nodes") || !uiWorkflow.contains("links")) {
        return api;
    }

    auto& nodes = uiWorkflow["nodes"];
    auto& links = uiWorkflow["links"];

    // Build link lookup: link_id -> {origin_node, origin_slot}
    std::unordered_map<int, std::pair<int, int>> linkMap;
    for (auto& link : links) {
        if (link.is_array() && link.size() >= 5) {
            int linkId = link[0].get<int>();
            int originNode = link[1].get<int>();
            int originSlot = link[2].get<int>();
            linkMap[linkId] = {originNode, originSlot};
        }
    }

    for (auto& node : nodes) {
        if (!node.contains("id") || !node.contains("type")) continue;

        std::string nodeId = std::to_string(node["id"].get<int>());
        nlohmann::json apiNode;
        apiNode["class_type"] = node["type"];

        nlohmann::json inputs;

        // Process linked inputs
        if (node.contains("inputs") && node["inputs"].is_array()) {
            for (auto& input : node["inputs"]) {
                std::string name = input["name"].get<std::string>();
                if (input.contains("link") && !input["link"].is_null()) {
                    int linkId = input["link"].get<int>();
                    auto it = linkMap.find(linkId);
                    if (it != linkMap.end()) {
                        inputs[name] = nlohmann::json::array({
                            std::to_string(it->second.first),
                            it->second.second
                        });
                    }
                }
            }
        }

        // Process widget values
        if (node.contains("widgets_values") && node["widgets_values"].is_array()) {
            auto& widgets = node["widgets_values"];
            // Match widget values to inputs that aren't linked
            if (node.contains("inputs") && node["inputs"].is_array()) {
                int widgetIdx = 0;
                for (auto& input : node["inputs"]) {
                    std::string name = input["name"].get<std::string>();
                    bool isLinked = input.contains("link") && !input["link"].is_null();
                    if (!isLinked && widgetIdx < (int)widgets.size()) {
                        inputs[name] = widgets[widgetIdx];
                    }
                    if (!isLinked) widgetIdx++;
                }
            } else {
                // No input definitions, just use widget values as-is
                for (size_t i = 0; i < widgets.size(); i++) {
                    inputs["param_" + std::to_string(i)] = widgets[i];
                }
            }
        }

        apiNode["inputs"] = inputs;
        api[nodeId] = apiNode;
    }

    return api;
}

nlohmann::json WorkflowConverter::apiToUi(const nlohmann::json& apiWorkflow) {
    nlohmann::json ui;
    ui["version"] = 1;

    nlohmann::json nodes = nlohmann::json::array();
    nlohmann::json links = nlohmann::json::array();

    int linkIdCounter = 1;
    float xPos = 100.0f;
    float yPos = 100.0f;
    int nodesPerRow = 4;
    int nodeCount = 0;

    for (auto& [nodeId, nodeData] : apiWorkflow.items()) {
        nlohmann::json node;
        node["id"] = std::stoi(nodeId);
        node["type"] = nodeData["class_type"];
        node["pos"] = nlohmann::json::array({xPos, yPos});
        node["size"] = nlohmann::json::array({300, 200});
        node["flags"] = nlohmann::json::object();
        node["order"] = nodeCount;
        node["mode"] = 0;

        nlohmann::json nodeInputs = nlohmann::json::array();
        nlohmann::json widgetValues = nlohmann::json::array();

        if (nodeData.contains("inputs")) {
            for (auto& [inputName, inputValue] : nodeData["inputs"].items()) {
                if (inputValue.is_array() && inputValue.size() == 2 &&
                    inputValue[0].is_string()) {
                    // This is a link reference: ["source_node_id", output_index]
                    nlohmann::json inp;
                    inp["name"] = inputName;
                    inp["type"] = "*";
                    inp["link"] = linkIdCounter;

                    int srcNode = std::stoi(inputValue[0].get<std::string>());
                    int srcSlot = inputValue[1].get<int>();

                    links.push_back(nlohmann::json::array({
                        linkIdCounter, srcNode, srcSlot, std::stoi(nodeId), (int)nodeInputs.size(), "*"
                    }));
                    linkIdCounter++;

                    nodeInputs.push_back(inp);
                } else {
                    // Widget value
                    widgetValues.push_back(inputValue);
                }
            }
        }

        node["inputs"] = nodeInputs;
        node["widgets_values"] = widgetValues;
        node["outputs"] = nlohmann::json::array();
        node["properties"] = {{"Node name for S&R", nodeData["class_type"]}};

        nodes.push_back(node);

        nodeCount++;
        xPos += 350.0f;
        if (nodeCount % nodesPerRow == 0) {
            xPos = 100.0f;
            yPos += 250.0f;
        }
    }

    ui["nodes"] = nodes;
    ui["links"] = links;
    ui["groups"] = nlohmann::json::array();
    ui["state"] = {
        {"lastNodeId", nodeCount},
        {"lastLinkId", linkIdCounter - 1}
    };

    return ui;
}

bool WorkflowConverter::validateApiFormat(const nlohmann::json& workflow, std::string& error) {
    if (!workflow.is_object() || workflow.empty()) {
        error = "Workflow must be a non-empty JSON object";
        return false;
    }

    for (auto& [nodeId, nodeData] : workflow.items()) {
        if (!nodeData.contains("class_type")) {
            error = "Node " + nodeId + " missing 'class_type'";
            return false;
        }
        if (!nodeData.contains("inputs")) {
            error = "Node " + nodeId + " missing 'inputs'";
            return false;
        }
    }

    return true;
}

nlohmann::json WorkflowConverter::extractJsonFromText(const std::string& text) {
    // Try to find JSON in code blocks first
    std::regex codeBlockRegex(R"(```(?:json)?\s*\n?([\s\S]*?)\n?```)");
    std::smatch match;

    if (std::regex_search(text, match, codeBlockRegex)) {
        try {
            return nlohmann::json::parse(match[1].str());
        } catch (...) {}
    }

    // Try to find raw JSON object
    size_t start = text.find('{');
    if (start != std::string::npos) {
        int depth = 0;
        for (size_t i = start; i < text.size(); i++) {
            if (text[i] == '{') depth++;
            if (text[i] == '}') depth--;
            if (depth == 0) {
                try {
                    return nlohmann::json::parse(text.substr(start, i - start + 1));
                } catch (...) {}
                break;
            }
        }
    }

    return {};
}

} // namespace ComfyX
