using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using ComfyX.Helpers;
using ComfyX.Models;
using ComfyX.Services;

namespace ComfyX.Dialogs
{
    public partial class LicenseDialog : Window
    {
        public LicenseDialog()
        {
            InitializeComponent();
            LicenseKeyBox.TextChanged += LicenseKeyBox_TextChanged;
            LoadCurrentState();
        }

        private void LoadCurrentState()
        {
            var guard = LicenseGuard.Instance;

            if (!string.IsNullOrEmpty(guard.SavedLicenseKey))
            {
                LicenseKeyBox.Text = guard.SavedLicenseKey;
            }

            if (guard.IsLicensed && guard.CurrentLicense != null)
            {
                ShowLicenseInfo(guard.CurrentLicense);
            }
        }

        private void LicenseKeyBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            KeyPlaceholder.Visibility = string.IsNullOrEmpty(LicenseKeyBox.Text)
                ? Visibility.Visible
                : Visibility.Collapsed;
        }

        private async void BtnActivate_Click(object sender, RoutedEventArgs e)
        {
            string key = LicenseKeyBox.Text?.Trim();
            if (string.IsNullOrEmpty(key))
            {
                ShowStatus("Please enter a license key.", false);
                return;
            }

            btnActivate.IsEnabled = false;
            btnActivate.Content = "Activating...";
            HideStatus();

            try
            {
                var result = await LicenseGuard.Instance.ActivateAsync(key);
                if (result.IsValid)
                {
                    ShowLicenseInfo(result);
                    Logger.Info("License activated successfully.");
                }
                else
                {
                    ShowStatus($"Activation failed: {result.Status}", false);
                }
            }
            catch (Exception ex)
            {
                ShowStatus($"Error: {ex.Message}", false);
                Logger.Error($"License activation error: {ex.Message}");
            }
            finally
            {
                btnActivate.IsEnabled = true;
                btnActivate.Content = "Activate";
            }
        }

        private async void BtnDeactivate_Click(object sender, RoutedEventArgs e)
        {
            var result = MessageBox.Show(
                "Are you sure you want to deactivate this license?",
                "Deactivate License",
                MessageBoxButton.YesNo,
                MessageBoxImage.Question);

            if (result != MessageBoxResult.Yes) return;

            btnDeactivate.IsEnabled = false;

            try
            {
                await LicenseGuard.Instance.DeactivateAsync();
                InfoPanel.Visibility = Visibility.Collapsed;
                btnDeactivate.Visibility = Visibility.Collapsed;
                LicenseKeyBox.Text = string.Empty;

                StatusIconBorder.BorderBrush = FindResource("TextMutedBrush") as Brush;
                StatusTitle.Text = "Enter your license key";
                StatusIcon.Text = "🔑";

                ShowStatus("License deactivated.", true);
                Logger.Info("License deactivated.");
            }
            catch (Exception ex)
            {
                ShowStatus($"Error: {ex.Message}", false);
            }
            finally
            {
                btnDeactivate.IsEnabled = true;
            }
        }

        private void ShowLicenseInfo(LicenseInfo info)
        {
            InfoPanel.Visibility = Visibility.Visible;
            btnDeactivate.Visibility = Visibility.Visible;

            txtLicenseStatus.Text = info.Status;
            txtLicenseType.Text = info.LicenseType;
            txtLicenseExpires.Text = string.IsNullOrEmpty(info.ExpiresAt) ? "Never" : info.ExpiresAt;
            txtLicenseActivations.Text = $"{info.Activations} / {info.MaxActivations}";

            if (info.IsValid)
            {
                var green = FindResource("SuccessBrush") as Brush;
                StatusIconBorder.BorderBrush = green;
                StatusTitle.Text = "License Active";
                StatusTitle.Foreground = green;
                StatusIcon.Text = "✓";
                txtLicenseStatus.Foreground = green;
                HideStatus();
            }
            else
            {
                var red = FindResource("ErrorBrush") as Brush;
                StatusIconBorder.BorderBrush = red;
                StatusTitle.Text = "License Invalid";
                StatusTitle.Foreground = red;
                StatusIcon.Text = "✗";
                txtLicenseStatus.Foreground = red;
            }
        }

        private void ShowStatus(string message, bool isSuccess)
        {
            txtStatusMessage.Text = message;
            txtStatusMessage.Foreground = isSuccess
                ? FindResource("SuccessBrush") as Brush
                : FindResource("ErrorBrush") as Brush;
            txtStatusMessage.Visibility = Visibility.Visible;
        }

        private void HideStatus()
        {
            txtStatusMessage.Visibility = Visibility.Collapsed;
        }

        private void TitleBar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void BtnClose_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
