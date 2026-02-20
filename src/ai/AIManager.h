#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

namespace ComfyX {

struct ChatMessage {
    enum Role { User, Assistant, System };
    Role role;
    std::string content;
};

class AIManager {
public:
    static AIManager& instance();

    using StreamCallback = std::function<void(const std::string& token)>;
    using CompleteCallback = std::function<void(const std::string& fullResponse, bool success)>;

    // Generate workflow from natural language prompt
    void generateWorkflow(const std::string& userPrompt,
                          StreamCallback onToken = nullptr,
                          CompleteCallback onComplete = nullptr);

    // Chat with context
    void chat(const std::string& userMessage,
              StreamCallback onToken = nullptr,
              CompleteCallback onComplete = nullptr);

    // Cancel ongoing generation
    void cancel();

    bool isGenerating() const { return m_generating; }

    std::vector<ChatMessage> getHistory() const {
        std::lock_guard<std::mutex> lock(m_historyMutex);
        return m_history;
    }

    void clearHistory();

    // Active provider
    std::string getActiveProvider() const;

private:
    AIManager() = default;

    mutable std::mutex m_historyMutex;
    std::vector<ChatMessage> m_history;
    std::atomic<bool> m_generating{false};
    std::atomic<bool> m_cancelled{false};
    std::thread m_genThread;
};

} // namespace ComfyX
