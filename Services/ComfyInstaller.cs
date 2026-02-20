using System;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;
using ComfyX.Helpers;

namespace ComfyX.Services
{
    /// <summary>
    /// Handles automatic downloading and installation of ComfyUI and its
    /// Python runtime into the portable runtime/ directory. Currently a
    /// placeholder; will be fully implemented in a future release.
    /// </summary>
    public class ComfyInstaller
    {
        // ── Singleton ────────────────────────────────────────────────────
        public static ComfyInstaller Instance { get; } = new ComfyInstaller();

        private ComfyInstaller() { }

        // ── Events ───────────────────────────────────────────────────────

        /// <summary>
        /// Raised to report installation progress. Parameters are a status
        /// description string and a percentage (0-100).
        /// </summary>
        public event Action<string, double> InstallProgress;

        // ── Properties ───────────────────────────────────────────────────

        /// <summary>
        /// Returns true when a ComfyUI installation is detected in the
        /// runtime directory (runtime/ComfyUI/main.py exists).
        /// </summary>
        public bool IsInstalled
        {
            get
            {
                string mainPy = Path.Combine(PortablePaths.RuntimeDir, "ComfyUI", "main.py");
                return File.Exists(mainPy);
            }
        }

        // ── Public Methods ───────────────────────────────────────────────

        /// <summary>
        /// Initiates the ComfyUI installation process. Currently a placeholder
        /// that logs the intent; future versions will download portable Python
        /// and clone the ComfyUI repository.
        /// </summary>
        public async Task InstallAsync()
        {
            Logger.Info("Installation not yet implemented.");
            InstallProgress?.Invoke("Installation not yet implemented.", 0);

            // Placeholder: simulate a brief delay so callers awaiting this
            // method don't get a synchronous return.
            await Task.CompletedTask;
        }

        /// <summary>
        /// Attempts to locate a working Python installation by running
        /// "python --version". Returns true if Python is available.
        /// </summary>
        public async Task<bool> CheckPythonAsync()
        {
            try
            {
                // First check the portable Python in the runtime directory
                string portablePython = Path.Combine(PortablePaths.RuntimeDir, "python", "python.exe");
                if (File.Exists(portablePython))
                {
                    bool ok = await RunPythonVersionAsync(portablePython);
                    if (ok) return true;
                }

                // Fall back to system PATH
                return await RunPythonVersionAsync("python");
            }
            catch (Exception ex)
            {
                Logger.Error($"Python check failed: {ex.Message}");
                return false;
            }
        }

        // ── Helpers ──────────────────────────────────────────────────────

        /// <summary>
        /// Runs a Python executable with --version and returns true if it
        /// exits successfully with a version string on stdout.
        /// </summary>
        private static async Task<bool> RunPythonVersionAsync(string pythonPath)
        {
            try
            {
                var startInfo = new ProcessStartInfo
                {
                    FileName = pythonPath,
                    Arguments = "--version",
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                };

                using var process = new Process { StartInfo = startInfo };
                process.Start();

                string output = await process.StandardOutput.ReadToEndAsync();
                string error = await process.StandardError.ReadToEndAsync();

                await process.WaitForExitAsync();

                string versionText = !string.IsNullOrEmpty(output) ? output.Trim() : error.Trim();

                if (process.ExitCode == 0 && versionText.StartsWith("Python", StringComparison.OrdinalIgnoreCase))
                {
                    Logger.Info($"Python found: {versionText} ({pythonPath})");
                    return true;
                }

                return false;
            }
            catch (Exception)
            {
                // Python executable not found or cannot be started
                return false;
            }
        }
    }
}
