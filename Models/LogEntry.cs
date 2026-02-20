using System;

namespace ComfyX.Models
{
    /// <summary>
    /// Severity levels for application log entries.
    /// </summary>
    public enum LogLevel
    {
        Debug,
        Info,
        Warn,
        Error
    }

    /// <summary>
    /// Represents a single entry in the application activity log.
    /// </summary>
    public class LogEntry
    {
        /// <summary>
        /// Severity level of this log entry.
        /// </summary>
        public LogLevel Level { get; set; }

        /// <summary>
        /// Human-readable log message.
        /// </summary>
        public string Message { get; set; } = string.Empty;

        /// <summary>
        /// UTC timestamp when the log entry was created.
        /// </summary>
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;

        public LogEntry() { }

        public LogEntry(LogLevel level, string message)
        {
            Level = level;
            Message = message;
            Timestamp = DateTime.UtcNow;
        }

        public override string ToString()
        {
            string tag = Level switch
            {
                LogLevel.Debug => "DBG",
                LogLevel.Info  => "INF",
                LogLevel.Warn  => "WRN",
                LogLevel.Error => "ERR",
                _ => "???"
            };
            return $"[{Timestamp:HH:mm:ss}] [{tag}] {Message}";
        }
    }
}
