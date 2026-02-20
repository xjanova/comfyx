using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;

namespace ComfyX.Pages
{
    /// <summary>
    /// Preview page - displays generated images from ComfyUI workflow execution.
    /// Currently shows a placeholder until an image is loaded.
    /// </summary>
    public partial class PreviewPage : UserControl
    {
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
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Preview image load error: {ex.Message}");
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
        }
    }
}
