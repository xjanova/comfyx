using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ComfyX.Helpers;
using ComfyX.Models;

namespace ComfyX.Services
{
    public class NodeRegistry
    {
        public static NodeRegistry Instance { get; } = new NodeRegistry();

        public Dictionary<string, NodeDefinition> Nodes { get; private set; } = new Dictionary<string, NodeDefinition>();
        public bool IsLoaded => Nodes.Count > 0;

        private NodeRegistry() { }

        public async Task RefreshAsync()
        {
            try
            {
                var nodes = await ComfyClient.Instance.GetObjectInfoAsync();
                if (nodes != null)
                {
                    Nodes = nodes;
                    Logger.Info($"NodeRegistry loaded {Nodes.Count} node definitions.");
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to refresh node registry: {ex.Message}");
            }
        }

        public NodeDefinition GetNode(string className)
        {
            Nodes.TryGetValue(className, out var node);
            return node;
        }

        public List<NodeDefinition> SearchNodes(string query)
        {
            if (string.IsNullOrWhiteSpace(query))
                return Nodes.Values.ToList();

            return Nodes.Values
                .Where(n => n.DisplayName.Contains(query, StringComparison.OrdinalIgnoreCase)
                         || n.Category.Contains(query, StringComparison.OrdinalIgnoreCase)
                         || n.ClassName.Contains(query, StringComparison.OrdinalIgnoreCase))
                .ToList();
        }
    }
}
