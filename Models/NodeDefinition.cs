using System.Collections.Generic;

namespace ComfyX.Models
{
    /// <summary>
    /// Describes a single input slot on a ComfyUI node.
    /// </summary>
    public class NodeInput
    {
        /// <summary>
        /// Display name of the input (e.g. "image", "seed", "model").
        /// </summary>
        public string Name { get; set; } = string.Empty;

        /// <summary>
        /// Data type identifier (e.g. "IMAGE", "INT", "STRING", "MODEL").
        /// </summary>
        public string Type { get; set; } = string.Empty;

        /// <summary>
        /// Whether this input must be connected or have a value for the node to execute.
        /// </summary>
        public bool Required { get; set; }

        /// <summary>
        /// Default value for the input when none is explicitly provided. Null if no default.
        /// </summary>
        public object DefaultValue { get; set; }

        /// <summary>
        /// Enumerated options for combo/dropdown inputs. Empty for free-form inputs.
        /// </summary>
        public List<string> Options { get; set; } = new List<string>();
    }

    /// <summary>
    /// Describes a single output slot on a ComfyUI node.
    /// </summary>
    public class NodeOutput
    {
        /// <summary>
        /// Display name of the output (e.g. "IMAGE", "LATENT").
        /// </summary>
        public string Name { get; set; } = string.Empty;

        /// <summary>
        /// Data type identifier matching the ComfyUI type system.
        /// </summary>
        public string Type { get; set; } = string.Empty;
    }

    /// <summary>
    /// Full definition of a ComfyUI node type, parsed from the server's object_info endpoint.
    /// </summary>
    public class NodeDefinition
    {
        /// <summary>
        /// Internal Python class name used by ComfyUI (e.g. "KSampler", "CLIPTextEncode").
        /// </summary>
        public string ClassName { get; set; } = string.Empty;

        /// <summary>
        /// Human-readable name shown in the node graph UI.
        /// </summary>
        public string DisplayName { get; set; } = string.Empty;

        /// <summary>
        /// Hierarchical category path for the node menu (e.g. "sampling", "conditioning/text").
        /// </summary>
        public string Category { get; set; } = string.Empty;

        /// <summary>
        /// Optional description of what this node does.
        /// </summary>
        public string Description { get; set; } = string.Empty;

        /// <summary>
        /// All input slots defined by this node type.
        /// </summary>
        public List<NodeInput> Inputs { get; set; } = new List<NodeInput>();

        /// <summary>
        /// All output slots defined by this node type.
        /// </summary>
        public List<NodeOutput> Outputs { get; set; } = new List<NodeOutput>();

        /// <summary>
        /// Whether this node produces final output (images, video) that should trigger preview.
        /// </summary>
        public bool IsOutputNode { get; set; }
    }
}
