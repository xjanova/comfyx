using System;
using System.IO;
using System.Threading.Tasks;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// Singleton that manages the application license state including persistence
    /// of the license key to disk and periodic validation against the server.
    /// </summary>
    public class LicenseGuard
    {
        /// <summary>
        /// Singleton instance.
        /// </summary>
        public static LicenseGuard Instance { get; } = new LicenseGuard();

        /// <summary>
        /// Current license validation state.
        /// </summary>
        public LicenseInfo CurrentLicense { get; private set; } = new LicenseInfo();

        /// <summary>
        /// The license key that has been saved to disk, or empty if none is stored.
        /// </summary>
        public string SavedLicenseKey { get; private set; } = string.Empty;

        /// <summary>
        /// Convenience property indicating whether the application is currently licensed.
        /// </summary>
        public bool IsLicensed => CurrentLicense?.IsValid == true;

        /// <summary>
        /// Raised whenever the license state changes (validation, activation, deactivation).
        /// </summary>
        public event Action<LicenseInfo> LicenseChanged;

        /// <summary>
        /// File name used to persist the license key inside the data directory.
        /// </summary>
        private const string LicenseFileName = "license.key";

        /// <summary>
        /// Private constructor to enforce singleton usage.
        /// </summary>
        private LicenseGuard() { }

        /// <summary>
        /// Loads the saved license key from disk and validates it against the server.
        /// Call this once during application startup.
        /// </summary>
        public async Task InitializeAsync()
        {
            Logger.Info("Initializing license guard...");

            LoadSavedKey();

            if (!string.IsNullOrWhiteSpace(SavedLicenseKey))
            {
                Logger.Info("Found saved license key, validating...");
                LicenseInfo result = await LicenseClient.ValidateAsync(SavedLicenseKey);
                CurrentLicense = result;
                LicenseChanged?.Invoke(CurrentLicense);

                if (result.IsValid)
                    Logger.Info($"License valid: {result.Status} ({result.LicenseType})");
                else
                    Logger.Warn($"License validation failed: {result.Status}");
            }
            else
            {
                Logger.Info("No saved license key found");
            }
        }

        /// <summary>
        /// Activates a license key on the current machine. If successful the key
        /// is persisted to disk for subsequent launches.
        /// </summary>
        /// <param name="licenseKey">The license key to activate.</param>
        /// <returns>The resulting <see cref="LicenseInfo"/>.</returns>
        public async Task<LicenseInfo> ActivateAsync(string licenseKey)
        {
            Logger.Info("Activating license...");

            LicenseInfo result = await LicenseClient.ActivateAsync(licenseKey);

            if (result.IsValid)
            {
                SavedLicenseKey = licenseKey;
                SaveKey(licenseKey);
                Logger.Info($"License activated: {result.Status} ({result.LicenseType})");
            }
            else
            {
                Logger.Warn($"License activation failed: {result.Status}");
            }

            CurrentLicense = result;
            LicenseChanged?.Invoke(CurrentLicense);
            return result;
        }

        /// <summary>
        /// Deactivates the current license key on the server, removes the persisted
        /// key from disk, and resets the local license state.
        /// </summary>
        public async Task DeactivateAsync()
        {
            Logger.Info("Deactivating license...");

            if (!string.IsNullOrWhiteSpace(SavedLicenseKey))
            {
                await LicenseClient.DeactivateAsync(SavedLicenseKey);
            }

            DeleteKeyFile();
            SavedLicenseKey = string.Empty;
            CurrentLicense = new LicenseInfo();
            LicenseChanged?.Invoke(CurrentLicense);

            Logger.Info("License deactivated and removed");
        }

        /// <summary>
        /// Re-validates the saved license key against the server and updates the
        /// local state. Does nothing if no key is saved.
        /// </summary>
        public async Task RefreshAsync()
        {
            if (string.IsNullOrWhiteSpace(SavedLicenseKey))
            {
                Logger.Debug("No license key to refresh");
                return;
            }

            Logger.Debug("Refreshing license status...");

            LicenseInfo result = await LicenseClient.ValidateAsync(SavedLicenseKey);
            CurrentLicense = result;
            LicenseChanged?.Invoke(CurrentLicense);

            if (result.IsValid)
                Logger.Debug($"License refresh OK: {result.Status}");
            else
                Logger.Warn($"License refresh failed: {result.Status}");
        }

        /// <summary>
        /// Resolves the full path to the license key file inside the data directory.
        /// </summary>
        private static string GetKeyFilePath()
        {
            return Path.Combine(PortablePaths.DataDir, LicenseFileName);
        }

        /// <summary>
        /// Loads the license key from the on-disk file into <see cref="SavedLicenseKey"/>.
        /// </summary>
        private void LoadSavedKey()
        {
            try
            {
                string path = GetKeyFilePath();
                if (File.Exists(path))
                {
                    SavedLicenseKey = File.ReadAllText(path).Trim();
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to load license key: {ex.Message}");
                SavedLicenseKey = string.Empty;
            }
        }

        /// <summary>
        /// Persists a license key to the on-disk file.
        /// </summary>
        private static void SaveKey(string licenseKey)
        {
            try
            {
                string path = GetKeyFilePath();
                string dir = Path.GetDirectoryName(path);
                if (!Directory.Exists(dir))
                    Directory.CreateDirectory(dir);

                File.WriteAllText(path, licenseKey);
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to save license key: {ex.Message}");
            }
        }

        /// <summary>
        /// Deletes the on-disk license key file if it exists.
        /// </summary>
        private static void DeleteKeyFile()
        {
            try
            {
                string path = GetKeyFilePath();
                if (File.Exists(path))
                {
                    File.Delete(path);
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to delete license key file: {ex.Message}");
            }
        }
    }
}
