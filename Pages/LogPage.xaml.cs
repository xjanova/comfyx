using System;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Pages
{
    /// <summary>
    /// Activity log page - displays application log entries with color-coded severity levels.
    /// Supports auto-scrolling and clearing the log.
    /// </summary>
    public partial class LogPage : UserControl
    {
        public LogPage()
        {
            InitializeComponent();

            // Bind the ListBox to Logger.Entries
            LogListBox.ItemsSource = Logger.Entries;

            // Wire up auto-scroll when new entries are added
            Logger.EntryAdded += OnLogEntryAdded;

            // Also listen for collection changes to handle auto-scroll
            ((INotifyCollectionChanged)Logger.Entries).CollectionChanged += OnEntriesCollectionChanged;
        }

        /// <summary>
        /// Called when Logger raises EntryAdded. Scrolls the list to the bottom
        /// if auto-scroll is enabled.
        /// </summary>
        private void OnLogEntryAdded(LogEntry entry)
        {
            if (AutoScrollCheckBox.IsChecked == true)
            {
                ScrollToBottom();
            }
        }

        /// <summary>
        /// Handles collection changes for auto-scroll support.
        /// </summary>
        private void OnEntriesCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Add && AutoScrollCheckBox.IsChecked == true)
            {
                ScrollToBottom();
            }
        }

        /// <summary>
        /// Scrolls the log ListBox to the bottom.
        /// </summary>
        private void ScrollToBottom()
        {
            if (LogListBox.Items.Count > 0)
            {
                // Use Dispatcher to ensure layout has updated before scrolling
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    LogListBox.ScrollIntoView(LogListBox.Items[LogListBox.Items.Count - 1]);
                }), System.Windows.Threading.DispatcherPriority.Background);
            }
        }

        /// <summary>
        /// Clears all log entries.
        /// </summary>
        private void ClearButton_Click(object sender, RoutedEventArgs e)
        {
            Logger.Clear();
        }
    }
}
