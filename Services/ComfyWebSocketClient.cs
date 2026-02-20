using System;
using System.Net.WebSockets;
using System.Text;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    /// <summary>
    /// WebSocket client that connects to the ComfyUI server for real-time
    /// execution progress updates. Parses incoming JSON messages and raises
    /// typed events for progress, completion, and errors. Thread-safe;
    /// events are marshalled to the WPF UI thread via Dispatcher.
    /// </summary>
    public class ComfyWebSocketClient
    {
        // ── Singleton ────────────────────────────────────────────────────
        public static ComfyWebSocketClient Instance { get; } = new ComfyWebSocketClient();

        private ClientWebSocket _ws;
        private CancellationTokenSource _cts;
        private readonly object _lock = new object();
        private string _clientId;

        private ComfyWebSocketClient()
        {
            _clientId = Guid.NewGuid().ToString("N");
        }

        // ── Events ───────────────────────────────────────────────────────

        /// <summary>
        /// Raised when ComfyUI reports progress on a node (type "progress").
        /// </summary>
        public event Action<ComfyProgress> ProgressUpdated;

        /// <summary>
        /// Raised when a prompt finishes execution (type "executed").
        /// The string parameter is the prompt_id.
        /// </summary>
        public event Action<string> ExecutionCompleted;

        /// <summary>
        /// Raised when a prompt encounters an error (type "execution_error").
        /// The string parameter contains the error details.
        /// </summary>
        public event Action<string> ExecutionError;

        // ── Properties ───────────────────────────────────────────────────

        /// <summary>
        /// Unique client identifier sent to the ComfyUI WebSocket server.
        /// </summary>
        public string ClientId => _clientId;

        /// <summary>
        /// Whether the WebSocket is currently connected.
        /// </summary>
        public bool IsConnected
        {
            get
            {
                lock (_lock)
                {
                    return _ws != null && _ws.State == WebSocketState.Open;
                }
            }
        }

        // ── Public Methods ───────────────────────────────────────────────

        /// <summary>
        /// Connects to the ComfyUI WebSocket endpoint. The URL should be
        /// the HTTP base URL (e.g. "http://127.0.0.1:8188"); the method
        /// converts it to a ws:// URL and appends the client ID.
        /// </summary>
        public async Task ConnectAsync(string url)
        {
            await DisconnectAsync();

            lock (_lock)
            {
                _cts = new CancellationTokenSource();
                _ws = new ClientWebSocket();
            }

            try
            {
                // Convert http(s) to ws(s)
                string wsUrl = url.TrimEnd('/');
                wsUrl = wsUrl.Replace("https://", "wss://").Replace("http://", "ws://");
                wsUrl = $"{wsUrl}/ws?clientId={_clientId}";

                Logger.Info($"WebSocket connecting to: {wsUrl}");
                await _ws.ConnectAsync(new Uri(wsUrl), _cts.Token);
                Logger.Info("WebSocket connected.");

                // Start the background receive loop
                _ = Task.Run(() => ReceiveLoopAsync(_ws, _cts.Token));
            }
            catch (Exception ex)
            {
                Logger.Error($"WebSocket connection failed: {ex.Message}");
                CleanupSocket();
            }
        }

        /// <summary>
        /// Gracefully disconnects the WebSocket connection.
        /// </summary>
        public async Task DisconnectAsync()
        {
            ClientWebSocket ws;
            CancellationTokenSource cts;

            lock (_lock)
            {
                ws = _ws;
                cts = _cts;
                _ws = null;
                _cts = null;
            }

            if (cts != null)
            {
                cts.Cancel();
            }

            if (ws != null)
            {
                try
                {
                    if (ws.State == WebSocketState.Open)
                    {
                        // Use a short timeout for the close handshake
                        using var closeCts = new CancellationTokenSource(TimeSpan.FromSeconds(3));
                        await ws.CloseAsync(WebSocketCloseStatus.NormalClosure, "Client disconnecting", closeCts.Token);
                    }
                }
                catch (Exception ex)
                {
                    Logger.Debug($"WebSocket close exception (expected during shutdown): {ex.Message}");
                }
                finally
                {
                    ws.Dispose();
                }
            }

            cts?.Dispose();

            Logger.Info("WebSocket disconnected.");
        }

        // ── Receive Loop ─────────────────────────────────────────────────

        /// <summary>
        /// Continuously reads messages from the WebSocket and dispatches
        /// parsed events. Runs until cancelled or the connection drops.
        /// </summary>
        private async Task ReceiveLoopAsync(ClientWebSocket ws, CancellationToken token)
        {
            var buffer = new byte[8192];

            try
            {
                while (!token.IsCancellationRequested && ws.State == WebSocketState.Open)
                {
                    var messageBuffer = new StringBuilder();
                    WebSocketReceiveResult result;

                    do
                    {
                        result = await ws.ReceiveAsync(new ArraySegment<byte>(buffer), token);

                        if (result.MessageType == WebSocketMessageType.Close)
                        {
                            Logger.Info("WebSocket server initiated close.");
                            return;
                        }

                        if (result.MessageType == WebSocketMessageType.Text)
                        {
                            messageBuffer.Append(Encoding.UTF8.GetString(buffer, 0, result.Count));
                        }
                    }
                    while (!result.EndOfMessage);

                    if (messageBuffer.Length > 0)
                    {
                        ProcessMessage(messageBuffer.ToString());
                    }
                }
            }
            catch (OperationCanceledException)
            {
                // Normal shutdown — ignore
            }
            catch (WebSocketException ex)
            {
                Logger.Error($"WebSocket receive error: {ex.Message}");
            }
            catch (Exception ex)
            {
                Logger.Error($"WebSocket unexpected error: {ex.Message}");
            }
        }

        // ── Message Processing ───────────────────────────────────────────

        /// <summary>
        /// Parses a JSON message from ComfyUI and raises the appropriate event.
        /// Expected message format: { "type": "...", "data": { ... } }
        /// </summary>
        private void ProcessMessage(string json)
        {
            try
            {
                using var doc = JsonDocument.Parse(json);
                var root = doc.RootElement;

                if (!root.TryGetProperty("type", out var typeElem))
                    return;

                string type = typeElem.GetString();

                switch (type)
                {
                    case "progress":
                        HandleProgress(root);
                        break;

                    case "executed":
                        HandleExecuted(root);
                        break;

                    case "execution_error":
                        HandleExecutionError(root);
                        break;

                    default:
                        Logger.Debug($"WebSocket unhandled message type: {type}");
                        break;
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to process WebSocket message: {ex.Message}");
            }
        }

        private void HandleProgress(JsonElement root)
        {
            if (!root.TryGetProperty("data", out var data))
                return;

            var progress = new ComfyProgress();

            if (data.TryGetProperty("node", out var nodeElem))
                progress.NodeId = nodeElem.GetString() ?? string.Empty;

            if (data.TryGetProperty("value", out var valueElem))
                progress.Value = valueElem.GetInt32();

            if (data.TryGetProperty("max", out var maxElem))
                progress.Max = maxElem.GetInt32();

            DispatchToUI(() => ProgressUpdated?.Invoke(progress));
        }

        private void HandleExecuted(JsonElement root)
        {
            string promptId = string.Empty;

            if (root.TryGetProperty("data", out var data)
                && data.TryGetProperty("prompt_id", out var pidElem))
            {
                promptId = pidElem.GetString() ?? string.Empty;
            }

            DispatchToUI(() => ExecutionCompleted?.Invoke(promptId));
        }

        private void HandleExecutionError(JsonElement root)
        {
            string errorText = string.Empty;

            if (root.TryGetProperty("data", out var data))
            {
                // Try to extract a human-readable error message
                if (data.TryGetProperty("exception_message", out var msgElem))
                    errorText = msgElem.GetString() ?? string.Empty;
                else
                    errorText = data.ToString();
            }

            Logger.Error($"ComfyUI execution error: {errorText}");
            DispatchToUI(() => ExecutionError?.Invoke(errorText));
        }

        // ── Helpers ──────────────────────────────────────────────────────

        /// <summary>
        /// Marshals an action to the WPF UI thread. Falls back to direct
        /// invocation when no dispatcher is available (e.g. in tests).
        /// </summary>
        private static void DispatchToUI(Action action)
        {
            Dispatcher dispatcher = Application.Current?.Dispatcher;

            if (dispatcher == null || dispatcher.CheckAccess())
            {
                action();
            }
            else
            {
                dispatcher.BeginInvoke(action);
            }
        }

        private void CleanupSocket()
        {
            lock (_lock)
            {
                _ws?.Dispose();
                _ws = null;
                _cts?.Dispose();
                _cts = null;
            }
        }
    }
}
