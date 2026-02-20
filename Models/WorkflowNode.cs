using System.Collections.Generic;
using System.Windows.Media;

namespace ComfyX.Models
{
    public class NodePort
    {
        public string Name { get; set; } = string.Empty;
        public string Type { get; set; } = string.Empty;
        public bool IsConnected { get; set; }
        public string ConnectedNodeId { get; set; }
        public int ConnectedSlot { get; set; }
    }

    public class NodeConnection
    {
        public string FromNodeId { get; set; }
        public int FromSlot { get; set; }
        public string ToNodeId { get; set; }
        public int ToSlot { get; set; }
    }

    public class WorkflowNode
    {
        public string Id { get; set; } = string.Empty;
        public string ClassType { get; set; } = string.Empty;
        public string Title { get; set; } = string.Empty;
        public double X { get; set; }
        public double Y { get; set; }
        public double Width { get; set; } = 220;
        public double Height { get; set; } = 120;
        public Color AccentColor { get; set; } = Colors.Cyan;
        public List<NodePort> Inputs { get; set; } = new List<NodePort>();
        public List<NodePort> Outputs { get; set; } = new List<NodePort>();
        public Dictionary<string, object> WidgetValues { get; set; } = new Dictionary<string, object>();
    }
}
