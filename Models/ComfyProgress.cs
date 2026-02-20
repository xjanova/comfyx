namespace ComfyX.Models
{
    /// <summary>
    /// Tracks execution progress of a single ComfyUI node during workflow execution.
    /// </summary>
    public class ComfyProgress
    {
        /// <summary>
        /// The unique identifier of the node currently being executed.
        /// </summary>
        public string NodeId { get; set; } = string.Empty;

        /// <summary>
        /// Current progress value (e.g. current step number).
        /// </summary>
        public int Value { get; set; }

        /// <summary>
        /// Maximum progress value (e.g. total steps).
        /// </summary>
        public int Max { get; set; }

        /// <summary>
        /// Calculated completion percentage (0-100). Returns 0 when Max is zero.
        /// </summary>
        public double Percent => Max > 0 ? (double)Value / Max * 100 : 0;
    }
}
