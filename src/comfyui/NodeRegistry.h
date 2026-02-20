#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace ComfyX {

struct NodeInput {
    std::string name;
    std::string type;
    bool required = true;
    nlohmann::json defaultValue;
    nlohmann::json options; // For combo boxes
};

struct NodeOutput {
    std::string name;
    std::string type;
};

struct NodeDefinition {
    std::string className;
    std::string displayName;
    std::string category;
    std::string description;
    std::vector<NodeInput> inputs;
    std::vector<NodeOutput> outputs;
    bool isOutputNode = false;
};

class NodeRegistry {
public:
    static NodeRegistry& instance();

    bool loadFromComfyUI();
    bool loadFromCache();
    void saveToCache();

    const NodeDefinition* getNode(const std::string& className) const;
    std::vector<std::string> getNodeClasses() const;
    std::vector<std::string> getCategories() const;
    std::vector<std::string> getNodesByCategory(const std::string& category) const;

    bool validateWorkflow(const nlohmann::json& workflow, std::string& error) const;

    size_t nodeCount() const { return m_nodes.size(); }
    bool isLoaded() const { return !m_nodes.empty(); }

    // Generate a summary for AI prompts
    std::string generateNodeSummary(int maxNodes = 100) const;

private:
    NodeRegistry() = default;
    void parseObjectInfo(const nlohmann::json& info);

    std::unordered_map<std::string, NodeDefinition> m_nodes;
};

} // namespace ComfyX
