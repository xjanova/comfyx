using System;
using System.Windows;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX
{
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            // Initialize portable paths
            PortablePaths.Initialize();

            // Load configuration
            AppConfig.Load();

            // Initialize internationalization
            I18n.Initialize(AppConfig.Current.Language);

            // Initialize logger
            Logger.Info("ComfyX starting...");
        }
    }
}
