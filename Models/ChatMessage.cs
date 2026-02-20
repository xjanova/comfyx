using System;

namespace ComfyX.Models
{
    /// <summary>
    /// Represents a single message in the AI chat conversation.
    /// </summary>
    public class ChatMessage
    {
        /// <summary>
        /// The role of the message author: "user", "assistant", or "system".
        /// </summary>
        public string Role { get; set; } = string.Empty;

        /// <summary>
        /// The text content of the message.
        /// </summary>
        public string Content { get; set; } = string.Empty;

        /// <summary>
        /// UTC timestamp when the message was created.
        /// </summary>
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;

        public ChatMessage() { }

        public ChatMessage(string role, string content)
        {
            Role = role;
            Content = content;
            Timestamp = DateTime.UtcNow;
        }
    }
}
