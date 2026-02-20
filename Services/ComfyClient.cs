using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// HTTP client for the ComfyUI REST API. Provides methods for querying
    /// system status, fetching node definitions, queueing prompts, and
    /// retrieving generated images. Singleton instance.
    /// </summary>
    public class ComfyClient
    {
        // ── Singleton ────────────────────────────────────────────────────
        public static ComfyClient Instance { get; } = new ComfyClient();

        private readonly HttpClient _http;

        private ComfyClient()
        {
            _http = new HttpClient();
            _http.Timeout = TimeSpan.FromSeconds(30);
        }

        // ── Base URL ─────────────────────────────────────────────────────

        /// <summary>
        /// Derives the base URL from the current AppConfig ComfyUI settings.
        /// In "external" mode the configured URL is used directly; in "embedded"
        /// mode it is constructed from the embedded port.
        /// </summary>
        public string BaseUrl
        {
            get
            {
                var cfg = AppConfig.Current.ComfyUI;
                if (string.Equals(cfg.Mode, "external", StringComparison.OrdinalIgnoreCase))
                    return cfg.ExternalUrl.TrimEnd('/');

                return $"http://127.0.0.1:{cfg.EmbeddedPort}";
            }
        }

        // ── API Methods ──────────────────────────────────────────────────

        /// <summary>
        /// Checks whether the ComfyUI server is reachable by requesting /system_stats.
        /// Returns true when the server responds with HTTP 200.
        /// </summary>
        public async Task<bool> CheckConnectionAsync()
        {
            try
            {
                var response = await _http.GetAsync($"{BaseUrl}/system_stats");
                return response.IsSuccessStatusCode;
            }
            catch (Exception ex)
            {
                Logger.Error($"ComfyUI connection check failed: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// Fetches the full node registry from the /object_info endpoint and
        /// parses each entry into a <see cref="NodeDefinition"/>.
        /// </summary>
        public async Task<Dictionary<string, NodeDefinition>> GetObjectInfoAsync()
        {
            var result = new Dictionary<string, NodeDefinition>();

            try
            {
                string json = await _http.GetStringAsync($"{BaseUrl}/object_info");

                using var doc = JsonDocument.Parse(json);
                var root = doc.RootElement;

                foreach (var prop in root.EnumerateObject())
                {
                    string className = prop.Name;
                    var node = ParseNodeDefinition(className, prop.Value);
                    result[className] = node;
                }

                Logger.Info($"Loaded {result.Count} node definitions from ComfyUI.");
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to fetch object_info: {ex.Message}");
            }

            return result;
        }

        /// <summary>
        /// Queues a workflow for execution by POSTing to /prompt.
        /// Returns the prompt_id assigned by ComfyUI, or null on failure.
        /// </summary>
        public async Task<string> QueuePromptAsync(string workflowJson)
        {
            try
            {
                string payload = $"{{\"prompt\":{workflowJson}}}";
                var content = new StringContent(payload, Encoding.UTF8, "application/json");

                var response = await _http.PostAsync($"{BaseUrl}/prompt", content);
                response.EnsureSuccessStatusCode();

                string responseJson = await response.Content.ReadAsStringAsync();
                using var doc = JsonDocument.Parse(responseJson);

                if (doc.RootElement.TryGetProperty("prompt_id", out var promptIdElem))
                {
                    string promptId = promptIdElem.GetString();
                    Logger.Info($"Prompt queued: {promptId}");
                    return promptId;
                }

                Logger.Warn("QueuePrompt response did not contain prompt_id.");
                return null;
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to queue prompt: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Downloads a generated image from the /view endpoint.
        /// Returns the raw image bytes, or null on failure.
        /// </summary>
        public async Task<byte[]> GetImageAsync(string filename, string subfolder, string type)
        {
            try
            {
                string url = $"{BaseUrl}/view?filename={Uri.EscapeDataString(filename)}" +
                             $"&subfolder={Uri.EscapeDataString(subfolder)}" +
                             $"&type={Uri.EscapeDataString(type)}";

                byte[] data = await _http.GetByteArrayAsync(url);
                Logger.Debug($"Downloaded image: {filename} ({data.Length} bytes)");
                return data;
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to download image '{filename}': {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Retrieves the execution history for a given prompt.
        /// Returns the raw JSON string, or null on failure.
        /// </summary>
        public async Task<string> GetHistoryAsync(string promptId)
        {
            try
            {
                string json = await _http.GetStringAsync($"{BaseUrl}/history/{promptId}");
                return json;
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to fetch history for prompt '{promptId}': {ex.Message}");
                return null;
            }
        }

        // ── Parsing helpers ──────────────────────────────────────────────

        /// <summary>
        /// Parses a single node entry from the object_info JSON into a NodeDefinition.
        /// </summary>
        private static NodeDefinition ParseNodeDefinition(string className, JsonElement element)
        {
            var node = new NodeDefinition { ClassName = className };

            if (element.TryGetProperty("display_name", out var displayName))
                node.DisplayName = displayName.GetString() ?? className;
            else
                node.DisplayName = className;

            if (element.TryGetProperty("category", out var category))
                node.Category = category.GetString() ?? string.Empty;

            if (element.TryGetProperty("description", out var description))
                node.Description = description.GetString() ?? string.Empty;

            if (element.TryGetProperty("output_node", out var outputNode))
                node.IsOutputNode = outputNode.GetBoolean();

            // Parse inputs from "input.required" and "input.optional"
            if (element.TryGetProperty("input", out var input))
            {
                ParseInputGroup(input, "required", true, node.Inputs);
                ParseInputGroup(input, "optional", false, node.Inputs);
            }

            // Parse outputs
            if (element.TryGetProperty("output", out var output) && output.ValueKind == JsonValueKind.Array)
            {
                var outputNames = new List<string>();
                if (element.TryGetProperty("output_name", out var outNames) && outNames.ValueKind == JsonValueKind.Array)
                {
                    foreach (var name in outNames.EnumerateArray())
                        outputNames.Add(name.GetString() ?? string.Empty);
                }

                int index = 0;
                foreach (var outType in output.EnumerateArray())
                {
                    string typeName = outType.ValueKind == JsonValueKind.String
                        ? outType.GetString() ?? string.Empty
                        : string.Empty;

                    string outName = index < outputNames.Count ? outputNames[index] : typeName;

                    node.Outputs.Add(new NodeOutput
                    {
                        Name = outName,
                        Type = typeName
                    });
                    index++;
                }
            }

            return node;
        }

        /// <summary>
        /// Parses a group of inputs (required or optional) from the object_info JSON.
        /// </summary>
        private static void ParseInputGroup(JsonElement inputElement, string groupName,
            bool isRequired, List<NodeInput> inputs)
        {
            if (!inputElement.TryGetProperty(groupName, out var group)
                || group.ValueKind != JsonValueKind.Object)
                return;

            foreach (var prop in group.EnumerateObject())
            {
                var nodeInput = new NodeInput
                {
                    Name = prop.Name,
                    Required = isRequired
                };

                // Each input value is typically an array: [type_or_options, config]
                if (prop.Value.ValueKind == JsonValueKind.Array)
                {
                    var arr = prop.Value;
                    if (arr.GetArrayLength() > 0)
                    {
                        var first = arr[0];
                        if (first.ValueKind == JsonValueKind.String)
                        {
                            nodeInput.Type = first.GetString() ?? string.Empty;
                        }
                        else if (first.ValueKind == JsonValueKind.Array)
                        {
                            // Combo/dropdown: first element is an array of options
                            nodeInput.Type = "COMBO";
                            foreach (var option in first.EnumerateArray())
                            {
                                if (option.ValueKind == JsonValueKind.String)
                                    nodeInput.Options.Add(option.GetString() ?? string.Empty);
                            }
                        }
                    }

                    // Check for default value in config object
                    if (arr.GetArrayLength() > 1 && arr[1].ValueKind == JsonValueKind.Object)
                    {
                        var config = arr[1];
                        if (config.TryGetProperty("default", out var defaultVal))
                        {
                            nodeInput.DefaultValue = ExtractJsonValue(defaultVal);
                        }
                    }
                }

                inputs.Add(nodeInput);
            }
        }

        /// <summary>
        /// Extracts a primitive CLR value from a JsonElement.
        /// </summary>
        private static object ExtractJsonValue(JsonElement element)
        {
            switch (element.ValueKind)
            {
                case JsonValueKind.String:  return element.GetString();
                case JsonValueKind.Number:
                    if (element.TryGetInt64(out long l)) return l;
                    return element.GetDouble();
                case JsonValueKind.True:    return true;
                case JsonValueKind.False:   return false;
                default:                    return null;
            }
        }
    }
}
