#include "NodeRegistry.h"
#include "ComfyClient.h"
#include "app/PortablePaths.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

namespace ComfyX {

NodeRegistry& NodeRegistry::instance() {
    static NodeRegistry inst;
    return inst;
}

bool NodeRegistry::loadFromComfyUI() {
    auto info = ComfyClient::instance().getObjectInfo();
    if (info.empty()) return false;

    parseObjectInfo(info);
    saveToCache();

    std::cout << "[NodeRegistry] Loaded " << m_nodes.size() << " node types from ComfyUI" << std::endl;
    return true;
}

bool NodeRegistry::loadFromCache() {
    auto cachePath = PortablePaths::instance().cacheDir() / "node_registry.json";
    if (!std::filesystem::exists(cachePath)) return false;

    try {
        std::ifstream file(cachePath);
        nlohmann::json info;
        file >> info;
        parseObjectInfo(info);
        std::cout << "[NodeRegistry] Loaded " << m_nodes.size() << " node types from cache" << std::endl;
        return true;
    } catch (...) {
        return false;
    }
}

void NodeRegistry::saveToCache() {
    auto cachePath = PortablePaths::instance().cacheDir() / "node_registry.json";
    try {
        nlohmann::json cache;
        for (auto& [name, def] : m_nodes) {
            nlohmann::json node;
            node["display_name"] = def.displayName;
            node["category"] = def.category;
            node["description"] = def.description;
            node["output_node"] = def.isOutputNode;

            nlohmann::json inputs;
            for (auto& inp : def.inputs) {
                nlohmann::json i;
                i["name"] = inp.name;
                i["type"] = inp.type;
                i["required"] = inp.required;
                if (!inp.defaultValue.is_null()) i["default"] = inp.defaultValue;
                if (!inp.options.is_null()) i["options"] = inp.options;
                inputs.push_back(i);
            }
            node["inputs"] = inputs;

            nlohmann::json outputs;
            for (auto& out : def.outputs) {
                outputs.push_back({{"name", out.name}, {"type", out.type}});
            }
            node["outputs"] = outputs;

            cache[name] = node;
        }

        std::ofstream file(cachePath);
        file << cache.dump(2);
    } catch (const std::exception& e) {
        std::cerr << "[NodeRegistry] Failed to save cache: " << e.what() << std::endl;
    }
}

void NodeRegistry::parseObjectInfo(const nlohmann::json& info) {
    m_nodes.clear();

    for (auto& [className, nodeInfo] : info.items()) {
        NodeDefinition def;
        def.className = className;
        def.displayName = nodeInfo.value("display_name", className);
        def.category = nodeInfo.value("category", "uncategorized");
        def.description = nodeInfo.value("description", "");
        def.isOutputNode = nodeInfo.value("output_node", false);

        // Parse inputs
        if (nodeInfo.contains("input") && nodeInfo["input"].contains("required")) {
            for (auto& [inputName, inputDef] : nodeInfo["input"]["required"].items()) {
                NodeInput input;
                input.name = inputName;
                input.required = true;

                if (inputDef.is_array() && !inputDef.empty()) {
                    if (inputDef[0].is_string()) {
                        input.type = inputDef[0].get<std::string>();
                    } else if (inputDef[0].is_array()) {
                        input.type = "COMBO";
                        input.options = inputDef[0];
                    }

                    if (inputDef.size() > 1 && inputDef[1].is_object()) {
                        if (inputDef[1].contains("default")) {
                            input.defaultValue = inputDef[1]["default"];
                        }
                    }
                }

                def.inputs.push_back(input);
            }
        }

        if (nodeInfo.contains("input") && nodeInfo["input"].contains("optional")) {
            for (auto& [inputName, inputDef] : nodeInfo["input"]["optional"].items()) {
                NodeInput input;
                input.name = inputName;
                input.required = false;

                if (inputDef.is_array() && !inputDef.empty()) {
                    if (inputDef[0].is_string()) {
                        input.type = inputDef[0].get<std::string>();
                    } else if (inputDef[0].is_array()) {
                        input.type = "COMBO";
                        input.options = inputDef[0];
                    }
                }

                def.inputs.push_back(input);
            }
        }

        // Parse outputs
        if (nodeInfo.contains("output") && nodeInfo["output"].is_array()) {
            auto outputNames = nodeInfo.value("output_name", nlohmann::json::array());
            for (size_t i = 0; i < nodeInfo["output"].size(); i++) {
                NodeOutput output;
                output.type = nodeInfo["output"][i].get<std::string>();
                output.name = (i < outputNames.size()) ? outputNames[i].get<std::string>() : output.type;
                def.outputs.push_back(output);
            }
        }

        m_nodes[className] = def;
    }
}

const NodeDefinition* NodeRegistry::getNode(const std::string& className) const {
    auto it = m_nodes.find(className);
    return it != m_nodes.end() ? &it->second : nullptr;
}

std::vector<std::string> NodeRegistry::getNodeClasses() const {
    std::vector<std::string> classes;
    classes.reserve(m_nodes.size());
    for (auto& [name, _] : m_nodes) {
        classes.push_back(name);
    }
    std::sort(classes.begin(), classes.end());
    return classes;
}

std::vector<std::string> NodeRegistry::getCategories() const {
    std::vector<std::string> categories;
    for (auto& [_, def] : m_nodes) {
        if (std::find(categories.begin(), categories.end(), def.category) == categories.end()) {
            categories.push_back(def.category);
        }
    }
    std::sort(categories.begin(), categories.end());
    return categories;
}

std::vector<std::string> NodeRegistry::getNodesByCategory(const std::string& category) const {
    std::vector<std::string> nodes;
    for (auto& [name, def] : m_nodes) {
        if (def.category == category) {
            nodes.push_back(name);
        }
    }
    std::sort(nodes.begin(), nodes.end());
    return nodes;
}

bool NodeRegistry::validateWorkflow(const nlohmann::json& workflow, std::string& error) const {
    for (auto& [nodeId, nodeData] : workflow.items()) {
        std::string classType = nodeData.value("class_type", "");
        if (classType.empty()) {
            error = "Node " + nodeId + " missing class_type";
            return false;
        }

        auto* def = getNode(classType);
        if (!def) {
            error = "Unknown node type: " + classType;
            return false;
        }
    }
    return true;
}

std::string NodeRegistry::generateNodeSummary(int maxNodes) const {
    std::stringstream ss;
    ss << "Available ComfyUI Nodes (" << m_nodes.size() << " total):\n\n";

    // Group by category
    auto categories = getCategories();
    int count = 0;

    for (auto& cat : categories) {
        if (count >= maxNodes) break;

        ss << "## " << cat << "\n";
        auto nodes = getNodesByCategory(cat);
        for (auto& nodeName : nodes) {
            if (count >= maxNodes) break;

            auto* def = getNode(nodeName);
            if (!def) continue;

            ss << "- **" << nodeName << "**";
            if (!def->description.empty()) {
                ss << ": " << def->description;
            }
            ss << "\n  Inputs: ";
            for (size_t i = 0; i < def->inputs.size(); i++) {
                if (i > 0) ss << ", ";
                ss << def->inputs[i].name << "(" << def->inputs[i].type << ")";
            }
            ss << "\n  Outputs: ";
            for (size_t i = 0; i < def->outputs.size(); i++) {
                if (i > 0) ss << ", ";
                ss << def->outputs[i].type;
            }
            ss << "\n";
            count++;
        }
        ss << "\n";
    }

    return ss.str();
}

} // namespace ComfyX
