using System;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using ComfyX.Models;

namespace ComfyX.Pages
{
    /// <summary>
    /// AI Chat page - allows users to describe workflows in natural language
    /// and receive AI-generated ComfyUI workflow responses.
    /// </summary>
    public partial class ChatPage : UserControl
    {
        /// <summary>
        /// Observable collection of chat messages displayed in the conversation.
        /// </summary>
        public ObservableCollection<ChatMessage> Messages { get; } = new ObservableCollection<ChatMessage>();

        public ChatPage()
        {
            InitializeComponent();
            MessagesItemsControl.ItemsSource = Messages;

            // Wire up placeholder visibility based on input text
            InputTextBox.TextChanged += InputTextBox_TextChanged;

            // Update welcome/chat visibility when messages change
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

        /// <summary>
        /// Handles the Send button click.
        /// </summary>
        private void SendButton_Click(object sender, RoutedEventArgs e)
        {
            SendMessage();
        }

        /// <summary>
        /// Handles Enter key press in the input box to send the message.
        /// Shift+Enter inserts a newline instead.
        /// </summary>
        private void InputTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && !Keyboard.Modifiers.HasFlag(ModifierKeys.Shift))
            {
                e.Handled = true;
                SendMessage();
            }
        }

        /// <summary>
        /// Sends the current input as a user message, then adds a placeholder AI response.
        /// </summary>
        private void SendMessage()
        {
            string text = InputTextBox.Text?.Trim();
            if (string.IsNullOrEmpty(text))
                return;

            // Add user message
            Messages.Add(new ChatMessage("user", text));
            InputTextBox.Text = string.Empty;

            // Placeholder AI response (will be replaced by real AI integration)
            Messages.Add(new ChatMessage("assistant",
                "I understand your request. I'm analyzing the best workflow approach for this...\n\n" +
                "(AI engine not connected yet. Configure your API key in Settings.)"));

            // Scroll to bottom
            ChatScrollViewer.ScrollToEnd();
        }
    }
}
