using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Microsoft.Win32;

namespace ComfyX.Pages
{
    /// <summary>
    /// Preview page - displays generated images from ComfyUI workflow execution.
    /// Currently shows a placeholder until an image is loaded.
    /// </summary>
    public partial class PreviewPage : UserControl
    {
        private bool _isFitMode = true;

        public PreviewPage()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Loads and displays an image from the given file path.
        /// </summary>
        public void ShowImage(string imagePath)
        {
            try
            {
                var bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.CacheOption = BitmapCacheOption.OnLoad;
                bitmap.UriSource = new Uri(imagePath, UriKind.Absolute);
                bitmap.EndInit();
                bitmap.Freeze();

                PreviewImage.Source = bitmap;
                PlaceholderPanel.Visibility = Visibility.Collapsed;
                ImageContainer.Visibility = Visibility.Visible;

                UpdateStatusBar(bitmap);
                Helpers.Logger.Info($"Preview: {bitmap.PixelWidth}x{bitmap.PixelHeight}");
            }
            catch (Exception ex)
            {
                Helpers.Logger.Error($"Preview image load error: {ex.Message}");
            }
        }

        /// <summary>
        /// Clears the current preview and shows the placeholder again.
        /// </summary>
        public void ClearPreview()
        {
            PreviewImage.Source = null;
            ImageContainer.Visibility = Visibility.Collapsed;
            PlaceholderPanel.Visibility = Visibility.Visible;
            UpdateStatusBar();
        }

        /// <summary>
        /// Loads and displays an image from raw byte data using a MemoryStream.
        /// </summary>
        public void ShowImageFromBytes(byte[] imageData)
        {
            try
            {
                var bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.CacheOption = BitmapCacheOption.OnLoad;
                bitmap.StreamSource = new MemoryStream(imageData);
                bitmap.EndInit();
                bitmap.Freeze();

                PreviewImage.Source = bitmap;
                PlaceholderPanel.Visibility = Visibility.Collapsed;
                ImageContainer.Visibility = Visibility.Visible;

                UpdateStatusBar(bitmap);
                Helpers.Logger.Info($"Preview: {bitmap.PixelWidth}x{bitmap.PixelHeight}");
            }
            catch (Exception ex)
            {
                Helpers.Logger.Error($"Preview image load error: {ex.Message}");
            }
        }

        /// <summary>
        /// Opens a Save File dialog to save the current preview image to disk as PNG.
        /// </summary>
        public void SaveImage()
        {
            if (PreviewImage.Source is not BitmapSource bitmapSource)
            {
                Helpers.Logger.Info("No image to save.");
                return;
            }

            var dialog = new SaveFileDialog
            {
                Filter = "PNG Image|*.png|JPEG Image|*.jpg|BMP Image|*.bmp",
                DefaultExt = ".png",
                FileName = $"ComfyX_Preview_{DateTime.Now:yyyyMMdd_HHmmss}"
            };

            if (dialog.ShowDialog() == true)
            {
                try
                {
                    BitmapEncoder encoder = Path.GetExtension(dialog.FileName).ToLowerInvariant() switch
                    {
                        ".jpg" or ".jpeg" => new JpegBitmapEncoder { QualityLevel = 95 },
                        ".bmp" => new BmpBitmapEncoder(),
                        _ => new PngBitmapEncoder()
                    };

                    encoder.Frames.Add(BitmapFrame.Create(bitmapSource));

                    using var stream = new FileStream(dialog.FileName, FileMode.Create);
                    encoder.Save(stream);

                    Helpers.Logger.Info($"Image saved to: {dialog.FileName}");
                }
                catch (Exception ex)
                {
                    Helpers.Logger.Error($"Failed to save image: {ex.Message}");
                }
            }
        }

        /// <summary>
        /// Toggles between Fit (Uniform) and Fill (UniformToFill) stretch modes for the preview image.
        /// </summary>
        public void ToggleFitFill()
        {
            _isFitMode = !_isFitMode;
            PreviewImage.Stretch = _isFitMode ? Stretch.Uniform : Stretch.UniformToFill;
            FitFillButton.Content = _isFitMode ? "Fit" : "Fill";
        }

        /// <summary>
        /// Updates the status bar with image dimension information.
        /// </summary>
        private void UpdateStatusBar(BitmapImage bitmap = null)
        {
            if (bitmap != null)
            {
                StatusText.Text = $"{bitmap.PixelWidth} x {bitmap.PixelHeight} px";
            }
            else
            {
                StatusText.Text = "No image loaded";
            }
        }

        // --- Toolbar event handlers ---

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            SaveImage();
        }

        private void ClearButton_Click(object sender, RoutedEventArgs e)
        {
            ClearPreview();
        }

        private void FitFillButton_Click(object sender, RoutedEventArgs e)
        {
            ToggleFitFill();
        }
    }
}
