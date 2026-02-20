using System;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// HTTP client for communicating with the ComfyX license server API.
    /// Provides methods to validate, activate, and deactivate license keys.
    /// </summary>
    public static class LicenseClient
    {
        /// <summary>
        /// Base URL of the licensing server.
        /// </summary>
        private const string BaseUrl = "https://xman4289.com";

        /// <summary>
        /// Product slug used to identify ComfyX in API requests.
        /// </summary>
        private const string ProductSlug = "comfyx";

        /// <summary>
        /// Shared HttpClient instance for all license API calls.
        /// </summary>
        private static readonly HttpClient _http = new HttpClient
        {
            Timeout = TimeSpan.FromSeconds(15)
        };

        /// <summary>
        /// JSON serializer options using camelCase for property names.
        /// </summary>
        private static readonly JsonSerializerOptions _jsonOptions = new JsonSerializerOptions
        {
            PropertyNameCaseInsensitive = true
        };

        /// <summary>
        /// Validates a license key against the server without consuming an activation slot.
        /// </summary>
        /// <param name="licenseKey">The license key to validate.</param>
        /// <returns>A <see cref="LicenseInfo"/> describing the current license state.</returns>
        public static async Task<LicenseInfo> ValidateAsync(string licenseKey)
        {
            return await PostAsync($"{BaseUrl}/api/license/validate", licenseKey);
        }

        /// <summary>
        /// Activates a license key on the current machine, consuming an activation slot.
        /// </summary>
        /// <param name="licenseKey">The license key to activate.</param>
        /// <returns>A <see cref="LicenseInfo"/> describing the resulting license state.</returns>
        public static async Task<LicenseInfo> ActivateAsync(string licenseKey)
        {
            return await PostAsync($"{BaseUrl}/api/license/activate", licenseKey);
        }

        /// <summary>
        /// Deactivates a license key on the current machine, freeing an activation slot.
        /// </summary>
        /// <param name="licenseKey">The license key to deactivate.</param>
        /// <returns>True if deactivation succeeded; false otherwise.</returns>
        public static async Task<bool> DeactivateAsync(string licenseKey)
        {
            try
            {
                string body = BuildRequestBody(licenseKey);
                using (StringContent content = new StringContent(body, Encoding.UTF8, "application/json"))
                {
                    HttpResponseMessage response = await _http.PostAsync(
                        $"{BaseUrl}/api/license/deactivate", content);

                    if (response.IsSuccessStatusCode)
                    {
                        Logger.Info("License deactivated successfully");
                        return true;
                    }

                    Logger.Warn($"License deactivation failed: HTTP {(int)response.StatusCode}");
                    return false;
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"License deactivation error: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// Sends a POST request to the specified endpoint and parses the response
        /// into a <see cref="LicenseInfo"/> object.
        /// </summary>
        private static async Task<LicenseInfo> PostAsync(string url, string licenseKey)
        {
            try
            {
                string body = BuildRequestBody(licenseKey);
                using (StringContent content = new StringContent(body, Encoding.UTF8, "application/json"))
                {
                    HttpResponseMessage response = await _http.PostAsync(url, content);
                    string json = await response.Content.ReadAsStringAsync();

                    Logger.Debug($"License API response ({url}): {json}");
                    return ParseResponse(json);
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"License API error ({url}): {ex.Message}");
                return new LicenseInfo
                {
                    IsValid = false,
                    Status = "Network error"
                };
            }
        }

        /// <summary>
        /// Builds the JSON request body containing the license key, product slug,
        /// and machine fingerprint.
        /// </summary>
        private static string BuildRequestBody(string licenseKey)
        {
            var payload = new
            {
                license_key = licenseKey,
                product_slug = ProductSlug,
                fingerprint = MachineFingerprint.Generate()
            };
            return JsonSerializer.Serialize(payload);
        }

        /// <summary>
        /// Parses the server JSON response into a <see cref="LicenseInfo"/> object.
        /// Expected shape: { "valid": bool, "license_key": { "status", "license_type",
        /// "expires_at", "activation_usage", "activation_limit" } }
        /// </summary>
        private static LicenseInfo ParseResponse(string json)
        {
            LicenseInfo info = new LicenseInfo();

            try
            {
                using (JsonDocument doc = JsonDocument.Parse(json))
                {
                    JsonElement root = doc.RootElement;

                    if (root.TryGetProperty("valid", out JsonElement validEl))
                    {
                        info.IsValid = validEl.GetBoolean();
                    }

                    if (root.TryGetProperty("license_key", out JsonElement lkEl))
                    {
                        if (lkEl.TryGetProperty("status", out JsonElement statusEl))
                            info.Status = statusEl.GetString() ?? string.Empty;

                        if (lkEl.TryGetProperty("license_type", out JsonElement typeEl))
                            info.LicenseType = typeEl.GetString() ?? string.Empty;

                        if (lkEl.TryGetProperty("expires_at", out JsonElement expiresEl))
                        {
                            info.ExpiresAt = expiresEl.GetString() ?? string.Empty;

                            // Calculate days remaining if an expiration date is present
                            if (DateTime.TryParse(info.ExpiresAt, out DateTime expiresDate))
                            {
                                info.DaysRemaining = (int)(expiresDate.Date - DateTime.UtcNow.Date).TotalDays;
                            }
                        }

                        if (lkEl.TryGetProperty("activation_usage", out JsonElement usageEl))
                            info.Activations = usageEl.GetInt32();

                        if (lkEl.TryGetProperty("activation_limit", out JsonElement limitEl))
                            info.MaxActivations = limitEl.GetInt32();
                    }
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to parse license response: {ex.Message}");
                info.IsValid = false;
                info.Status = "Parse error";
            }

            return info;
        }
    }
}
