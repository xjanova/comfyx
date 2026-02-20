#pragma once
#include <string>

namespace ComfyX {

class PromptBuilder {
public:
    // Build system prompt for workflow generation
    static std::string buildWorkflowPrompt(const std::string& userRequest);

    // Build system prompt with specific node context
    static std::string buildWorkflowPromptWithNodes(const std::string& userRequest,
                                                      const std::string& nodeInfo);

    // Load custom system prompt from file
    static std::string loadSystemPrompt();

    // Get the default system prompt
    static std::string getDefaultSystemPrompt();
};

} // namespace ComfyX
