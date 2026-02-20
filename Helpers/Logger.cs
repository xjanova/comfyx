using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Threading;
using ComfyX.Models;

namespace ComfyX.Helpers
{
    /// <summary>
    /// Application-wide logger that stores entries in an ObservableCollection
    /// suitable for binding to WPF list controls. Thread-safe: all mutations
    /// are marshalled to the UI dispatcher so the collection can be bound directly.
    /// Maximum 1000 entries are retained; oldest entries are trimmed when exceeded.
    /// </summary>
    public static class Logger
    {
        /// <summary>
        /// Maximum number of log entries to retain. When exceeded the oldest entries are trimmed.
        /// </summary>
        private const int MaxEntries = 1000;

        /// <summary>
        /// Observable collection of log entries. Bind this to an ItemsControl or ListBox.
        /// </summary>
        public static ObservableCollection<LogEntry> Entries { get; } = new ObservableCollection<LogEntry>();

        /// <summary>
        /// Raised whenever a new log entry is added. Useful for auto-scrolling or external sinks.
        /// </summary>
        public static event Action<LogEntry> EntryAdded;

        /// <summary>
        /// Logs a debug-level message.
        /// </summary>
        public static void Debug(string message)
        {
            Add(new LogEntry(LogLevel.Debug, message));
        }

        /// <summary>
        /// Logs an informational message.
        /// </summary>
        public static void Info(string message)
        {
            Add(new LogEntry(LogLevel.Info, message));
        }

        /// <summary>
        /// Logs a warning message.
        /// </summary>
        public static void Warn(string message)
        {
            Add(new LogEntry(LogLevel.Warn, message));
        }

        /// <summary>
        /// Logs an error message.
        /// </summary>
        public static void Error(string message)
        {
            Add(new LogEntry(LogLevel.Error, message));
        }

        /// <summary>
        /// Adds a log entry to the collection, ensuring the operation
        /// runs on the UI thread and enforcing the maximum entry limit.
        /// </summary>
        private static void Add(LogEntry entry)
        {
            // If there is no WPF application running (e.g. unit tests, design time),
            // fall back to a direct add.
            Dispatcher dispatcher = Application.Current?.Dispatcher;

            if (dispatcher == null || dispatcher.CheckAccess())
            {
                AddEntry(entry);
            }
            else
            {
                dispatcher.Invoke(() => AddEntry(entry));
            }
        }

        /// <summary>
        /// Performs the actual insertion and trim. Must be called on the UI thread
        /// (or when no dispatcher is available).
        /// </summary>
        private static void AddEntry(LogEntry entry)
        {
            Entries.Add(entry);

            // Trim oldest entries when we exceed the cap
            while (Entries.Count > MaxEntries)
            {
                Entries.RemoveAt(0);
            }

            EntryAdded?.Invoke(entry);
        }

        /// <summary>
        /// Clears all log entries. Safe to call from any thread.
        /// </summary>
        public static void Clear()
        {
            Dispatcher dispatcher = Application.Current?.Dispatcher;

            if (dispatcher == null || dispatcher.CheckAccess())
            {
                Entries.Clear();
            }
            else
            {
                dispatcher.Invoke(() => Entries.Clear());
            }
        }
    }
}
