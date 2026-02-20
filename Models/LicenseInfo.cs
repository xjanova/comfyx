namespace ComfyX.Models
{
    /// <summary>
    /// Holds the current license validation state returned from the licensing server.
    /// </summary>
    public class LicenseInfo
    {
        /// <summary>
        /// Whether the license key has been validated successfully.
        /// </summary>
        public bool IsValid { get; set; }

        /// <summary>
        /// Human-readable status string (e.g. "active", "expired", "trial").
        /// </summary>
        public string Status { get; set; } = string.Empty;

        /// <summary>
        /// License tier identifier (e.g. "pro", "enterprise", "trial").
        /// </summary>
        public string LicenseType { get; set; } = string.Empty;

        /// <summary>
        /// ISO 8601 expiration date string, or empty if perpetual.
        /// </summary>
        public string ExpiresAt { get; set; } = string.Empty;

        /// <summary>
        /// Number of days remaining before the license expires. Negative if expired.
        /// </summary>
        public int DaysRemaining { get; set; }

        /// <summary>
        /// Current number of device activations for this license key.
        /// </summary>
        public int Activations { get; set; }

        /// <summary>
        /// Maximum allowed device activations for this license key.
        /// </summary>
        public int MaxActivations { get; set; }
    }
}
