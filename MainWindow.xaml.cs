using System;
using System.Text.Json;
using IOPath = System.IO.Path;
using IOFile = System.IO.File;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using Microsoft.Win32;
using ComfyX.Dialogs;
using ComfyX.Helpers;
using ComfyX.Models;
using ComfyX.Pages;
using ComfyX.Services;

namespace ComfyX
{
    /// <summary>
    /// Available pages for sidebar navigation.
    /// </summary>
    public enum Page
    {
        Chat,
        NodeGraph,
        Preview,
        Log,
        Settings
    }

    /// <summary>
    /// ComfyUI process states for the toggle button.
    /// </summary>
    public enum ComfyState
    {
        Stopped,
        Starting,
        Running,
        Error
    }

    /// <summary>
    /// Main application window — app shell with TopBar, Sidebar, Content Area, and StatusBar.
    /// Uses code-behind pattern (no MVVM).
    /// </summary>
    public partial class MainWindow : Window
    {
        // ─── State ────────────────────────────────────────────────────
        private Page _activePage = Page.Chat;
        private ComfyState _comfyState = ComfyState.Stopped;
        private string _currentLanguage = "EN";
        private string _lastWorkflowJson;

        // ─── Lazy Page Instances ──────────────────────────────────────
        private ChatPage _chatPage;
        private NodeGraphPage _nodeGraphPage;
        private PreviewPage _previewPage;
        private UserControl _logPage;
        private UserControl _settingsPage;

        // ─── Accent Colors Per Page ───────────────────────────────────
        private static readonly SolidColorBrush CyanAccent   = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#00F5FF"));
        private static readonly SolidColorBrush GreenAccent   = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#00FF88"));
        private static readonly SolidColorBrush PinkAccent    = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF00FF"));
        private static readonly SolidColorBrush OrangeAccent  = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF6B35"));
        private static readonly SolidColorBrush PurpleAccent  = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#BF00FF"));
        private static readonly SolidColorBrush InactiveColor = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#808090"));
        private static readonly SolidColorBrush TransparentBrush = Brushes.Transparent;

        public MainWindow()
        {
            InitializeComponent();
            NavigateTo(Page.Chat);
            Loaded += MainWindow_Loaded;
        }

        private async void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // Initialize license
            await LicenseGuard.Instance.InitializeAsync();

            // Initialize I18n from config
            string lang = AppConfig.Current.Language ?? "en";
            I18n.SetLanguage(lang);
            _currentLanguage = lang.Equals("th", StringComparison.OrdinalIgnoreCase) ? "TH" : "EN";
            txtLanguage.Text = _currentLanguage;

            // Wire ComfyUI process events
            ComfyProcess.Instance.StateChanged += (running) =>
            {
                Dispatcher.Invoke(() =>
                {
                    if (running)
                    {
                        SetComfyState(ComfyState.Running);
                        _ = ComfyWebSocketClient.Instance.ConnectAsync(
                            ComfyClient.Instance.BaseUrl.Replace("http", "ws"));
                    }
                    else
                    {
                        SetComfyState(ComfyState.Stopped);
                        _ = ComfyWebSocketClient.Instance.DisconnectAsync();
                    }
                });
            };

            // Wire WebSocket progress events
            ComfyWebSocketClient.Instance.ProgressUpdated += OnProgressUpdated;
            ComfyWebSocketClient.Instance.ExecutionCompleted += OnExecutionCompleted;

            // Auto-connect to ComfyUI if config says autoStart
            if (AppConfig.Current.ComfyUI.AutoStart)
            {
                try
                {
                    SetComfyState(ComfyState.Starting);
                    txtStatusBar.Text = "Auto-connecting to ComfyUI...";

                    bool alreadyRunning = await ComfyClient.Instance.CheckConnectionAsync();
                    if (alreadyRunning)
                    {
                        SetComfyState(ComfyState.Running);
                        await ComfyWebSocketClient.Instance.ConnectAsync(
                            ComfyClient.Instance.BaseUrl.Replace("http", "ws"));
                    }
                    else if (string.Equals(AppConfig.Current.ComfyUI.Mode, "embedded", StringComparison.OrdinalIgnoreCase))
                    {
                        ComfyProcess.Instance.Start();
                    }
                    else
                    {
                        SetComfyState(ComfyState.Stopped);
                        txtStatusBar.Text = "ComfyUI not detected. Start manually.";
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error($"Auto-connect failed: {ex.Message}");
                    SetComfyState(ComfyState.Error);
                }
            }
        }

        // ═══════════════════════════════════════════════════════════════
        // Navigation
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Swaps the ContentArea content to the requested page and updates sidebar button states.
        /// Page UserControl instances are created lazily on first access.
        /// </summary>
        private void NavigateTo(Page page)
        {
            _activePage = page;

            // Get or create the target page (real pages, not placeholders)
            UserControl targetPage = page switch
            {
                Page.Chat      => _chatPage      ??= new ChatPage(),
                Page.NodeGraph => _nodeGraphPage  ??= new NodeGraphPage(),
                Page.Preview   => _previewPage    ??= new PreviewPage(),
                Page.Log       => _logPage        ??= new LogPage(),
                Page.Settings  => _settingsPage   ??= new SettingsPage(),
                _ => throw new ArgumentOutOfRangeException(nameof(page))
            };

            ContentArea.Content = targetPage;
            UpdateSidebarState();
        }

        /// <summary>
        /// Updates sidebar button foreground colors and left accent bar visibility
        /// to reflect the currently active page.
        /// </summary>
        private void UpdateSidebarState()
        {
            // Reset all buttons to inactive state
            SetSidebarButton(barChat,     btnNavChat,     _activePage == Page.Chat,      CyanAccent);
            SetSidebarButton(barNode,     btnNavNode,     _activePage == Page.NodeGraph,  GreenAccent);
            SetSidebarButton(barPreview,  btnNavPreview,  _activePage == Page.Preview,    PinkAccent);
            SetSidebarButton(barLog,      btnNavLog,      _activePage == Page.Log,        OrangeAccent);
            SetSidebarButton(barSettings, btnNavSettings, _activePage == Page.Settings,   PurpleAccent);
        }

        /// <summary>
        /// Sets a single sidebar button to active or inactive visual state.
        /// Active: colored left bar, accent foreground, tinted background.
        /// Inactive: transparent bar, muted foreground, transparent background.
        /// </summary>
        private void SetSidebarButton(Rectangle bar, Button button, bool isActive, SolidColorBrush accentBrush)
        {
            if (isActive)
            {
                bar.Fill = accentBrush;
                button.Background = new SolidColorBrush(accentBrush.Color) { Opacity = 0.1 };

                // Update the TextBlock inside the button
                if (button.Content is TextBlock tb)
                {
                    tb.Foreground = accentBrush;
                }
            }
            else
            {
                bar.Fill = TransparentBrush;
                button.Background = TransparentBrush;

                if (button.Content is TextBlock tb)
                {
                    tb.Foreground = InactiveColor;
                }
            }
        }

        // ─── Sidebar Click Handlers ───────────────────────────────────

        private void BtnNavChat_Click(object sender, RoutedEventArgs e) => NavigateTo(Page.Chat);
        private void BtnNavNode_Click(object sender, RoutedEventArgs e) => NavigateTo(Page.NodeGraph);
        private void BtnNavPreview_Click(object sender, RoutedEventArgs e) => NavigateTo(Page.Preview);
        private void BtnNavLog_Click(object sender, RoutedEventArgs e) => NavigateTo(Page.Log);
        private void BtnNavSettings_Click(object sender, RoutedEventArgs e) => NavigateTo(Page.Settings);

        // ═══════════════════════════════════════════════════════════════
        // Custom Window Chrome
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Enables dragging the window by holding the TopBar area.
        /// Double-click toggles maximize/restore.
        /// </summary>
        private void TopBar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ClickCount == 2)
            {
                ToggleMaximize();
            }
            else
            {
                DragMove();
            }
        }

        private void BtnMinimize_Click(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        private void BtnMaximize_Click(object sender, RoutedEventArgs e)
        {
            ToggleMaximize();
        }

        private void BtnClose_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void ToggleMaximize()
        {
            if (WindowState == WindowState.Maximized)
            {
                WindowState = WindowState.Normal;
            }
            else
            {
                // Account for taskbar when maximizing a borderless window
                MaxHeight = SystemParameters.MaximizedPrimaryScreenHeight;
                MaxWidth = SystemParameters.MaximizedPrimaryScreenWidth;
                WindowState = WindowState.Maximized;
            }
        }

        // ═══════════════════════════════════════════════════════════════
        // ComfyUI Toggle
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Cycles through ComfyUI states: Stopped -> Starting -> Running -> Stopped.
        /// Error state can be triggered externally; clicking Retry goes back to Starting.
        /// This is a placeholder — actual process management will be added later.
        /// </summary>
        private async void BtnComfyToggle_Click(object sender, RoutedEventArgs e)
        {
            switch (_comfyState)
            {
                case ComfyState.Stopped:
                case ComfyState.Error:
                    SetComfyState(ComfyState.Starting);
                    txtStatusBar.Text = "Connecting to ComfyUI...";
                    try
                    {
                        // Check if ComfyUI is already running externally
                        bool alreadyRunning = await ComfyClient.Instance.CheckConnectionAsync();
                        if (alreadyRunning)
                        {
                            SetComfyState(ComfyState.Running);
                            await ComfyWebSocketClient.Instance.ConnectAsync(
                                ComfyClient.Instance.BaseUrl.Replace("http", "ws"));
                            return;
                        }

                        if (string.Equals(AppConfig.Current.ComfyUI.Mode, "external", StringComparison.OrdinalIgnoreCase))
                        {
                            // External mode: just try to connect, no process start
                            Logger.Warn("ComfyUI not reachable at configured external URL.");
                            SetComfyState(ComfyState.Error);
                            txtStatusBar.Text = "ComfyUI not reachable. Check URL in Settings.";
                        }
                        else
                        {
                            // Embedded mode: start process
                            ComfyProcess.Instance.Start();
                        }
                    }
                    catch (Exception ex)
                    {
                        Logger.Error($"Failed to start ComfyUI: {ex.Message}");
                        SetComfyState(ComfyState.Error);
                    }
                    break;
                case ComfyState.Starting:
                case ComfyState.Running:
                    ComfyProcess.Instance.Stop();
                    await ComfyWebSocketClient.Instance.DisconnectAsync();
                    SetComfyState(ComfyState.Stopped);
                    break;
            }
        }

        /// <summary>
        /// Updates the ComfyUI toggle button text and LED indicator color.
        /// </summary>
        private void SetComfyState(ComfyState state)
        {
            _comfyState = state;

            switch (state)
            {
                case ComfyState.Stopped:
                    txtComfyToggle.Text = "Start ComfyUI";
                    ledComfy.Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#555555"));
                    break;
                case ComfyState.Starting:
                    txtComfyToggle.Text = "Starting...";
                    ledComfy.Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FFD700")); // Yellow
                    break;
                case ComfyState.Running:
                    txtComfyToggle.Text = "Stop ComfyUI";
                    ledComfy.Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#00FF88")); // Green
                    UpdateConnectionStatus(true);
                    break;
                case ComfyState.Error:
                    txtComfyToggle.Text = "Retry";
                    ledComfy.Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF3366")); // Red
                    UpdateConnectionStatus(false);
                    break;
            }
        }

        /// <summary>
        /// Exposes the last workflow JSON for child pages to read.
        /// </summary>
        public string LastWorkflowJson => _lastWorkflowJson;

        /// <summary>
        /// Public navigation entry point for child pages.
        /// </summary>
        public void GoToPage(Page page) => NavigateTo(page);

        /// <summary>
        /// Sets the last workflow JSON and loads it into the NodeGraph page.
        /// Called from ChatPage when AI generates a workflow, or from File Open.
        /// </summary>
        public void SetWorkflowJson(string json, bool navigateToGraph = false)
        {
            _lastWorkflowJson = json;

            // Ensure NodeGraphPage exists and load the workflow
            if (_nodeGraphPage == null)
                _nodeGraphPage = new NodeGraphPage();

            _nodeGraphPage.LoadWorkflow(json);

            if (navigateToGraph)
                NavigateTo(Page.NodeGraph);
        }

        // ═══════════════════════════════════════════════════════════════
        // Connection Status
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Updates the connection status badge in the TopBar and the StatusBar LED.
        /// </summary>
        private void UpdateConnectionStatus(bool connected)
        {
            if (connected)
            {
                var green = (Color)ColorConverter.ConvertFromString("#00FF88");
                ledConnection.Fill = new SolidColorBrush(green);
                txtConnectionStatus.Text = "Connected";
                txtConnectionStatus.Foreground = new SolidColorBrush(green);
                badgeConnection.Background = new SolidColorBrush(green) { Opacity = 0.12 };

                ledStatus.Fill = new SolidColorBrush(green);
                txtStatusBar.Text = "Connected to ComfyUI";
                txtStatusBar.Foreground = new SolidColorBrush(green);
            }
            else
            {
                var red = (Color)ColorConverter.ConvertFromString("#FF3366");
                ledConnection.Fill = new SolidColorBrush(red);
                txtConnectionStatus.Text = "Disconnected";
                txtConnectionStatus.Foreground = new SolidColorBrush(red);
                badgeConnection.Background = new SolidColorBrush(red) { Opacity = 0.12 };

                ledStatus.Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#555555"));
                txtStatusBar.Text = "Disconnected";
                txtStatusBar.Foreground = FindResource("TextSecondaryBrush") as SolidColorBrush;
            }
        }

        // ═══════════════════════════════════════════════════════════════
        // Language Toggle
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Cycles between EN and TH language. Updates the pill button text.
        /// </summary>
        private void BtnLanguage_Click(object sender, RoutedEventArgs e)
        {
            _currentLanguage = _currentLanguage == "EN" ? "TH" : "EN";
            txtLanguage.Text = _currentLanguage;

            string langCode = _currentLanguage == "TH" ? "th" : "en";
            I18n.SetLanguage(langCode);
            AppConfig.Current.Language = langCode;
            AppConfig.Save();
            Logger.Info($"Language changed to: {langCode}");
        }

        // ═══════════════════════════════════════════════════════════════
        // TopBar File Button Handlers (Placeholders)
        // ═══════════════════════════════════════════════════════════════

        private void BtnNew_Click(object sender, RoutedEventArgs e)
        {
            _lastWorkflowJson = null;

            // Reset chat page messages
            _chatPage?.Messages.Clear();

            // Clear preview
            _previewPage?.ClearPreview();

            NavigateTo(Page.Chat);
            txtStatusBar.Text = "New workflow created.";
            Logger.Info("New workflow created.");
        }

        private void BtnOpen_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new OpenFileDialog
            {
                Title = "Open ComfyUI Workflow",
                Filter = "JSON Workflow Files (*.json)|*.json|All Files (*.*)|*.*",
                DefaultExt = ".json"
            };

            if (dlg.ShowDialog(this) == true)
            {
                try
                {
                    string json = IOFile.ReadAllText(dlg.FileName);

                    // Validate it's parseable JSON
                    using (JsonDocument.Parse(json)) { }

                    SetWorkflowJson(json, navigateToGraph: true);
                    txtStatusBar.Text = $"Loaded: {IOPath.GetFileName(dlg.FileName)}";
                    Logger.Info($"Workflow loaded from: {dlg.FileName}");
                }
                catch (Exception ex)
                {
                    Logger.Error($"Failed to open workflow: {ex.Message}");
                    MessageBox.Show(this, $"Failed to open workflow:\n{ex.Message}",
                        "Open Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        private void BtnSave_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(_lastWorkflowJson))
            {
                MessageBox.Show(this, "No workflow to save. Generate or open a workflow first.",
                    "Save", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            var dlg = new SaveFileDialog
            {
                Title = "Save ComfyUI Workflow",
                Filter = "JSON Workflow Files (*.json)|*.json|All Files (*.*)|*.*",
                DefaultExt = ".json",
                FileName = "workflow.json"
            };

            if (dlg.ShowDialog(this) == true)
            {
                try
                {
                    IOFile.WriteAllText(dlg.FileName, _lastWorkflowJson);
                    txtStatusBar.Text = $"Saved: {IOPath.GetFileName(dlg.FileName)}";
                    Logger.Info($"Workflow saved to: {dlg.FileName}");
                }
                catch (Exception ex)
                {
                    Logger.Error($"Failed to save workflow: {ex.Message}");
                    MessageBox.Show(this, $"Failed to save workflow:\n{ex.Message}",
                        "Save Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        private void BtnLicense_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new LicenseDialog { Owner = this };
            dialog.ShowDialog();
        }

        // ═══════════════════════════════════════════════════════════════
        // WebSocket Event Handlers
        // ═══════════════════════════════════════════════════════════════

        private void OnProgressUpdated(ComfyProgress progress)
        {
            txtStatusBar.Text = $"Generating... {progress.Percent:F0}% ({progress.Value}/{progress.Max})";
        }

        private async void OnExecutionCompleted(string promptId)
        {
            try
            {
                Logger.Info($"Execution completed: {promptId}");
                txtStatusBar.Text = "Execution completed! Fetching images...";

                // Wait a moment for ComfyUI to save the image
                await Task.Delay(500);

                string historyJson = await ComfyClient.Instance.GetHistoryAsync(promptId);
                if (historyJson == null) return;

                using var doc = JsonDocument.Parse(historyJson);
                var root = doc.RootElement;

                // Parse: { "promptId": { "outputs": { "nodeId": { "images": [{ "filename": "...", "subfolder": "...", "type": "output" }] } } } }
                foreach (var promptEntry in root.EnumerateObject())
                {
                    if (!promptEntry.Value.TryGetProperty("outputs", out var outputs)) continue;

                    foreach (var nodeOutput in outputs.EnumerateObject())
                    {
                        if (!nodeOutput.Value.TryGetProperty("images", out var images)) continue;

                        foreach (var img in images.EnumerateArray())
                        {
                            string filename = img.GetProperty("filename").GetString();
                            string subfolder = img.TryGetProperty("subfolder", out var sf) ? sf.GetString() ?? "" : "";
                            string type = img.TryGetProperty("type", out var t) ? t.GetString() ?? "output" : "output";

                            byte[] imageData = await ComfyClient.Instance.GetImageAsync(filename, subfolder, type);
                            if (imageData != null && imageData.Length > 0)
                            {
                                // Save to temp and show in preview
                                string tempPath = IOPath.Combine(IOPath.GetTempPath(), $"comfyx_{filename}");
                                IOFile.WriteAllBytes(tempPath, imageData);

                                if (_previewPage == null)
                                    _previewPage = new PreviewPage();
                                _previewPage.ShowImage(tempPath);
                                NavigateTo(Page.Preview);
                                txtStatusBar.Text = $"Image ready: {filename}";
                                Logger.Info($"Preview image loaded: {filename}");
                                return; // Show first image
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to fetch execution results: {ex.Message}");
            }
        }
    }
}
