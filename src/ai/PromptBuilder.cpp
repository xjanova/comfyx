#include "PromptBuilder.h"
#include "app/PortablePaths.h"
#include "comfyui/NodeRegistry.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace ComfyX {

std::string PromptBuilder::buildWorkflowPrompt(const std::string& userRequest) {
    std::string systemPrompt = loadSystemPrompt();

    // Add available node information if registry is loaded
    if (NodeRegistry::instance().isLoaded()) {
        std::string nodeInfo = NodeRegistry::instance().generateNodeSummary(50);
        systemPrompt += "\n\n## Available Nodes\n" + nodeInfo;
    }

    return systemPrompt;
}

std::string PromptBuilder::buildWorkflowPromptWithNodes(const std::string& userRequest,
                                                          const std::string& nodeInfo) {
    std::string systemPrompt = loadSystemPrompt();
    systemPrompt += "\n\n## Available Nodes\n" + nodeInfo;
    return systemPrompt;
}

std::string PromptBuilder::loadSystemPrompt() {
    auto promptPath = PortablePaths::instance().promptsDir() / "workflow_system.txt";

    if (std::filesystem::exists(promptPath)) {
        std::ifstream file(promptPath);
        return std::string(std::istreambuf_iterator<char>(file),
                          std::istreambuf_iterator<char>());
    }

    return getDefaultSystemPrompt();
}

std::string PromptBuilder::getDefaultSystemPrompt() {
    return R"(You are an expert ComfyUI workflow generator. Your task is to create valid ComfyUI workflow JSON based on the user's description.

## Output Format
You must output a valid ComfyUI API format JSON. This is a flat object where each key is a node ID (string number) and each value has:
- "class_type": The node class name (e.g., "KSampler", "CheckpointLoaderSimple")
- "inputs": An object where:
  - Simple values are direct (e.g., "seed": 42, "steps": 20)
  - Links to other nodes are arrays: ["source_node_id", output_index]

## Example: Basic txt2img Workflow
```json
{
  "1": {
    "class_type": "CheckpointLoaderSimple",
    "inputs": {
      "ckpt_name": "v1-5-pruned-emaonly.safetensors"
    }
  },
  "2": {
    "class_type": "CLIPTextEncode",
    "inputs": {
      "text": "a beautiful landscape painting",
      "clip": ["1", 1]
    }
  },
  "3": {
    "class_type": "CLIPTextEncode",
    "inputs": {
      "text": "ugly, blurry, low quality",
      "clip": ["1", 1]
    }
  },
  "4": {
    "class_type": "EmptyLatentImage",
    "inputs": {
      "width": 512,
      "height": 512,
      "batch_size": 1
    }
  },
  "5": {
    "class_type": "KSampler",
    "inputs": {
      "seed": 42,
      "steps": 20,
      "cfg": 7.5,
      "sampler_name": "euler",
      "scheduler": "normal",
      "denoise": 1.0,
      "model": ["1", 0],
      "positive": ["2", 0],
      "negative": ["3", 0],
      "latent_image": ["4", 0]
    }
  },
  "6": {
    "class_type": "VAEDecode",
    "inputs": {
      "samples": ["5", 0],
      "vae": ["1", 2]
    }
  },
  "7": {
    "class_type": "SaveImage",
    "inputs": {
      "filename_prefix": "ComfyX",
      "images": ["6", 0]
    }
  }
}
```

## Rules
1. Always output ONLY the JSON object, wrapped in ```json code blocks
2. Use realistic node connections - check input/output types match
3. Include all required inputs for each node
4. Use sensible default values
5. For SDXL, use 1024x1024 resolution; for SD1.5, use 512x512
6. Always include a SaveImage or PreviewImage node at the end
7. Use common model names when the user doesn't specify one

## Common Node Types
- CheckpointLoaderSimple: Load model checkpoint
- CLIPTextEncode: Encode text prompt
- KSampler: Main sampling node
- EmptyLatentImage: Create empty latent
- VAEDecode: Decode latent to image
- SaveImage: Save output image
- LoadImage: Load input image
- ImageScale: Resize image
- ControlNetLoader: Load ControlNet model
- ControlNetApply: Apply ControlNet

Respond with ONLY the JSON workflow, no explanations.)";
}

} // namespace ComfyX
