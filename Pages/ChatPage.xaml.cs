using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using ComfyX.Helpers;
using ComfyX.Models;
using ComfyX.Services;

namespace ComfyX.Pages
{
    public partial class ChatPage : UserControl
    {
        public ObservableCollection<ChatMessage> Messages { get; } = new ObservableCollection<ChatMessage>();

        private string _lastWorkflowJson;
        private bool _isSending;

        public ChatPage()
        {
            InitializeComponent();
            MessagesItemsControl.ItemsSource = Messages;
            InputTextBox.TextChanged += InputTextBox_TextChanged;
            Messages.CollectionChanged += Messages_CollectionChanged;
        }

        private void InputTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            PlaceholderText.Visibility = string.IsNullOrEmpty(InputTextBox.Text)
                ? Visibility.Visible
                : Visibility.Collapsed;
        }

        private void Messages_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            bool hasMessages = Messages.Count > 0;
            WelcomePanel.Visibility = hasMessages ? Visibility.Collapsed : Visibility.Visible;
            ChatContainer.Visibility = hasMessages ? Visibility.Visible : Visibility.Collapsed;
        }

        private void SendButton_Click(object sender, RoutedEventArgs e)
        {
            SendMessage();
        }

        private void InputTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && !Keyboard.Modifiers.HasFlag(ModifierKeys.Shift))
            {
                e.Handled = true;
                SendMessage();
            }
        }

        private async void SendMessage()
        {
            string text = InputTextBox.Text?.Trim();
            if (string.IsNullOrEmpty(text) || _isSending)
                return;

            _isSending = true;
            SendButton.IsEnabled = false;
            SendButton.Content = "...";

            // Add user message
            Messages.Add(new ChatMessage("user", text));
            InputTextBox.Text = string.Empty;
            ChatScrollViewer.ScrollToEnd();

            try
            {
                // Check if API key is configured
                var config = AppConfig.Current.AI;
                string provider = config.ActiveProvider?.ToLowerInvariant() ?? "openai";
                string apiKey = provider switch
                {
                    "claude" => config.ClaudeApiKey,
                    "gemini" => config.GeminiApiKey,
                    _ => config.OpenAIApiKey
                };

                if (string.IsNullOrEmpty(apiKey))
                {
                    Messages.Add(new ChatMessage("assistant",
                        $"⚠️ No API key configured for {provider}.\n\n" +
                        "Please go to Settings → AI Engine and enter your API key."));
                    ChatScrollViewer.ScrollToEnd();
                    return;
                }

                // Add thinking indicator
                var thinkingMsg = new ChatMessage("assistant", "🔄 Thinking...");
                Messages.Add(thinkingMsg);
                ChatScrollViewer.ScrollToEnd();

                // Build conversation history (exclude the thinking message)
                var history = Messages
                    .Where(m => m != thinkingMsg)
                    .Select(m => new ChatMessage(m.Role, m.Content))
                    .ToList();

                // Remove the user message we just added from history since AIManager adds it
                if (history.Count > 0 && history.Last().Role == "user")
                    history.RemoveAt(history.Count - 1);

                // Send to AI
                string reply = await AIManager.Instance.SendAsync(text, history);

                // Replace thinking message with actual response
                int thinkingIndex = Messages.IndexOf(thinkingMsg);
                if (thinkingIndex >= 0)
                {
                    Messages.RemoveAt(thinkingIndex);
                }
                Messages.Add(new ChatMessage("assistant", reply));
                ChatScrollViewer.ScrollToEnd();

                // Try to extract workflow JSON from the response
                _lastWorkflowJson = WorkflowParser.ExtractWorkflowJson(reply);
                if (_lastWorkflowJson != null && WorkflowParser.ValidateWorkflow(_lastWorkflowJson))
                {
                    Logger.Info("Valid ComfyUI workflow detected in AI response.");

                    // Check if ComfyUI is connected and auto-queue
                    bool connected = await ComfyClient.Instance.CheckConnectionAsync();
                    if (connected)
                    {
                        Messages.Add(new ChatMessage("assistant",
                            "✅ Workflow detected! Queueing to ComfyUI..."));
                        ChatScrollViewer.ScrollToEnd();

                        string promptId = await ComfyClient.Instance.QueuePromptAsync(_lastWorkflowJson);
                        if (promptId != null)
                        {
                            Messages.Add(new ChatMessage("assistant",
                                $"🚀 Workflow queued! (ID: {promptId.Substring(0, Math.Min(8, promptId.Length))}...)\n" +
                                "Check the Preview tab for results."));
                        }
                        else
                        {
                            Messages.Add(new ChatMessage("assistant",
                                "❌ Failed to queue workflow. Check the Log tab for details."));
                        }
                    }
                    else
                    {
                        Messages.Add(new ChatMessage("assistant",
                            "💡 Workflow generated! Start ComfyUI and click 'Start ComfyUI' to connect, " +
                            "then I can queue it automatically."));
                    }
                    ChatScrollViewer.ScrollToEnd();
                }
            }
            catch (Exception ex)
            {
                Messages.Add(new ChatMessage("assistant", $"❌ Error: {ex.Message}"));
                Logger.Error($"Chat error: {ex.Message}");
                ChatScrollViewer.ScrollToEnd();
            }
            finally
            {
                _isSending = false;
                SendButton.IsEnabled = true;
                SendButton.Content = "Send";
            }
        }
    }
}
