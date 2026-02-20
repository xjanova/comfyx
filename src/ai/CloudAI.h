#pragma once
#include <string>
#include <functional>

namespace ComfyX {

class CloudAI {
public:
    using StreamCallback = std::function<void(const std::string& token)>;

    // Generate response using specified provider
    static std::string generate(const std::string& systemPrompt,
                                 const std::string& userPrompt,
                                 const std::string& provider,
                                 StreamCallback onToken = nullptr);

    // Provider-specific implementations
    static std::string generateOpenAI(const std::string& systemPrompt,
                                       const std::string& userPrompt,
                                       StreamCallback onToken = nullptr);

    static std::string generateClaude(const std::string& systemPrompt,
                                       const std::string& userPrompt,
                                       StreamCallback onToken = nullptr);

    static std::string generateGemini(const std::string& systemPrompt,
                                       const std::string& userPrompt,
                                       StreamCallback onToken = nullptr);
};

} // namespace ComfyX
