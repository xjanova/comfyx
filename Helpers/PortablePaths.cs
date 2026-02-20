using System;
using System.IO;
using System.Reflection;

namespace ComfyX.Helpers
{
    /// <summary>
    /// Provides portable directory paths relative to the executable location.
    /// All paths are resolved at startup so the application can run from any directory.
    /// </summary>
    public static class PortablePaths
    {
        /// <summary>
        /// Root directory containing the executable.
        /// </summary>
        public static string ExeDir { get; private set; } = string.Empty;

        /// <summary>
        /// assets/ directory containing bundled resources (i18n, prompts, fonts, icons).
        /// </summary>
        public static string AssetsDir { get; private set; } = string.Empty;

        /// <summary>
        /// data/ directory for user-writable persistent storage (config, workflows, cache).
        /// </summary>
        public static string DataDir { get; private set; } = string.Empty;

        /// <summary>
        /// runtime/ directory for embedded ComfyUI or other runtime binaries.
        /// </summary>
        public static string RuntimeDir { get; private set; } = string.Empty;

        /// <summary>
        /// assets/fonts/ directory.
        /// </summary>
        public static string FontsDir { get; private set; } = string.Empty;

        /// <summary>
        /// assets/i18n/ directory containing translation JSON files.
        /// </summary>
        public static string I18nDir { get; private set; } = string.Empty;

        /// <summary>
        /// assets/prompts/ directory containing AI prompt templates.
        /// </summary>
        public static string PromptsDir { get; private set; } = string.Empty;

        /// <summary>
        /// Initializes all portable paths based on the executable's location
        /// and ensures required data subdirectories exist.
        /// </summary>
        public static void Initialize()
        {
            string exePath = AppContext.BaseDirectory;
            ExeDir = exePath.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

            AssetsDir = Path.Combine(ExeDir, "assets");
            DataDir = Path.Combine(ExeDir, "data");
            RuntimeDir = Path.Combine(ExeDir, "runtime");
            FontsDir = Path.Combine(AssetsDir, "fonts");
            I18nDir = Path.Combine(AssetsDir, "i18n");
            PromptsDir = Path.Combine(AssetsDir, "prompts");

            // Ensure data subdirectories exist
            EnsureDirectory(DataDir);
            EnsureDirectory(Path.Combine(DataDir, "workflows"));
            EnsureDirectory(Path.Combine(DataDir, "history"));
            EnsureDirectory(Path.Combine(DataDir, "cache"));
            EnsureDirectory(Path.Combine(DataDir, "ai_models"));
        }

        private static void EnsureDirectory(string path)
        {
            if (!Directory.Exists(path))
            {
                Directory.CreateDirectory(path);
            }
        }
    }
}
