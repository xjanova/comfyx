using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.Json;
using System.Windows.Media;
using ComfyX.Models;

namespace ComfyX.Helpers
{
    public static class NodeGraphRenderer
    {
        private const double NodeWidth = 220;
        private const double NodeSpacingX = 280;
        private const double NodeSpacingY = 160;
        private const double StartX = 60;
        private const double StartY = 60;

        public static (List<WorkflowNode> Nodes, List<NodeConnection> Connections) ParseWorkflow(string workflowJson)
        {
            var nodes = new List<WorkflowNode>();
            var connections = new List<NodeConnection>();

            if (string.IsNullOrWhiteSpace(workflowJson))
                return (nodes, connections);

            try
            {
                using var doc = JsonDocument.Parse(workflowJson);
                var root = doc.RootElement;

                if (root.ValueKind != JsonValueKind.Object)
                    return (nodes, connections);

                // First pass: create nodes
                foreach (var prop in root.EnumerateObject())
                {
                    string nodeId = prop.Name;
                    var nodeData = prop.Value;

                    if (nodeData.ValueKind != JsonValueKind.Object)
                        continue;

                    string classType = "";
                    if (nodeData.TryGetProperty("class_type", out var ct))
                        classType = ct.GetString() ?? "";

                    string title = classType;
                    if (nodeData.TryGetProperty("_meta", out var meta) &&
                        meta.TryGetProperty("title", out var titleProp))
                        title = titleProp.GetString() ?? classType;

                    var node = new WorkflowNode
                    {
                        Id = nodeId,
                        ClassType = classType,
                        Title = title,
                        AccentColor = GetCategoryColor(classType)
                    };

                    // Parse inputs
                    if (nodeData.TryGetProperty("inputs", out var inputs) && inputs.ValueKind == JsonValueKind.Object)
                    {
                        foreach (var input in inputs.EnumerateObject())
                        {
                            var port = new NodePort { Name = input.Name, Type = "any" };

                            if (input.Value.ValueKind == JsonValueKind.Array && input.Value.GetArrayLength() == 2)
                            {
                                // This is a connection: ["nodeId", slotIndex]
                                string srcNodeId = input.Value[0].GetString() ?? "";
                                int srcSlot = input.Value[1].GetInt32();
                                port.IsConnected = true;
                                port.ConnectedNodeId = srcNodeId;
                                port.ConnectedSlot = srcSlot;

                                connections.Add(new NodeConnection
                                {
                                    FromNodeId = srcNodeId,
                                    FromSlot = srcSlot,
                                    ToNodeId = nodeId,
                                    ToSlot = node.Inputs.Count
                                });
                            }
                            else
                            {
                                // Widget value
                                object val = ExtractValue(input.Value);
                                if (val != null)
                                    node.WidgetValues[input.Name] = val;
                            }

                            node.Inputs.Add(port);
                        }
                    }

                    nodes.Add(node);
                }

                // Auto-layout: topological sort → columns
                AutoLayout(nodes, connections);
            }
            catch (Exception ex)
            {
                Logger.Error($"NodeGraphRenderer parse failed: {ex.Message}");
            }

            return (nodes, connections);
        }

        private static void AutoLayout(List<WorkflowNode> nodes, List<NodeConnection> connections)
        {
            if (nodes.Count == 0) return;

            var nodeMap = nodes.ToDictionary(n => n.Id);

            // Calculate depth for each node (how many predecessor layers)
            var depths = new Dictionary<string, int>();
            foreach (var node in nodes)
                depths[node.Id] = 0;

            // BFS to find depths
            bool changed = true;
            int maxIterations = nodes.Count * 2;
            while (changed && maxIterations-- > 0)
            {
                changed = false;
                foreach (var conn in connections)
                {
                    if (depths.ContainsKey(conn.FromNodeId) && depths.ContainsKey(conn.ToNodeId))
                    {
                        int newDepth = depths[conn.FromNodeId] + 1;
                        if (newDepth > depths[conn.ToNodeId])
                        {
                            depths[conn.ToNodeId] = newDepth;
                            changed = true;
                        }
                    }
                }
            }

            // Group by depth (column)
            var columns = nodes.GroupBy(n => depths.GetValueOrDefault(n.Id, 0))
                               .OrderBy(g => g.Key)
                               .ToList();

            foreach (var column in columns)
            {
                int col = column.Key;
                int row = 0;
                foreach (var node in column)
                {
                    node.X = StartX + col * NodeSpacingX;
                    node.Y = StartY + row * NodeSpacingY;

                    // Auto-height based on content
                    int portCount = Math.Max(node.Inputs.Count(p => p.IsConnected), 1);
                    int widgetCount = node.WidgetValues.Count;
                    node.Height = 50 + (portCount + widgetCount) * 22;
                    node.Height = Math.Max(node.Height, 80);

                    row++;
                }
            }
        }

        private static Color GetCategoryColor(string classType)
        {
            string ct = classType.ToLowerInvariant();

            if (ct.Contains("loader") || ct.Contains("checkpoint") || ct.Contains("load"))
                return (Color)ColorConverter.ConvertFromString("#00F5FF"); // Cyan - loaders
            if (ct.Contains("sampler") || ct.Contains("ksampler"))
                return (Color)ColorConverter.ConvertFromString("#FF00FF"); // Pink - sampling
            if (ct.Contains("clip") || ct.Contains("encode") || ct.Contains("conditioning"))
                return (Color)ColorConverter.ConvertFromString("#BF00FF"); // Purple - conditioning
            if (ct.Contains("vae") || ct.Contains("decode"))
                return (Color)ColorConverter.ConvertFromString("#FF6B35"); // Orange - VAE
            if (ct.Contains("save") || ct.Contains("preview") || ct.Contains("output"))
                return (Color)ColorConverter.ConvertFromString("#00FF88"); // Green - output
            if (ct.Contains("latent") || ct.Contains("empty"))
                return (Color)ColorConverter.ConvertFromString("#FFD700"); // Gold - latent
            if (ct.Contains("controlnet") || ct.Contains("control"))
                return (Color)ColorConverter.ConvertFromString("#FF3366"); // Red - controlnet

            return (Color)ColorConverter.ConvertFromString("#808090"); // Gray - default
        }

        private static object ExtractValue(JsonElement element)
        {
            switch (element.ValueKind)
            {
                case JsonValueKind.String: return element.GetString();
                case JsonValueKind.Number:
                    if (element.TryGetInt64(out long l)) return l;
                    return element.GetDouble();
                case JsonValueKind.True: return true;
                case JsonValueKind.False: return false;
                default: return null;
            }
        }
    }
}
