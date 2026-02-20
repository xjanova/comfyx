using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// Orchestrates AI chat sessions by combining system prompts, conversation history,
    /// and user messages, then dispatching them through <see cref="CloudAIClient"/>.
    /// Singleton instance accessed via <see cref="Instance"/>.
    /// </summary>
    public class AIManager
    {
        // ── Singleton ────────────────────────────────────────────────────────

        public static AIManager Instance { get; } = new AIManager();

        private AIManager() { }

        // ── Properties ───────────────────────────────────────────────────────

        /// <summary>
        /// Indicates whether an AI request is currently in progress.
        /// </summary>
        public bool IsBusy { get; private set; }

        // ── Events ───────────────────────────────────────────────────────────

        /// <summary>
        /// Raised when a new message (user or assistant) is appended to the conversation.
        /// Useful for updating the UI chat panel.
        /// </summary>
        public event Action<ChatMessage> MessageReceived;

        // ── Public Methods ───────────────────────────────────────────────────

        /// <summary>
        /// Sends a user message to the active AI provider and returns the assistant's reply.
        /// </summary>
        /// <param name="userMessage">The user's input text.</param>
        /// <param name="conversationHistory">
        /// Previous messages in the conversation (excluding the new user message).
        /// </param>
        /// <returns>The AI assistant's reply text, or an error message.</returns>
        public async Task<string> SendAsync(string userMessage, List<ChatMessage> conversationHistory)
        {
            if (IsBusy)
                return "[Error] An AI request is already in progress. Please wait.";

            if (string.IsNullOrWhiteSpace(userMessage))
                return "[Error] Message cannot be empty.";

            IsBusy = true;

            try
            {
                var config = AppConfig.Current.AI;

                // Resolve provider, API key, and model from configuration.
                string provider = config.ActiveProvider?.ToLowerInvariant() ?? "openai";
                string apiKey;
                string model;

                switch (provider)
                {
                    case "claude":
                        apiKey = config.ClaudeApiKey;
                        model = config.ClaudeModel;
                        break;
                    case "gemini":
                        apiKey = config.GeminiApiKey;
                        model = config.GeminiModel;
                        break;
                    case "openai":
                    default:
                        apiKey = config.OpenAIApiKey;
                        model = config.OpenAIModel;
                        break;
                }

                // Build the full message list: system prompt + history + new user message.
                string systemPrompt = PromptBuilder.BuildSystemPrompt();
                var messages = new List<ChatMessage>();

                // System prompt as the first message.
                if (!string.IsNullOrWhiteSpace(systemPrompt))
                {
                    messages.Add(new ChatMessage("system", systemPrompt));
                }

                // Append existing conversation history.
                if (conversationHistory != null)
                {
                    messages.AddRange(conversationHistory);
                }

                // Append the new user message.
                var userMsg = new ChatMessage("user", userMessage);
                messages.Add(userMsg);
                MessageReceived?.Invoke(userMsg);

                Logger.Debug($"Sending message to {provider} ({model}), " +
                             $"{messages.Count} messages in context.");

                // Dispatch to the cloud provider.
                string reply = await CloudAIClient.SendMessageAsync(provider, apiKey, model, messages);

                // Wrap the reply as an assistant message and notify listeners.
                var assistantMsg = new ChatMessage("assistant", reply);
                MessageReceived?.Invoke(assistantMsg);

                Logger.Info($"AI response received from {provider} " +
                            $"({reply.Length} chars).");

                return reply;
            }
            catch (Exception ex)
            {
                Logger.Error($"AIManager.SendAsync failed: {ex.Message}");
                return $"[Error] {ex.Message}";
            }
            finally
            {
                IsBusy = false;
            }
        }
    }
}
