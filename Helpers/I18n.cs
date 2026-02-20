using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace ComfyX.Helpers
{
    /// <summary>
    /// Internationalization helper. Loads English as the base layer,
    /// then overlays the target language on top. Keys not found in
    /// the target language fall back to English.
    /// </summary>
    public static class I18n
    {
        private static Dictionary<string, string> _strings = new Dictionary<string, string>();
        private static string _currentLanguage = "en";

        public static string CurrentLanguage => _currentLanguage;

        /// <summary>
        /// Initializes with the given language. Loads English first, then overlays target.
        /// Must be called after PortablePaths.Initialize().
        /// </summary>
        public static void Initialize(string language)
        {
            _currentLanguage = language ?? "en";
            _strings = new Dictionary<string, string>();

            // Always load English as the base/fallback layer
            MergeLanguageFile("en");

            // Overlay target language on top if it differs from English
            if (!string.Equals(_currentLanguage, "en", StringComparison.OrdinalIgnoreCase))
                MergeLanguageFile(_currentLanguage);
        }

        /// <summary>
        /// Switches to a different language at runtime.
        /// </summary>
        public static void SetLanguage(string language)
        {
            Initialize(language);
        }

        /// <summary>
        /// Translates a key to its localized string. Returns "[???]" if not found.
        /// </summary>
        public static string T(string key)
        {
            if (string.IsNullOrEmpty(key))
                return "[???]";

            return _strings.TryGetValue(key, out string value) ? value : "[???]";
        }

        /// <summary>
        /// Loads a JSON file and merges its entries into the current dictionary.
        /// Later values overwrite earlier ones for the same key.
        /// </summary>
        private static void MergeLanguageFile(string languageCode)
        {
            try
            {
                string filePath = Path.Combine(PortablePaths.I18nDir, $"{languageCode}.json");
                if (!File.Exists(filePath))
                    return;

                string json = File.ReadAllText(filePath);
                var loaded = JsonSerializer.Deserialize<Dictionary<string, string>>(json);
                if (loaded == null)
                    return;

                foreach (var pair in loaded)
                    _strings[pair.Key] = pair.Value;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"I18n load error for '{languageCode}': {ex.Message}");
            }
        }
    }
}
