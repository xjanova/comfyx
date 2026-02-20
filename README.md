# ComfyX - AI-Powered ComfyUI Workflow Studio

<div align="center">

**Portable C++ Desktop Application for Intelligent ComfyUI Workflow Generation**

[English](#overview) | [ภาษาไทย](#ภาพรวม)

[![Build](https://github.com/xjanova/comfyx/actions/workflows/build.yml/badge.svg)](https://github.com/xjanova/comfyx/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

</div>

---

## Overview

ComfyX is a portable desktop application that uses AI to generate ComfyUI workflows from natural language descriptions. Simply describe the image or video you want to create, and ComfyX will generate the complete node graph, validate it, and execute it on ComfyUI — all from a beautiful native interface.

### Key Features

- **AI Workflow Generation** — Describe your workflow in plain English or Thai, AI generates valid ComfyUI JSON
- **Multi-AI Support** — OpenAI GPT-4o, Anthropic Claude, Google Gemini, or Local AI (Qwen via llama.cpp)
- **Visual Node Graph** — Preview and edit generated workflows as interactive node graphs
- **Embedded ComfyUI** — Optionally bundle ComfyUI inside the app (no separate install needed)
- **External ComfyUI** — Connect to any running ComfyUI server via REST API/WebSocket
- **Portable** — Zero installation, runs from any folder (USB drive, network share)
- **Bilingual UI** — Full Thai and English interface
- **License System** — Integrated with [XmanStudio](https://github.com/xjanova/xmanstudio) licensing
- **Smart Themes** — Preset workflow templates (txt2img, img2img, inpainting, upscale, video)

---

## ภาพรวม

ComfyX คือโปรแกรมเดสก์ท็อปแบบพกพา ที่ใช้ AI สร้าง ComfyUI Workflow จากคำอธิบายภาษาธรรมชาติ เพียงพิมพ์อธิบายว่าต้องการสร้างภาพหรือวีดีโอแบบไหน ComfyX จะสร้างกราฟโหนดทั้งหมด ตรวจสอบความถูกต้อง และส่งไปรันบน ComfyUI ให้อัตโนมัติ

### คุณสมบัติหลัก

- **สร้าง Workflow ด้วย AI** — อธิบายเป็นภาษาไทยหรืออังกฤษ AI สร้าง JSON ให้ทันที
- **รองรับ AI หลายตัว** — OpenAI, Claude, Gemini หรือ AI ในเครื่อง (Qwen ผ่าน llama.cpp)
- **กราฟโหนดแบบ Visual** — ดูและแก้ไข Workflow เป็นกราฟโหนดแบบโต้ตอบได้
- **ฝัง ComfyUI ในตัว** — ไม่ต้องติดตั้ง ComfyUI แยก (optional)
- **เชื่อมต่อภายนอก** — เชื่อมต่อ ComfyUI server ที่รันอยู่แล้วได้
- **Portable** — ไม่ต้องติดตั้ง รันจาก folder ไหนก็ได้
- **สองภาษา** — UI ภาษาไทยและอังกฤษครบ
- **ระบบไลเซนส์** — เชื่อมต่อกับ [XmanStudio](https://github.com/xjanova/xmanstudio)

---

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│                   ComfyX (C++) - Portable                │
│                                                          │
│  ┌──────────┐  ┌──────────┐  ┌───────────────────┐      │
│  │ AI Chat  │  │  Node    │  │ Workflow Preview  │      │
│  │  Panel   │  │  Graph   │  │   & History       │      │
│  └────┬─────┘  └────┬─────┘  └────────┬──────────┘      │
│       └──────────────┴─────────────────┘                 │
│  Core: AI Manager | ComfyUI Connector | License Client   │
│  Bundled: llama.cpp + Python Embedded + ComfyUI          │
└──────────────┬──────────────┬────────────────┬───────────┘
               │              │                │
       Cloud AI APIs    ComfyUI Server    XmanStudio
      (OpenAI/Claude/   (Embedded or      License API
       Gemini)           External)
```

---

## Quick Start

### Download (Portable)
1. Download the latest release from [Releases](https://github.com/xjanova/comfyx/releases)
2. Extract the ZIP to any folder
3. Run `ComfyX.exe`
4. On first launch, choose **Embedded** mode to auto-install ComfyUI, or **External** mode to connect to an existing server

### Build from Source

```bash
# Clone with submodules
git clone --recursive https://github.com/xjanova/comfyx.git
cd comfyx

# Configure & Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel

# Run
./build/bin/ComfyX
```

### Build with Local AI Support
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCOMFYX_ENABLE_LOCAL_AI=ON
cmake --build build --config Release --parallel
```

---

## Technology Stack

| Component | Technology | License |
|-----------|-----------|---------|
| Language | C++17 | — |
| GUI | [Dear ImGui](https://github.com/ocornut/imgui) (docking) | MIT |
| Node Editor | [imgui-node-editor](https://github.com/thedmd/imgui-node-editor) | MIT |
| Local AI | [llama.cpp](https://github.com/ggml-org/llama.cpp) + Qwen2.5-Coder | MIT |
| HTTP | [cpp-httplib](https://github.com/yhirose/cpp-httplib) | MIT |
| WebSocket | [IXWebSocket](https://github.com/machinezone/IXWebSocket) | BSD |
| JSON | [nlohmann/json](https://github.com/nlohmann/json) | MIT |
| Windowing | [GLFW](https://github.com/glfw/glfw) | zlib |

---

## Project Structure

```
comfyx/
├── src/
│   ├── main.cpp                 # Entry point
│   ├── app/                     # Application core (Config, PortablePaths)
│   ├── ui/                      # ImGui panels (Chat, NodeGraph, Preview, Settings)
│   ├── ai/                      # AI engines (Cloud APIs, Local llama.cpp)
│   ├── comfyui/                 # ComfyUI integration (REST, WebSocket, Process)
│   ├── license/                 # XmanStudio license client
│   ├── i18n/                    # Thai/English translations
│   └── utils/                   # Utilities (Logger, FileUtils)
├── assets/                      # Fonts, icons, AI prompts
├── external/                    # Git submodules (imgui, glfw, etc.)
├── data/                        # [Portable] User config, workflows, cache
├── runtime/                     # [Portable] Embedded Python + ComfyUI
├── CMakeLists.txt
├── CMakePresets.json
└── .github/workflows/           # CI/CD pipelines
```

---

## Configuration

ComfyX stores all configuration in `data/config.json` (portable, relative to exe):

```json
{
  "language": "en",
  "theme": "midnight",
  "ai": {
    "activeProvider": "openai",
    "openaiApiKey": "sk-...",
    "claudeApiKey": "sk-ant-...",
    "geminiApiKey": "AI..."
  },
  "comfyui": {
    "mode": "external",
    "externalUrl": "http://127.0.0.1:8188"
  }
}
```

---

## License Tiers

| Feature | Demo (7 days) | Paid (Monthly/Yearly/Lifetime) |
|---------|:---:|:---:|
| Cloud AI Workflows | 3/day | Unlimited |
| Local AI (llama.cpp) | — | Yes |
| Workflow History | Last 5 | Unlimited |
| Custom Themes | — | Yes |
| Export Workflow | — | Yes |

Activate at: [xmanstudio.com](https://xmanstudio.com)

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development setup and guidelines.

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

<div align="center">
  Built with AI by <a href="https://github.com/xjanova">xjanova</a> / <a href="https://xmanstudio.com">XmanStudio</a>
</div>
