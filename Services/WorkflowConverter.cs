using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Text.Json.Nodes;
using ComfyX.Helpers;

namespace ComfyX.Services
{
    /// <summary>
    /// Converts between ComfyUI workflow formats:
    /// - API format (used for queue/prompt execution)
    /// - Web UI format (used by the ComfyUI web interface, includes layout info)
    /// </summary>
    public static class WorkflowConverter
    {
        /// <summary>
        /// Converts a ComfyUI web UI workflow JSON to API format.
        /// Web UI format has "nodes" array with "type", "widgets_values", etc.
        /// API format is a dict of node_id -> { class_type, inputs }.
        /// </summary>
        public static string WebToApi(string webWorkflowJson)
        {
            try
            {
                var doc = JsonNode.Parse(webWorkflowJson);
                if (doc == null) return null;

                // If it's already in API format (has numbered keys with class_type), return as-is
                if (doc is JsonObject obj && IsApiFormat(obj))
                    return webWorkflowJson;

                var nodes = doc["nodes"]?.AsArray();
                var links = doc["links"]?.AsArray();
                if (nodes == null) return null;

                // Build link lookup: link_id -> (source_node_id, source_slot)
                var linkMap = new Dictionary<long, (long nodeId, int slot)>();
                if (links != null)
                {
                    foreach (var link in links)
                    {
                        if (link == null) continue;
                        var arr = link.AsArray();
                        if (arr.Count >= 4)
                        {
                            long linkId = arr[0].GetValue<long>();
                            long srcNodeId = arr[1].GetValue<long>();
                            int srcSlot = arr[2].GetValue<int>();
                            linkMap[linkId] = (srcNodeId, srcSlot);
                        }
                    }
                }

                var apiWorkflow = new JsonObject();

                foreach (var node in nodes)
                {
                    if (node == null) continue;

                    string nodeId = node["id"]?.ToString();
                    string classType = node["type"]?.ToString();
                    if (string.IsNullOrEmpty(nodeId) || string.IsNullOrEmpty(classType))
                        continue;

                    var inputs = new JsonObject();
                    var widgetValues = node["widgets_values"]?.AsArray();
                    var nodeInputs = node["inputs"]?.AsArray();

                    // Map connected inputs (links)
                    if (nodeInputs != null)
                    {
                        int widgetIndex = 0;
                        foreach (var input in nodeInputs)
                        {
                            if (input == null) continue;
                            string inputName = input["name"]?.ToString();
                            if (string.IsNullOrEmpty(inputName)) continue;

                            var linkNode = input["link"];
                            if (linkNode != null && linkNode.GetValueKind() == JsonValueKind.Number)
                            {
                                long linkId = linkNode.GetValue<long>();
                                if (linkMap.TryGetValue(linkId, out var src))
                                {
                                    inputs[inputName] = new JsonArray(src.nodeId.ToString(), src.slot);
                                }
                            }
                            else if (input["widget"] != null)
                            {
                                // Widget input — get value from widgets_values
                                if (widgetValues != null && widgetIndex < widgetValues.Count)
                                {
                                    inputs[inputName] = widgetValues[widgetIndex]?.DeepClone();
                                }
                                widgetIndex++;
                            }
                        }
                    }

                    // Map widget values to inputs for non-connected widgets
                    if (widgetValues != null && nodeInputs != null)
                    {
                        int wIdx = 0;
                        foreach (var input in nodeInputs)
                        {
                            if (input == null) continue;
                            string inputName = input["name"]?.ToString();
                            if (string.IsNullOrEmpty(inputName)) continue;

                            var linkNode = input["link"];
                            bool isConnected = linkNode != null && linkNode.GetValueKind() == JsonValueKind.Number;

                            if (!isConnected && !inputs.ContainsKey(inputName))
                            {
                                if (wIdx < widgetValues.Count)
                                {
                                    inputs[inputName] = widgetValues[wIdx]?.DeepClone();
                                }
                                wIdx++;
                            }
                        }
                    }

                    var nodeObj = new JsonObject
                    {
                        ["class_type"] = classType,
                        ["inputs"] = inputs
                    };

                    apiWorkflow[nodeId] = nodeObj;
                }

                return apiWorkflow.ToJsonString(new JsonSerializerOptions { WriteIndented = true });
            }
            catch (Exception ex)
            {
                Logger.Error($"Workflow conversion failed: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Checks if a JSON object is already in ComfyUI API format.
        /// API format has numbered string keys with objects containing "class_type".
        /// </summary>
        public static bool IsApiFormat(JsonObject obj)
        {
            foreach (var kvp in obj)
            {
                if (kvp.Value is JsonObject nodeObj && nodeObj.ContainsKey("class_type"))
                    return true;
            }
            return false;
        }

        /// <summary>
        /// Pretty-prints a workflow JSON string.
        /// </summary>
        public static string PrettyPrint(string json)
        {
            try
            {
                var doc = JsonDocument.Parse(json);
                return JsonSerializer.Serialize(doc, new JsonSerializerOptions { WriteIndented = true });
            }
            catch
            {
                return json;
            }
        }
    }
}
