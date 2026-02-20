using System;
using System.IO;
using System.Text.Json;
using System.Text.Json.Serialization;
using ComfyX.Helpers;

namespace ComfyX.Models
{
    /// <summary>
    /// Configuration for cloud and local AI providers.
    /// </summary>
    public class AIConfig
    {
        [JsonPropertyName("openaiApiKey")]
        public string OpenAIApiKey { get; set; } = string.Empty;

        [JsonPropertyName("openaiModel")]
        public string OpenAIModel { get; set; } = "gpt-4o";

        [JsonPropertyName("claudeApiKey")]
        public string ClaudeApiKey { get; set; } = string.Empty;

        [JsonPropertyName("claudeModel")]
        public string ClaudeModel { get; set; } = "claude-sonnet-4-20250514";

        [JsonPropertyName("geminiApiKey")]
        public string GeminiApiKey { get; set; } = string.Empty;

        [JsonPropertyName("geminiModel")]
        public string GeminiModel { get; set; } = "gemini-2.0-flash";

        [JsonPropertyName("activeProvider")]
        public string ActiveProvider { get; set; } = "openai";

        [JsonPropertyName("localModelPath")]
        public string LocalModelPath { get; set; } = string.Empty;

        [JsonPropertyName("localGpuLayers")]
        public int LocalGpuLayers { get; set; } = 35;

        [JsonPropertyName("localContextSize")]
        public int LocalContextSize { get; set; } = 4096;
    }

    /// <summary>
    /// Configuration for the ComfyUI backend connection.
    /// </summary>
    public class ComfyUIConfig
    {
        [JsonPropertyName("mode")]
        public string Mode { get; set; } = "external";

        [JsonPropertyName("externalUrl")]
        public string ExternalUrl { get; set; } = "http://127.0.0.1:8188";

        [JsonPropertyName("embeddedPort")]
        public int EmbeddedPort { get; set; } = 8188;

        [JsonPropertyName("autoStart")]
        public bool AutoStart { get; set; } = true;
    }

    /// <summary>
    /// Root application configuration. Singleton with JSON persistence to data/config.json.
    /// Ported from the C++ Config.h / AppConfig struct.
    /// </summary>
    public class AppConfig
    {
        [JsonPropertyName("language")]
        public string Language { get; set; } = "en";

        [JsonPropertyName("uiScale")]
        public double UiScale { get; set; } = 1.0;

        [JsonPropertyName("theme")]
        public string Theme { get; set; } = "modern";

        [JsonPropertyName("showWelcome")]
        public bool ShowWelcome { get; set; } = true;

        [JsonPropertyName("ai")]
        public AIConfig AI { get; set; } = new AIConfig();

        [JsonPropertyName("comfyui")]
        public ComfyUIConfig ComfyUI { get; set; } = new ComfyUIConfig();

        // ── Singleton ──────────────────────────────────────────────────────

        [JsonIgnore]
        public static AppConfig Current { get; private set; } = new AppConfig();

        private static string ConfigPath =>
            Path.Combine(PortablePaths.DataDir, "config.json");

        // ── Persistence ────────────────────────────────────────────────────

        public static void Load()
        {
            try
            {
                if (File.Exists(ConfigPath))
                {
                    string json = File.ReadAllText(ConfigPath);
                    var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
                    var loaded = JsonSerializer.Deserialize<AppConfig>(json, options);
                    if (loaded != null)
                    {
                        Current = loaded;
                        return;
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Config load error: {ex.Message}");
            }
            Current = new AppConfig();
        }

        public static void Save()
        {
            try
            {
                var options = new JsonSerializerOptions { WriteIndented = true };
                string json = JsonSerializer.Serialize(Current, options);
                string dir = Path.GetDirectoryName(ConfigPath);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    Directory.CreateDirectory(dir);
                File.WriteAllText(ConfigPath, json);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Config save error: {ex.Message}");
            }
        }
    }
}
