using System;
using System.Windows;
using System.Windows.Controls;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Pages
{
    /// <summary>
    /// Settings page - provides configuration UI for ComfyUI, AI Engine, and Appearance.
    /// Loads values from AppConfig.Current on initialization and saves back on Save click.
    /// </summary>
    public partial class SettingsPage : UserControl
    {
        public SettingsPage()
        {
            InitializeComponent();
            Loaded += SettingsPage_Loaded;
        }

        /// <summary>
        /// Populates all controls from the current AppConfig when the page loads.
        /// </summary>
        private void SettingsPage_Loaded(object sender, RoutedEventArgs e)
        {
            LoadSettings();
        }

        /// <summary>
        /// Reads AppConfig.Current and populates all input controls.
        /// </summary>
        private void LoadSettings()
        {
            var cfg = AppConfig.Current;

            // -- ComfyUI tab --
            SetComboByContent(ComfyModeCombo, Capitalize(cfg.ComfyUI.Mode));
            ComfyPortTextBox.Text = cfg.ComfyUI.EmbeddedPort.ToString();
            ComfyAutoStartCheck.IsChecked = cfg.ComfyUI.AutoStart;
            ComfyExternalUrlTextBox.Text = cfg.ComfyUI.ExternalUrl;
            UpdateExternalUrlVisibility();

            // -- AI Engine tab --
            SetComboByContent(AiProviderCombo, MapProviderToDisplay(cfg.AI.ActiveProvider));
            OpenAiKeyBox.Password = cfg.AI.OpenAIApiKey;
            OpenAiModelTextBox.Text = cfg.AI.OpenAIModel;
            ClaudeKeyBox.Password = cfg.AI.ClaudeApiKey;
            ClaudeModelTextBox.Text = cfg.AI.ClaudeModel;
            GeminiKeyBox.Password = cfg.AI.GeminiApiKey;
            GeminiModelTextBox.Text = cfg.AI.GeminiModel;
            LocalModelPathTextBox.Text = cfg.AI.LocalModelPath;
            LocalGpuLayersTextBox.Text = cfg.AI.LocalGpuLayers.ToString();
            LocalContextSizeTextBox.Text = cfg.AI.LocalContextSize.ToString();
            UpdateAiProviderPanels();

            // -- Appearance tab --
            UiScaleSlider.Value = cfg.UiScale;
            UiScaleLabel.Text = $"{cfg.UiScale:F1}x";
            SetComboByTag(LanguageCombo, cfg.Language);
        }

        // ── Event handlers ──────────────────────────────────────────────

        /// <summary>
        /// Shows/hides the External URL field based on the selected ComfyUI mode.
        /// </summary>
        private void ComfyModeCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            UpdateExternalUrlVisibility();
        }

        /// <summary>
        /// Shows/hides AI provider-specific panels based on the selected provider.
        /// </summary>
        private void AiProviderCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            UpdateAiProviderPanels();
        }

        /// <summary>
        /// Updates the scale label text when the slider value changes.
        /// </summary>
        private void UiScaleSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (UiScaleLabel != null)
                UiScaleLabel.Text = $"{UiScaleSlider.Value:F1}x";
        }

        /// <summary>
        /// Saves all settings to AppConfig and persists to disk.
        /// </summary>
        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            var cfg = AppConfig.Current;

            // -- ComfyUI --
            cfg.ComfyUI.Mode = GetComboText(ComfyModeCombo).ToLowerInvariant();
            if (int.TryParse(ComfyPortTextBox.Text, out int port))
                cfg.ComfyUI.EmbeddedPort = port;
            cfg.ComfyUI.AutoStart = ComfyAutoStartCheck.IsChecked == true;
            cfg.ComfyUI.ExternalUrl = ComfyExternalUrlTextBox.Text?.Trim() ?? string.Empty;

            // -- AI Engine --
            cfg.AI.ActiveProvider = MapDisplayToProvider(GetComboText(AiProviderCombo));
            cfg.AI.OpenAIApiKey = OpenAiKeyBox.Password;
            cfg.AI.OpenAIModel = OpenAiModelTextBox.Text?.Trim() ?? string.Empty;
            cfg.AI.ClaudeApiKey = ClaudeKeyBox.Password;
            cfg.AI.ClaudeModel = ClaudeModelTextBox.Text?.Trim() ?? string.Empty;
            cfg.AI.GeminiApiKey = GeminiKeyBox.Password;
            cfg.AI.GeminiModel = GeminiModelTextBox.Text?.Trim() ?? string.Empty;
            cfg.AI.LocalModelPath = LocalModelPathTextBox.Text?.Trim() ?? string.Empty;
            if (int.TryParse(LocalGpuLayersTextBox.Text, out int gpuLayers))
                cfg.AI.LocalGpuLayers = gpuLayers;
            if (int.TryParse(LocalContextSizeTextBox.Text, out int ctxSize))
                cfg.AI.LocalContextSize = ctxSize;

            // -- Appearance --
            cfg.UiScale = UiScaleSlider.Value;
            string lang = GetComboTag(LanguageCombo) ?? "en";
            cfg.Language = lang;

            // Persist and apply language
            AppConfig.Save();
            I18n.SetLanguage(cfg.Language);

            Logger.Info("Settings saved.");
        }

        /// <summary>
        /// Reloads the settings from disk, discarding unsaved changes.
        /// </summary>
        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            LoadSettings();
            Logger.Info("Settings changes discarded.");
        }

        // ── UI visibility helpers ───────────────────────────────────────

        private void UpdateExternalUrlVisibility()
        {
            if (ExternalUrlPanel == null) return;
            string mode = GetComboText(ComfyModeCombo);
            ExternalUrlPanel.Visibility = string.Equals(mode, "External", StringComparison.OrdinalIgnoreCase)
                ? Visibility.Visible
                : Visibility.Collapsed;
        }

        private void UpdateAiProviderPanels()
        {
            if (OpenAiPanel == null) return;

            string provider = GetComboText(AiProviderCombo);
            OpenAiPanel.Visibility = Visibility.Collapsed;
            ClaudePanel.Visibility = Visibility.Collapsed;
            GeminiPanel.Visibility = Visibility.Collapsed;
            LocalAiPanel.Visibility = Visibility.Collapsed;

            switch (provider)
            {
                case "OpenAI": OpenAiPanel.Visibility = Visibility.Visible; break;
                case "Claude": ClaudePanel.Visibility = Visibility.Visible; break;
                case "Gemini": GeminiPanel.Visibility = Visibility.Visible; break;
                case "Local":  LocalAiPanel.Visibility = Visibility.Visible; break;
            }
        }

        // ── ComboBox helpers ────────────────────────────────────────────

        private static string GetComboText(ComboBox combo)
        {
            if (combo.SelectedItem is ComboBoxItem item)
                return item.Content?.ToString() ?? string.Empty;
            return string.Empty;
        }

        private static string GetComboTag(ComboBox combo)
        {
            if (combo.SelectedItem is ComboBoxItem item)
                return item.Tag?.ToString();
            return null;
        }

        private static void SetComboByContent(ComboBox combo, string value)
        {
            foreach (ComboBoxItem item in combo.Items)
            {
                if (string.Equals(item.Content?.ToString(), value, StringComparison.OrdinalIgnoreCase))
                {
                    combo.SelectedItem = item;
                    return;
                }
            }
            if (combo.Items.Count > 0)
                combo.SelectedIndex = 0;
        }

        private static void SetComboByTag(ComboBox combo, string tagValue)
        {
            foreach (ComboBoxItem item in combo.Items)
            {
                if (string.Equals(item.Tag?.ToString(), tagValue, StringComparison.OrdinalIgnoreCase))
                {
                    combo.SelectedItem = item;
                    return;
                }
            }
            if (combo.Items.Count > 0)
                combo.SelectedIndex = 0;
        }

        // ── Provider name mapping ───────────────────────────────────────

        /// <summary>
        /// Maps the config's lowercase provider key to the ComboBox display text.
        /// </summary>
        private static string MapProviderToDisplay(string provider)
        {
            return (provider ?? string.Empty).ToLowerInvariant() switch
            {
                "openai" => "OpenAI",
                "claude" => "Claude",
                "gemini" => "Gemini",
                "local"  => "Local",
                _        => "OpenAI"
            };
        }

        /// <summary>
        /// Maps the ComboBox display text to the config's lowercase provider key.
        /// </summary>
        private static string MapDisplayToProvider(string display)
        {
            return (display ?? string.Empty) switch
            {
                "OpenAI" => "openai",
                "Claude" => "claude",
                "Gemini" => "gemini",
                "Local"  => "local",
                _        => "openai"
            };
        }

        private static string Capitalize(string value)
        {
            if (string.IsNullOrEmpty(value)) return string.Empty;
            return char.ToUpper(value[0]) + value.Substring(1).ToLowerInvariant();
        }
    }
}
