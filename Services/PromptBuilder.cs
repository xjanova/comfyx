using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// Builds system prompts and instruction wrappers for AI-assisted workflow generation.
    /// Prompts can be loaded from an external file or fall back to a built-in default.
    /// </summary>
    public static class PromptBuilder
    {
        /// <summary>
        /// Default system prompt used when no external prompt file is found.
        /// </summary>
        private const string DefaultSystemPrompt =
            "You are ComfyX AI assistant. You help users create ComfyUI workflows. " +
            "When the user describes an image generation task, create a ComfyUI workflow in JSON API format. " +
            "Each node in the workflow should have a unique string ID as the key, and contain " +
            "\"class_type\", \"inputs\", and optionally \"_meta\" with a \"title\" field. " +
            "Connect nodes by referencing other node IDs in the format [\"node_id\", output_index]. " +
            "Always use standard ComfyUI node class names. " +
            "If the user asks a general question, answer helpfully without generating a workflow.";

        /// <summary>
        /// Filename of the external system prompt template.
        /// </summary>
        private const string SystemPromptFileName = "system_prompt.txt";

        // ── Public Methods ───────────────────────────────────────────────────

        /// <summary>
        /// Builds the full system prompt for an AI chat session.
        /// </summary>
        /// <param name="availableNodes">
        /// Optional dictionary of node definitions registered with ComfyUI.
        /// When provided, a summary of available nodes grouped by category is appended.
        /// </param>
        /// <returns>The complete system prompt string.</returns>
        public static string BuildSystemPrompt(Dictionary<string, NodeDefinition> availableNodes = null)
        {
            string basePrompt = LoadBasePrompt();
            var sb = new StringBuilder(basePrompt);

            // Append available node information when provided.
            if (availableNodes != null && availableNodes.Count > 0)
            {
                sb.AppendLine();
                sb.AppendLine();
                sb.AppendLine("## Available ComfyUI Nodes");
                sb.AppendLine();
                sb.AppendLine("Below are the node class names available on the connected ComfyUI server, " +
                              "grouped by category. Use only these class names when building workflows.");
                sb.AppendLine();

                // Group nodes by category, sorted alphabetically.
                var grouped = availableNodes.Values
                    .GroupBy(n => string.IsNullOrWhiteSpace(n.Category) ? "Uncategorized" : n.Category)
                    .OrderBy(g => g.Key);

                foreach (var group in grouped)
                {
                    sb.AppendLine($"### {group.Key}");

                    foreach (var node in group.OrderBy(n => n.ClassName))
                    {
                        string display = !string.IsNullOrWhiteSpace(node.DisplayName)
                            ? $" ({node.DisplayName})"
                            : string.Empty;
                        sb.AppendLine($"- {node.ClassName}{display}");
                    }

                    sb.AppendLine();
                }
            }

            return sb.ToString();
        }

        /// <summary>
        /// Wraps a user request with instructions to produce a valid ComfyUI API-format JSON workflow.
        /// </summary>
        /// <param name="userRequest">The user's natural-language description of the desired workflow.</param>
        /// <returns>A prompt string that instructs the AI to output valid workflow JSON.</returns>
        public static string BuildWorkflowPrompt(string userRequest)
        {
            var sb = new StringBuilder();

            sb.AppendLine("Please generate a ComfyUI workflow in API format JSON based on the following request.");
            sb.AppendLine();
            sb.AppendLine("Requirements:");
            sb.AppendLine("1. Output ONLY valid JSON in a ```json code block.");
            sb.AppendLine("2. Each node must have a unique string ID as the key (e.g. \"1\", \"2\", \"3\").");
            sb.AppendLine("3. Each node must contain \"class_type\" and \"inputs\".");
            sb.AppendLine("4. Node connections use the format [\"source_node_id\", output_index].");
            sb.AppendLine("5. Include a \"_meta\" object with a \"title\" field for each node.");
            sb.AppendLine("6. Use only standard ComfyUI node class names.");
            sb.AppendLine();
            sb.AppendLine("User request:");
            sb.AppendLine(userRequest);

            return sb.ToString();
        }

        // ── Private Helpers ──────────────────────────────────────────────────

        /// <summary>
        /// Loads the base system prompt from the external file, falling back to the built-in default.
        /// </summary>
        private static string LoadBasePrompt()
        {
            try
            {
                string promptPath = Path.Combine(PortablePaths.PromptsDir, SystemPromptFileName);

                if (File.Exists(promptPath))
                {
                    string content = File.ReadAllText(promptPath).Trim();

                    if (!string.IsNullOrEmpty(content))
                    {
                        Logger.Debug($"Loaded system prompt from {promptPath}");
                        return content;
                    }
                }
            }
            catch (Exception ex)
            {
                Logger.Warn($"Failed to load system prompt file: {ex.Message}");
            }

            Logger.Debug("Using built-in default system prompt.");
            return DefaultSystemPrompt;
        }
    }
}
