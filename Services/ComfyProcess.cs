using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// Manages an embedded ComfyUI Python process. Handles starting, stopping,
    /// and monitoring the process. Redirects stdout/stderr to the application
    /// logger and fires events for UI consumption.
    /// </summary>
    public class ComfyProcess : IDisposable
    {
        // ── Singleton ────────────────────────────────────────────────────
        public static ComfyProcess Instance { get; } = new ComfyProcess();

        private Process _process;
        private readonly object _lock = new object();
        private bool _disposed;

        private ComfyProcess() { }

        // ── Events ───────────────────────────────────────────────────────

        /// <summary>
        /// Raised whenever the ComfyUI process writes a line to stdout or stderr.
        /// </summary>
        public event Action<string> OutputReceived;

        /// <summary>
        /// Raised when the process starts (true) or exits (false).
        /// </summary>
        public event Action<bool> StateChanged;

        // ── Properties ───────────────────────────────────────────────────

        /// <summary>
        /// Whether the ComfyUI process is currently running.
        /// </summary>
        public bool IsRunning
        {
            get
            {
                lock (_lock)
                {
                    return _process != null && !_process.HasExited;
                }
            }
        }

        /// <summary>
        /// Resolved path to the Python executable. Searches the runtime directory
        /// first, then falls back to the system PATH.
        /// </summary>
        public string PythonPath
        {
            get
            {
                // Check for portable Python inside runtime/
                string runtimePython = Path.Combine(PortablePaths.RuntimeDir, "python", "python.exe");
                if (File.Exists(runtimePython))
                    return runtimePython;

                // Check for Python in the runtime root (alternative layout)
                string runtimeRoot = Path.Combine(PortablePaths.RuntimeDir, "python.exe");
                if (File.Exists(runtimeRoot))
                    return runtimeRoot;

                // Fall back to system PATH
                return "python";
            }
        }

        // ── Public Methods ───────────────────────────────────────────────

        /// <summary>
        /// Starts the embedded ComfyUI process. Locates Python, sets up the
        /// working directory to runtime/ComfyUI, and launches main.py with
        /// the configured port and listen address.
        /// </summary>
        public void Start()
        {
            lock (_lock)
            {
                if (_process != null && !_process.HasExited)
                {
                    Logger.Warn("ComfyUI process is already running.");
                    return;
                }

                var cfg = AppConfig.Current.ComfyUI;
                int port = cfg.EmbeddedPort;

                string comfyDir = Path.Combine(PortablePaths.RuntimeDir, "ComfyUI");
                string mainPy = Path.Combine(comfyDir, "main.py");

                if (!File.Exists(mainPy))
                {
                    Logger.Error($"ComfyUI main.py not found at: {mainPy}");
                    return;
                }

                try
                {
                    var startInfo = new ProcessStartInfo
                    {
                        FileName = PythonPath,
                        Arguments = $"\"{mainPy}\" --port {port} --listen 127.0.0.1",
                        WorkingDirectory = comfyDir,
                        UseShellExecute = false,
                        RedirectStandardOutput = true,
                        RedirectStandardError = true,
                        CreateNoWindow = true
                    };

                    _process = new Process { StartInfo = startInfo, EnableRaisingEvents = true };
                    _process.OutputDataReceived += OnOutputDataReceived;
                    _process.ErrorDataReceived += OnErrorDataReceived;
                    _process.Exited += OnProcessExited;

                    _process.Start();
                    _process.BeginOutputReadLine();
                    _process.BeginErrorReadLine();

                    Logger.Info($"ComfyUI process started (PID: {_process.Id}, port: {port}).");
                    StateChanged?.Invoke(true);
                }
                catch (Exception ex)
                {
                    Logger.Error($"Failed to start ComfyUI process: {ex.Message}");
                    _process = null;
                }
            }
        }

        /// <summary>
        /// Stops the ComfyUI process and its entire process tree.
        /// </summary>
        public void Stop()
        {
            lock (_lock)
            {
                if (_process == null)
                    return;

                try
                {
                    if (!_process.HasExited)
                    {
                        // Kill the entire process tree to clean up child processes
                        _process.Kill(entireProcessTree: true);
                        Logger.Info("ComfyUI process stopped.");
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error($"Error stopping ComfyUI process: {ex.Message}");
                }
                finally
                {
                    CleanupProcess();
                }
            }
        }

        /// <summary>
        /// Disposes the process resources and stops the process if running.
        /// </summary>
        public void Dispose()
        {
            if (_disposed)
                return;

            _disposed = true;
            Stop();
        }

        // ── Event Handlers ───────────────────────────────────────────────

        private void OnOutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Data))
                return;

            Logger.Info($"[ComfyUI] {e.Data}");
            OutputReceived?.Invoke(e.Data);
        }

        private void OnErrorDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Data))
                return;

            // ComfyUI writes many normal messages to stderr, so log at Info level
            Logger.Info($"[ComfyUI] {e.Data}");
            OutputReceived?.Invoke(e.Data);
        }

        private void OnProcessExited(object sender, EventArgs e)
        {
            int exitCode = -1;
            try
            {
                if (_process != null)
                    exitCode = _process.ExitCode;
            }
            catch { }

            Logger.Info($"ComfyUI process exited (code: {exitCode}).");
            CleanupProcess();
            StateChanged?.Invoke(false);
        }

        // ── Helpers ──────────────────────────────────────────────────────

        private void CleanupProcess()
        {
            if (_process != null)
            {
                _process.OutputDataReceived -= OnOutputDataReceived;
                _process.ErrorDataReceived -= OnErrorDataReceived;
                _process.Exited -= OnProcessExited;
                _process.Dispose();
                _process = null;
            }
        }
    }
}
