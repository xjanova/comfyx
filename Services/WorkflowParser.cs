using System;
using System.Text.Json;
using System.Text.RegularExpressions;
using ComfyX.Helpers;

namespace ComfyX.Services
{
    /// <summary>
    /// Extracts, validates, and formats ComfyUI workflow JSON from AI-generated responses.
    /// Handles common patterns such as fenced code blocks and raw JSON embedded in prose.
    /// </summary>
    public static class WorkflowParser
    {
        /// <summary>
        /// Regex that matches a fenced JSON code block: ```json ... ``` or ``` ... ```.
        /// Uses singleline mode so the dot matches newlines.
        /// </summary>
        private static readonly Regex CodeBlockRegex =
            new Regex(@"```(?:json)?\s*\n?(.*?)```", RegexOptions.Singleline | RegexOptions.Compiled);

        // ── Public Methods ───────────────────────────────────────────────────

        /// <summary>
        /// Extracts a ComfyUI workflow JSON string from an AI response.
        /// Looks for fenced code blocks first, then falls back to the outermost JSON object.
        /// </summary>
        /// <param name="aiResponse">The full text response from the AI provider.</param>
        /// <returns>The extracted JSON string, or null if no JSON was found.</returns>
        public static string ExtractWorkflowJson(string aiResponse)
        {
            if (string.IsNullOrWhiteSpace(aiResponse))
                return null;

            // Strategy 1: Look for a fenced ```json ... ``` code block.
            var match = CodeBlockRegex.Match(aiResponse);
            if (match.Success)
            {
                string candidate = match.Groups[1].Value.Trim();
                if (IsJsonObject(candidate))
                {
                    Logger.Debug("Extracted workflow JSON from fenced code block.");
                    return candidate;
                }
            }

            // Strategy 2: Find the outermost { ... } JSON object in the response.
            string rawJson = ExtractOutermostJsonObject(aiResponse);
            if (rawJson != null)
            {
                Logger.Debug("Extracted workflow JSON from raw JSON object in response.");
                return rawJson;
            }

            Logger.Warn("No workflow JSON found in AI response.");
            return null;
        }

        /// <summary>
        /// Validates that a JSON string represents a ComfyUI API-format workflow.
        /// A valid workflow is a JSON object where at least one value contains a "class_type" property.
        /// </summary>
        /// <param name="workflowJson">The JSON string to validate.</param>
        /// <returns>True if the JSON represents a valid ComfyUI workflow.</returns>
        public static bool ValidateWorkflow(string workflowJson)
        {
            if (string.IsNullOrWhiteSpace(workflowJson))
                return false;

            try
            {
                using var doc = JsonDocument.Parse(workflowJson);
                var root = doc.RootElement;

                // The workflow root must be a JSON object.
                if (root.ValueKind != JsonValueKind.Object)
                    return false;

                // At least one property value must be an object with a "class_type" field.
                foreach (var property in root.EnumerateObject())
                {
                    if (property.Value.ValueKind == JsonValueKind.Object &&
                        property.Value.TryGetProperty("class_type", out _))
                    {
                        return true;
                    }
                }

                Logger.Warn("Workflow JSON parsed but no node entries with 'class_type' found.");
                return false;
            }
            catch (JsonException ex)
            {
                Logger.Warn($"Workflow JSON validation failed: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// Pretty-prints workflow JSON with indentation for readability.
        /// Returns the original string if formatting fails.
        /// </summary>
        /// <param name="workflowJson">The raw JSON string to format.</param>
        /// <returns>The formatted JSON string with indentation.</returns>
        public static string FormatWorkflow(string workflowJson)
        {
            if (string.IsNullOrWhiteSpace(workflowJson))
                return workflowJson;

            try
            {
                using var doc = JsonDocument.Parse(workflowJson);
                var options = new JsonSerializerOptions { WriteIndented = true };
                return JsonSerializer.Serialize(doc.RootElement, options);
            }
            catch (JsonException ex)
            {
                Logger.Warn($"Workflow JSON formatting failed: {ex.Message}");
                return workflowJson;
            }
        }

        // ── Private Helpers ──────────────────────────────────────────────────

        /// <summary>
        /// Tests whether a string starts with '{' and can be parsed as a JSON object.
        /// </summary>
        private static bool IsJsonObject(string text)
        {
            if (string.IsNullOrWhiteSpace(text) || text[0] != '{')
                return false;

            try
            {
                using var doc = JsonDocument.Parse(text);
                return doc.RootElement.ValueKind == JsonValueKind.Object;
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// Finds the outermost balanced { ... } substring in the input text.
        /// Uses brace-depth counting to handle nested objects.
        /// </summary>
        private static string ExtractOutermostJsonObject(string text)
        {
            int startIndex = text.IndexOf('{');
            if (startIndex < 0)
                return null;

            int depth = 0;
            bool inString = false;
            bool escaped = false;

            for (int i = startIndex; i < text.Length; i++)
            {
                char c = text[i];

                if (escaped)
                {
                    escaped = false;
                    continue;
                }

                if (c == '\\' && inString)
                {
                    escaped = true;
                    continue;
                }

                if (c == '"')
                {
                    inString = !inString;
                    continue;
                }

                if (inString)
                    continue;

                if (c == '{')
                    depth++;
                else if (c == '}')
                {
                    depth--;
                    if (depth == 0)
                    {
                        string candidate = text.Substring(startIndex, i - startIndex + 1);
                        if (IsJsonObject(candidate))
                            return candidate;
                        return null;
                    }
                }
            }

            return null;
        }
    }
}
