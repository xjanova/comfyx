using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Effects;
using System.Windows.Shapes;
using ComfyX.Helpers;
using ComfyX.Models;
using ComfyX.Services;

namespace ComfyX.Pages
{
    /// <summary>
    /// Node graph canvas page - displays ComfyUI workflow nodes and connections.
    /// Supports pan, zoom, node dragging, and visual rendering of the workflow DAG.
    /// </summary>
    public partial class NodeGraphPage : UserControl
    {
        // ─── State ────────────────────────────────────────────────────
        private List<WorkflowNode> _nodes = new();
        private List<NodeConnection> _connections = new();
        private string _workflowJson;

        // ─── Pan / Zoom ──────────────────────────────────────────────
        private bool _isPanning;
        private Point _panStart;
        private double _panStartX;
        private double _panStartY;
        private const double MinZoom = 0.5;
        private const double MaxZoom = 3.0;
        private const double ZoomStep = 0.1;

        // ─── Node Drag ──────────────────────────────────────────────
        private bool _isDraggingNode;
        private Border _draggedNodeBorder;
        private WorkflowNode _draggedNode;
        private Point _dragOffset;

        // ─── Selection ──────────────────────────────────────────────
        private Border _selectedNodeBorder;
        private WorkflowNode _selectedNode;

        // ─── Node visual map (node id -> Border) ────────────────────
        private readonly Dictionary<string, Border> _nodeBorders = new();

        // ─── Constants ──────────────────────────────────────────────
        private const double PortRadius = 4;
        private const double PortSpacing = 22;
        private const double HeaderHeight = 36;
        private const double PortStartY = 42;

        public NodeGraphPage()
        {
            InitializeComponent();
        }

        // ═══════════════════════════════════════════════════════════════
        // Public API
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Parses a ComfyUI workflow JSON string and renders all nodes
        /// and connections onto the canvas.
        /// </summary>
        public void LoadWorkflow(string workflowJson)
        {
            if (string.IsNullOrWhiteSpace(workflowJson))
                return;

            try
            {
                _workflowJson = workflowJson;
                var (nodes, connections) = NodeGraphRenderer.ParseWorkflow(workflowJson);
                _nodes = nodes;
                _connections = connections;

                RenderGraph();
                Logger.Info($"Node graph loaded: {_nodes.Count} nodes, {_connections.Count} connections.");
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to load workflow into node graph: {ex.Message}");
            }
        }

        // ═══════════════════════════════════════════════════════════════
        // Rendering
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Clears and re-renders the entire graph (connections first, then nodes on top).
        /// </summary>
        private void RenderGraph()
        {
            GraphCanvas.Children.Clear();
            _nodeBorders.Clear();
            _selectedNodeBorder = null;
            _selectedNode = null;

            if (_nodes.Count == 0)
            {
                EmptyState.Visibility = Visibility.Visible;
                GraphCanvas.Visibility = Visibility.Collapsed;
                txtNodeCount.Text = "0 nodes";
                return;
            }

            EmptyState.Visibility = Visibility.Collapsed;
            GraphCanvas.Visibility = Visibility.Visible;
            txtNodeCount.Text = $"{_nodes.Count} nodes";

            // Render connections first (behind nodes)
            foreach (var conn in _connections)
            {
                var path = RenderConnection(conn);
                if (path != null)
                    GraphCanvas.Children.Add(path);
            }

            // Render nodes on top
            foreach (var node in _nodes)
            {
                var border = RenderNode(node);
                _nodeBorders[node.Id] = border;
                GraphCanvas.Children.Add(border);
            }
        }

        /// <summary>
        /// Renders a single workflow node as a styled Border element with header,
        /// input/output ports, and widget values.
        /// </summary>
        private Border RenderNode(WorkflowNode node)
        {
            var accentBrush = new SolidColorBrush(node.AccentColor);

            // Count visible items to compute height
            int inputCount = node.Inputs.Count;
            int outputCount = node.Outputs.Count;
            int widgetCount = node.WidgetValues.Count;
            int portRows = Math.Max(inputCount, outputCount);
            double computedHeight = HeaderHeight + 8 + portRows * PortSpacing + widgetCount * 20 + 12;
            computedHeight = Math.Max(computedHeight, 80);
            node.Height = computedHeight;

            // ── Outer border (the node card) ───────────────────────
            var outerBorder = new Border
            {
                Width = node.Width,
                Background = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#1A1A2E")),
                BorderBrush = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#2A2A3E")),
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(8),
                Effect = new DropShadowEffect
                {
                    Color = node.AccentColor,
                    BlurRadius = 8,
                    ShadowDepth = 0,
                    Opacity = 0.3,
                    Direction = 0
                },
                Cursor = Cursors.Hand,
                Tag = node
            };

            Canvas.SetLeft(outerBorder, node.X);
            Canvas.SetTop(outerBorder, node.Y);

            // ── Main content grid ───────────────────────────────────
            var mainGrid = new Grid();
            mainGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(4) });
            mainGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });

            // Left accent bar
            var accentBar = new Border
            {
                Background = accentBrush,
                CornerRadius = new CornerRadius(8, 0, 0, 8),
                Width = 4
            };
            Grid.SetColumn(accentBar, 0);
            mainGrid.Children.Add(accentBar);

            // Right content panel
            var contentPanel = new StackPanel { Margin = new Thickness(10, 8, 10, 8) };
            Grid.SetColumn(contentPanel, 1);
            mainGrid.Children.Add(contentPanel);

            // ── Header: class type ──────────────────────────────────
            var headerText = new TextBlock
            {
                Text = node.Title,
                FontSize = 13,
                FontWeight = FontWeights.Bold,
                Foreground = accentBrush,
                TextTrimming = TextTrimming.CharacterEllipsis,
                Margin = new Thickness(0, 0, 0, 6)
            };
            contentPanel.Children.Add(headerText);

            // ── Separator line ──────────────────────────────────────
            var separator = new Border
            {
                Height = 1,
                Background = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#2A2A3E")),
                Margin = new Thickness(0, 0, 0, 6)
            };
            contentPanel.Children.Add(separator);

            // ── Ports (inputs on left, outputs on right per row) ────
            int maxPorts = Math.Max(inputCount, outputCount);
            for (int i = 0; i < maxPorts; i++)
            {
                var portRow = new Grid();
                portRow.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
                portRow.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
                portRow.Margin = new Thickness(0, 1, 0, 1);

                // Input port (left side)
                if (i < inputCount)
                {
                    var inputPort = node.Inputs[i];
                    var inputPanel = new StackPanel { Orientation = Orientation.Horizontal, VerticalAlignment = VerticalAlignment.Center };

                    var inputDot = new Ellipse
                    {
                        Width = 8,
                        Height = 8,
                        Fill = inputPort.IsConnected
                            ? new SolidColorBrush((Color)ColorConverter.ConvertFromString("#00F5FF"))
                            : new SolidColorBrush((Color)ColorConverter.ConvertFromString("#555566")),
                        Margin = new Thickness(0, 0, 5, 0),
                        VerticalAlignment = VerticalAlignment.Center
                    };
                    inputPanel.Children.Add(inputDot);

                    var inputLabel = new TextBlock
                    {
                        Text = inputPort.Name,
                        FontSize = 10,
                        Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#B0B0C0")),
                        VerticalAlignment = VerticalAlignment.Center,
                        TextTrimming = TextTrimming.CharacterEllipsis
                    };
                    inputPanel.Children.Add(inputLabel);

                    Grid.SetColumn(inputPanel, 0);
                    portRow.Children.Add(inputPanel);
                }

                // Output port (right side)
                if (i < outputCount)
                {
                    var outputPort = node.Outputs[i];
                    var outputPanel = new StackPanel
                    {
                        Orientation = Orientation.Horizontal,
                        HorizontalAlignment = HorizontalAlignment.Right,
                        VerticalAlignment = VerticalAlignment.Center
                    };

                    var outputLabel = new TextBlock
                    {
                        Text = outputPort.Name,
                        FontSize = 10,
                        Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#B0B0C0")),
                        VerticalAlignment = VerticalAlignment.Center,
                        TextTrimming = TextTrimming.CharacterEllipsis,
                        Margin = new Thickness(0, 0, 5, 0)
                    };
                    outputPanel.Children.Add(outputLabel);

                    var outputDot = new Ellipse
                    {
                        Width = 8,
                        Height = 8,
                        Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#00FF88")),
                        VerticalAlignment = VerticalAlignment.Center
                    };
                    outputPanel.Children.Add(outputDot);

                    Grid.SetColumn(outputPanel, 1);
                    portRow.Children.Add(outputPanel);
                }

                contentPanel.Children.Add(portRow);
            }

            // ── Widget values ───────────────────────────────────────
            if (node.WidgetValues.Count > 0)
            {
                var widgetSeparator = new Border
                {
                    Height = 1,
                    Background = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#2A2A3E")),
                    Margin = new Thickness(0, 4, 0, 4)
                };
                contentPanel.Children.Add(widgetSeparator);

                foreach (var kv in node.WidgetValues)
                {
                    string valueStr = kv.Value?.ToString() ?? "";
                    // Truncate long values for display
                    if (valueStr.Length > 40)
                        valueStr = valueStr.Substring(0, 37) + "...";

                    var widgetText = new TextBlock
                    {
                        Text = $"{kv.Key}: {valueStr}",
                        FontSize = 10,
                        Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#707080")),
                        TextTrimming = TextTrimming.CharacterEllipsis,
                        Margin = new Thickness(0, 1, 0, 1)
                    };
                    contentPanel.Children.Add(widgetText);
                }
            }

            outerBorder.Child = mainGrid;

            // ── Mouse events for drag and selection ─────────────────
            outerBorder.MouseLeftButtonDown += Node_MouseLeftButtonDown;
            outerBorder.MouseLeftButtonUp += Node_MouseLeftButtonUp;
            outerBorder.MouseMove += Node_MouseMove;

            return outerBorder;
        }

        /// <summary>
        /// Renders a bezier curve connection between two node ports.
        /// </summary>
        private Path RenderConnection(NodeConnection conn)
        {
            // Find source and target nodes
            var fromNode = _nodes.FirstOrDefault(n => n.Id == conn.FromNodeId);
            var toNode = _nodes.FirstOrDefault(n => n.Id == conn.ToNodeId);
            if (fromNode == null || toNode == null)
                return null;

            // Calculate port positions
            // Output port: right edge of source node, offset by slot index
            double fromX = fromNode.X + fromNode.Width;
            double fromY = fromNode.Y + PortStartY + conn.FromSlot * PortSpacing;

            // Input port: left edge of target node, offset by slot index
            double toX = toNode.X;
            double toY = toNode.Y + PortStartY + conn.ToSlot * PortSpacing;

            // Bezier control points (horizontal offset for smooth curves)
            double controlDist = Math.Abs(toX - fromX) * 0.5;
            controlDist = Math.Max(controlDist, 60);

            var startPoint = new Point(fromX, fromY);
            var endPoint = new Point(toX, toY);
            var control1 = new Point(fromX + controlDist, fromY);
            var control2 = new Point(toX - controlDist, toY);

            var figure = new PathFigure { StartPoint = startPoint, IsClosed = false };
            figure.Segments.Add(new BezierSegment(control1, control2, endPoint, true));

            var geometry = new PathGeometry();
            geometry.Figures.Add(figure);

            var path = new Path
            {
                Data = geometry,
                Stroke = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#40FFFFFF")),
                StrokeThickness = 2,
                IsHitTestVisible = false
            };

            return path;
        }

        /// <summary>
        /// Re-renders only the connection lines (used after a node drag to update positions).
        /// Preserves node elements and re-inserts connections at the bottom of the z-order.
        /// </summary>
        private void RefreshConnections()
        {
            // Remove existing connection paths
            var connectionPaths = GraphCanvas.Children.OfType<Path>().ToList();
            foreach (var p in connectionPaths)
                GraphCanvas.Children.Remove(p);

            // Re-render connections and insert at index 0 (behind nodes)
            int insertIndex = 0;
            foreach (var conn in _connections)
            {
                var path = RenderConnection(conn);
                if (path != null)
                {
                    GraphCanvas.Children.Insert(insertIndex, path);
                    insertIndex++;
                }
            }
        }

        // ═══════════════════════════════════════════════════════════════
        // Node Selection
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Selects a node, increasing its glow effect and deselecting the previous one.
        /// </summary>
        private void SelectNode(Border nodeBorder, WorkflowNode node)
        {
            // Deselect previous
            if (_selectedNodeBorder != null && _selectedNode != null)
            {
                if (_selectedNodeBorder.Effect is DropShadowEffect prevShadow)
                {
                    prevShadow.Opacity = 0.3;
                    prevShadow.BlurRadius = 8;
                }
                _selectedNodeBorder.BorderBrush = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#2A2A3E"));
            }

            // Select new
            _selectedNodeBorder = nodeBorder;
            _selectedNode = node;

            if (nodeBorder.Effect is DropShadowEffect shadow)
            {
                shadow.Opacity = 0.7;
                shadow.BlurRadius = 16;
            }
            nodeBorder.BorderBrush = new SolidColorBrush(node.AccentColor) { Opacity = 0.6 };
        }

        // ═══════════════════════════════════════════════════════════════
        // Node Drag Handlers
        // ═══════════════════════════════════════════════════════════════

        private void Node_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is not Border border || border.Tag is not WorkflowNode node)
                return;

            // Select this node
            SelectNode(border, node);

            // Start drag
            _isDraggingNode = true;
            _draggedNodeBorder = border;
            _draggedNode = node;
            _dragOffset = e.GetPosition(border);
            border.CaptureMouse();

            e.Handled = true;
        }

        private void Node_MouseMove(object sender, MouseEventArgs e)
        {
            if (!_isDraggingNode || _draggedNodeBorder == null || _draggedNode == null)
                return;

            var pos = e.GetPosition(GraphCanvas);
            double newX = pos.X - _dragOffset.X;
            double newY = pos.Y - _dragOffset.Y;

            Canvas.SetLeft(_draggedNodeBorder, newX);
            Canvas.SetTop(_draggedNodeBorder, newY);

            _draggedNode.X = newX;
            _draggedNode.Y = newY;

            // Live-update connections while dragging
            RefreshConnections();

            e.Handled = true;
        }

        private void Node_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (!_isDraggingNode)
                return;

            _isDraggingNode = false;
            _draggedNodeBorder?.ReleaseMouseCapture();
            _draggedNodeBorder = null;
            _draggedNode = null;

            e.Handled = true;
        }

        // ═══════════════════════════════════════════════════════════════
        // Canvas Pan / Zoom
        // ═══════════════════════════════════════════════════════════════

        private void GraphCanvas_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            // Only start panning if Ctrl is held (otherwise, let node click through)
            if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
            {
                _isPanning = true;
                _panStart = e.GetPosition(this);
                _panStartX = CanvasPan.X;
                _panStartY = CanvasPan.Y;
                GraphCanvas.CaptureMouse();
                e.Handled = true;
            }
        }

        private void GraphCanvas_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (_isPanning)
            {
                _isPanning = false;
                GraphCanvas.ReleaseMouseCapture();
                e.Handled = true;
            }
        }

        private void GraphCanvas_MouseMove(object sender, MouseEventArgs e)
        {
            if (!_isPanning)
                return;

            var current = e.GetPosition(this);
            double dx = current.X - _panStart.X;
            double dy = current.Y - _panStart.Y;

            CanvasPan.X = _panStartX + dx;
            CanvasPan.Y = _panStartY + dy;

            e.Handled = true;
        }

        private void GraphCanvas_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double delta = e.Delta > 0 ? ZoomStep : -ZoomStep;
            ApplyZoom(delta);
            e.Handled = true;
        }

        /// <summary>
        /// Applies a zoom delta to the canvas scale, clamped to [MinZoom, MaxZoom].
        /// </summary>
        private void ApplyZoom(double delta)
        {
            double newScale = Math.Clamp(CanvasScale.ScaleX + delta, MinZoom, MaxZoom);
            CanvasScale.ScaleX = newScale;
            CanvasScale.ScaleY = newScale;
            txtZoom.Text = $"{newScale * 100:F0}%";
        }

        // ═══════════════════════════════════════════════════════════════
        // Toolbar Handlers
        // ═══════════════════════════════════════════════════════════════

        /// <summary>
        /// Loads workflow JSON from MainWindow's last generated/loaded workflow.
        /// </summary>
        private void BtnLoadChat_Click(object sender, RoutedEventArgs e)
        {
            if (Window.GetWindow(this) is not MainWindow mainWindow)
            {
                Logger.Warn("Cannot find MainWindow reference.");
                return;
            }

            string json = mainWindow.LastWorkflowJson;
            if (!string.IsNullOrEmpty(json))
            {
                LoadWorkflow(json);
            }
            else
            {
                Logger.Warn("No workflow available. Generate one from AI Chat first.");
            }
        }

        /// <summary>
        /// Queues the current workflow to ComfyUI for execution.
        /// </summary>
        private async void BtnQueue_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(_workflowJson))
            {
                Logger.Warn("No workflow to queue. Load a workflow first.");
                return;
            }

            try
            {
                btnQueue.IsEnabled = false;
                btnQueue.Content = "Queueing...";

                string promptId = await ComfyClient.Instance.QueuePromptAsync(_workflowJson);
                if (promptId != null)
                {
                    Logger.Info($"Workflow queued from Node Graph: {promptId.Substring(0, Math.Min(8, promptId.Length))}...");
                }
                else
                {
                    Logger.Warn("Failed to queue workflow. Check ComfyUI connection.");
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Queue failed: {ex.Message}");
            }
            finally
            {
                btnQueue.IsEnabled = true;
                btnQueue.Content = "Queue";
            }
        }

        /// <summary>
        /// Clears the canvas and resets to the empty state.
        /// </summary>
        private void BtnClear_Click(object sender, RoutedEventArgs e)
        {
            _nodes.Clear();
            _connections.Clear();
            _workflowJson = null;
            _nodeBorders.Clear();
            _selectedNodeBorder = null;
            _selectedNode = null;

            GraphCanvas.Children.Clear();
            EmptyState.Visibility = Visibility.Visible;
            GraphCanvas.Visibility = Visibility.Collapsed;
            txtNodeCount.Text = "0 nodes";

            Logger.Info("Node graph cleared.");
        }

        /// <summary>
        /// Navigates to the Chat page to generate a workflow with AI.
        /// </summary>
        private void BtnGenerateAI_Click(object sender, RoutedEventArgs e)
        {
            if (Window.GetWindow(this) is MainWindow mainWindow)
                mainWindow.GoToPage(Page.Chat);
        }

        /// <summary>
        /// Increases the canvas zoom level by one step.
        /// </summary>
        private void BtnZoomIn_Click(object sender, RoutedEventArgs e)
        {
            ApplyZoom(ZoomStep);
        }

        /// <summary>
        /// Decreases the canvas zoom level by one step.
        /// </summary>
        private void BtnZoomOut_Click(object sender, RoutedEventArgs e)
        {
            ApplyZoom(-ZoomStep);
        }
    }
}
