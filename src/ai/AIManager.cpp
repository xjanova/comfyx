#include "AIManager.h"
#include "CloudAI.h"
#include "PromptBuilder.h"
#include "WorkflowParser.h"
#include "app/Config.h"

#include <iostream>

namespace ComfyX {

AIManager& AIManager::instance() {
    static AIManager inst;
    return inst;
}

void AIManager::generateWorkflow(const std::string& userPrompt,
                                  StreamCallback onToken,
                                  CompleteCallback onComplete) {
    if (m_generating) {
        if (onComplete) onComplete("", false);
        return;
    }

    m_generating = true;
    m_cancelled = false;

    // Add user message to history
    {
        std::lock_guard<std::mutex> lock(m_historyMutex);
        m_history.push_back({ChatMessage::User, userPrompt});
    }

    if (m_genThread.joinable()) {
        m_genThread.join();
    }

    m_genThread = std::thread([this, userPrompt, onToken, onComplete]() {
        std::string fullResponse;

        try {
            // Build the prompt
            std::string systemPrompt = PromptBuilder::buildWorkflowPrompt(userPrompt);

            auto& config = Config::instance().get();
            std::string provider = config.ai.activeProvider;

            if (provider == "openai" || provider == "claude" || provider == "gemini") {
                // CloudAI::generate returns the full response and also calls the
                // callback with the content. We use the return value only to avoid
                // double-accumulation.
                fullResponse = CloudAI::generate(systemPrompt, userPrompt, provider,
                    [&](const std::string& token) {
                        if (m_cancelled) return;
                        if (onToken) onToken(token);
                    }
                );
            }
#ifdef COMFYX_LOCAL_AI_ENABLED
            else if (provider == "local") {
                // TODO: Local AI generation via llama.cpp
                fullResponse = "Local AI not yet implemented";
            }
#endif
            else {
                fullResponse = "Unknown AI provider: " + provider;
            }

            if (!m_cancelled) {
                {
                    std::lock_guard<std::mutex> lock(m_historyMutex);
                    m_history.push_back({ChatMessage::Assistant, fullResponse});
                }
                if (onComplete) onComplete(fullResponse, true);
            }

        } catch (const std::exception& e) {
            std::cerr << "[AI] Generation error: " << e.what() << std::endl;
            if (onComplete) onComplete(e.what(), false);
        }

        m_generating = false;
    });
}

void AIManager::chat(const std::string& userMessage,
                      StreamCallback onToken,
                      CompleteCallback onComplete) {
    // Same as generateWorkflow but with chat context
    generateWorkflow(userMessage, onToken, onComplete);
}

void AIManager::cancel() {
    m_cancelled = true;
}

void AIManager::clearHistory() {
    std::lock_guard<std::mutex> lock(m_historyMutex);
    m_history.clear();
}

std::string AIManager::getActiveProvider() const {
    return Config::instance().get().ai.activeProvider;
}

} // namespace ComfyX
