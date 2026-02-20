using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using ComfyX.Dialogs;
using ComfyX.Helpers;
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

        // ─── Lazy Page Instances ──────────────────────────────────────
        private UserControl _chatPage;
        private UserControl _nodeGraphPage;
        private UserControl _previewPage;
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
                Page.Chat      => _chatPage      ??= new Pages.ChatPage(),
                Page.NodeGraph => _nodeGraphPage  ??= new Pages.NodeGraphPage(),
                Page.Preview   => _previewPage    ??= new Pages.PreviewPage(),
                Page.Log       => _logPage        ??= new Pages.LogPage(),
                Page.Settings  => _settingsPage   ??= new Pages.SettingsPage(),
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
        private void BtnComfyToggle_Click(object sender, RoutedEventArgs e)
        {
            switch (_comfyState)
            {
                case ComfyState.Stopped:
                case ComfyState.Error:
                    SetComfyState(ComfyState.Starting);
                    try
                    {
                        ComfyProcess.Instance.Start();
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
        }

        // ═══════════════════════════════════════════════════════════════
        // TopBar File Button Handlers (Placeholders)
        // ═══════════════════════════════════════════════════════════════

        private void BtnNew_Click(object sender, RoutedEventArgs e)
        {
            // TODO: Create new workflow
        }

        private void BtnOpen_Click(object sender, RoutedEventArgs e)
        {
            // TODO: Open workflow file dialog
        }

        private void BtnSave_Click(object sender, RoutedEventArgs e)
        {
            // TODO: Save current workflow
        }

        private void BtnLicense_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new LicenseDialog { Owner = this };
            dialog.ShowDialog();
        }
    }
}
