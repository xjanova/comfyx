using System;
using System.Management;
using System.Security.Cryptography;
using System.Text;
using ComfyX.Helpers;

namespace ComfyX.Services
{
    /// <summary>
    /// Generates a unique machine identifier by hashing hardware serial numbers
    /// obtained from WMI (CPU, motherboard, disk). The fingerprint is computed
    /// once and cached for the lifetime of the process.
    /// </summary>
    public static class MachineFingerprint
    {
        /// <summary>
        /// Cached fingerprint value. Null until first call to <see cref="Generate"/>.
        /// </summary>
        private static string _cached;

        /// <summary>
        /// Generates (or returns the cached) machine fingerprint string.
        /// The fingerprint is a 32-character hex string derived from the
        /// SHA-256 hash of concatenated hardware identifiers.
        /// </summary>
        public static string Generate()
        {
            if (_cached != null)
                return _cached;

            string cpuId = QueryWmi("SELECT ProcessorId FROM Win32_Processor", "ProcessorId");
            string boardSerial = QueryWmi("SELECT SerialNumber FROM Win32_BaseBoard", "SerialNumber");
            string diskSerial = QueryWmi("SELECT SerialNumber FROM Win32_DiskDrive", "SerialNumber");

            string combined = $"{cpuId}|{boardSerial}|{diskSerial}";

            using (SHA256 sha = SHA256.Create())
            {
                byte[] hash = sha.ComputeHash(Encoding.UTF8.GetBytes(combined));
                StringBuilder sb = new StringBuilder(64);
                foreach (byte b in hash)
                {
                    sb.Append(b.ToString("x2"));
                }

                // Use first 32 hex characters for a shorter fingerprint
                _cached = sb.ToString().Substring(0, 32);
            }

            Logger.Debug($"Machine fingerprint generated: {_cached}");
            return _cached;
        }

        /// <summary>
        /// Executes a WMI query and returns the value of the specified property
        /// from the first result. Returns "unknown" if the query fails.
        /// </summary>
        private static string QueryWmi(string query, string property)
        {
            try
            {
                using (ManagementObjectSearcher searcher = new ManagementObjectSearcher(query))
                {
                    foreach (ManagementObject obj in searcher.Get())
                    {
                        object value = obj[property];
                        if (value != null)
                        {
                            string result = value.ToString().Trim();
                            if (!string.IsNullOrEmpty(result))
                                return result;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Logger.Warn($"WMI query failed ({property}): {ex.Message}");
            }

            return "unknown";
        }
    }
}
