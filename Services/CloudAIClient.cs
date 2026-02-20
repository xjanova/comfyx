using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// Multi-provider AI client that dispatches chat requests to OpenAI, Claude, or Gemini.
    /// Uses a shared static HttpClient for connection pooling and proper resource management.
    /// </summary>
    public static class CloudAIClient
    {
        private static readonly HttpClient _httpClient = new HttpClient
        {
            Timeout = TimeSpan.FromSeconds(120)
        };

        /// <summary>
        /// Sends a chat message to the specified AI provider and returns the assistant's reply.
        /// </summary>
        /// <param name="provider">Provider name: "openai", "claude", or "gemini".</param>
        /// <param name="apiKey">API key for the provider.</param>
        /// <param name="model">Model identifier (e.g. "gpt-4o", "claude-sonnet-4-20250514").</param>
        /// <param name="messages">Conversation history including the latest user message.</param>
        /// <returns>The assistant's reply text, or an error message prefixed with "[Error]".</returns>
        public static async Task<string> SendMessageAsync(
            string provider, string apiKey, string model, List<ChatMessage> messages)
        {
            if (string.IsNullOrWhiteSpace(apiKey))
                return "[Error] API key is not configured for the selected provider.";

            if (messages == null || messages.Count == 0)
                return "[Error] No messages to send.";

            try
            {
                return provider?.ToLowerInvariant() switch
                {
                    "openai" => await SendOpenAIAsync(apiKey, model, messages),
                    "claude" => await SendClaudeAsync(apiKey, model, messages),
                    "gemini" => await SendGeminiAsync(apiKey, model, messages),
                    _ => $"[Error] Unknown AI provider: {provider}"
                };
            }
            catch (TaskCanceledException)
            {
                return "[Error] Request timed out. Please try again.";
            }
            catch (Exception ex)
            {
                Logger.Error($"CloudAIClient error ({provider}): {ex.Message}");
                return $"[Error] {ex.Message}";
            }
        }

        // ── OpenAI ───────────────────────────────────────────────────────────

        private static async Task<string> SendOpenAIAsync(
            string apiKey, string model, List<ChatMessage> messages)
        {
            var url = "https://api.openai.com/v1/chat/completions";

            var payload = new
            {
                model = model,
                messages = messages.Select(m => new { role = m.Role, content = m.Content }).ToArray()
            };

            string json = JsonSerializer.Serialize(payload);

            using var request = new HttpRequestMessage(HttpMethod.Post, url);
            request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", apiKey);
            request.Content = new StringContent(json, Encoding.UTF8, "application/json");

            using var response = await _httpClient.SendAsync(request);
            string body = await response.Content.ReadAsStringAsync();

            if (!response.IsSuccessStatusCode)
            {
                Logger.Error($"OpenAI API error {(int)response.StatusCode}: {body}");
                return $"[Error] OpenAI API returned {(int)response.StatusCode}: {ExtractErrorMessage(body)}";
            }

            using var doc = JsonDocument.Parse(body);
            var root = doc.RootElement;

            if (root.TryGetProperty("choices", out var choices) &&
                choices.GetArrayLength() > 0)
            {
                var firstChoice = choices[0];
                if (firstChoice.TryGetProperty("message", out var message) &&
                    message.TryGetProperty("content", out var content))
                {
                    return content.GetString() ?? string.Empty;
                }
            }

            Logger.Warn("OpenAI response did not contain expected choices/message/content structure.");
            return "[Error] Unexpected response format from OpenAI.";
        }

        // ── Claude (Anthropic) ───────────────────────────────────────────────

        private static async Task<string> SendClaudeAsync(
            string apiKey, string model, List<ChatMessage> messages)
        {
            var url = "https://api.anthropic.com/v1/messages";

            // Claude expects the system prompt as a top-level parameter, not in messages.
            string systemPrompt = null;
            var conversationMessages = new List<object>();

            foreach (var msg in messages)
            {
                if (msg.Role == "system")
                {
                    systemPrompt = msg.Content;
                }
                else
                {
                    conversationMessages.Add(new { role = msg.Role, content = msg.Content });
                }
            }

            // Build the request payload
            var payloadDict = new Dictionary<string, object>
            {
                ["model"] = model,
                ["max_tokens"] = 4096,
                ["messages"] = conversationMessages
            };

            if (!string.IsNullOrEmpty(systemPrompt))
                payloadDict["system"] = systemPrompt;

            string json = JsonSerializer.Serialize(payloadDict);

            using var request = new HttpRequestMessage(HttpMethod.Post, url);
            request.Headers.Add("x-api-key", apiKey);
            request.Headers.Add("anthropic-version", "2023-06-01");
            request.Content = new StringContent(json, Encoding.UTF8, "application/json");

            using var response = await _httpClient.SendAsync(request);
            string body = await response.Content.ReadAsStringAsync();

            if (!response.IsSuccessStatusCode)
            {
                Logger.Error($"Claude API error {(int)response.StatusCode}: {body}");
                return $"[Error] Claude API returned {(int)response.StatusCode}: {ExtractErrorMessage(body)}";
            }

            using var doc = JsonDocument.Parse(body);
            var root = doc.RootElement;

            if (root.TryGetProperty("content", out var contentArray) &&
                contentArray.GetArrayLength() > 0)
            {
                var firstBlock = contentArray[0];
                if (firstBlock.TryGetProperty("text", out var text))
                {
                    return text.GetString() ?? string.Empty;
                }
            }

            Logger.Warn("Claude response did not contain expected content/text structure.");
            return "[Error] Unexpected response format from Claude.";
        }

        // ── Gemini (Google) ──────────────────────────────────────────────────

        private static async Task<string> SendGeminiAsync(
            string apiKey, string model, List<ChatMessage> messages)
        {
            var url = $"https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent?key={apiKey}";

            // Gemini uses "contents" with "parts" format.
            // System messages are sent via systemInstruction at the top level.
            string systemInstruction = null;
            var contents = new List<object>();

            foreach (var msg in messages)
            {
                if (msg.Role == "system")
                {
                    systemInstruction = msg.Content;
                }
                else
                {
                    // Gemini uses "user" and "model" as role names.
                    string geminiRole = msg.Role == "assistant" ? "model" : "user";
                    contents.Add(new
                    {
                        role = geminiRole,
                        parts = new[] { new { text = msg.Content } }
                    });
                }
            }

            var payloadDict = new Dictionary<string, object>
            {
                ["contents"] = contents
            };

            if (!string.IsNullOrEmpty(systemInstruction))
            {
                payloadDict["systemInstruction"] = new
                {
                    parts = new[] { new { text = systemInstruction } }
                };
            }

            string json = JsonSerializer.Serialize(payloadDict);

            using var request = new HttpRequestMessage(HttpMethod.Post, url);
            request.Content = new StringContent(json, Encoding.UTF8, "application/json");

            using var response = await _httpClient.SendAsync(request);
            string body = await response.Content.ReadAsStringAsync();

            if (!response.IsSuccessStatusCode)
            {
                Logger.Error($"Gemini API error {(int)response.StatusCode}: {body}");
                return $"[Error] Gemini API returned {(int)response.StatusCode}: {ExtractErrorMessage(body)}";
            }

            using var doc = JsonDocument.Parse(body);
            var root = doc.RootElement;

            if (root.TryGetProperty("candidates", out var candidates) &&
                candidates.GetArrayLength() > 0)
            {
                var firstCandidate = candidates[0];
                if (firstCandidate.TryGetProperty("content", out var content) &&
                    content.TryGetProperty("parts", out var parts) &&
                    parts.GetArrayLength() > 0)
                {
                    var firstPart = parts[0];
                    if (firstPart.TryGetProperty("text", out var text))
                    {
                        return text.GetString() ?? string.Empty;
                    }
                }
            }

            Logger.Warn("Gemini response did not contain expected candidates/content/parts structure.");
            return "[Error] Unexpected response format from Gemini.";
        }

        // ── Helpers ──────────────────────────────────────────────────────────

        /// <summary>
        /// Attempts to extract a human-readable error message from a provider's JSON error response.
        /// Falls back to returning a truncated version of the raw body.
        /// </summary>
        private static string ExtractErrorMessage(string responseBody)
        {
            try
            {
                using var doc = JsonDocument.Parse(responseBody);
                var root = doc.RootElement;

                // OpenAI format: { "error": { "message": "..." } }
                if (root.TryGetProperty("error", out var errorObj))
                {
                    if (errorObj.ValueKind == JsonValueKind.Object &&
                        errorObj.TryGetProperty("message", out var msg))
                        return msg.GetString() ?? responseBody;

                    if (errorObj.ValueKind == JsonValueKind.String)
                        return errorObj.GetString() ?? responseBody;
                }

                // Anthropic format: { "error": { "message": "..." } } (same structure)
                // Gemini format: { "error": { "message": "..." } }
            }
            catch
            {
                // Not valid JSON; fall through.
            }

            // Return a truncated version of the raw body to avoid flooding the UI.
            return responseBody.Length > 200 ? responseBody.Substring(0, 200) + "..." : responseBody;
        }
    }
}
