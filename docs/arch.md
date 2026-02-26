# Inferix — Full Architecture & Design
### A TUI for managing, configuring, and controlling llama.cpp server & models

---

## Directory Structure

```
src/
├── main.cpp
│
├── ui/
│   ├── app                        # Root screen, owns AppState, routes between views
│   ├── theme                      # Reads ThemeConfig, exposes FTXUI color/style helpers
│   └── panels/
│       ├── models                 # Browse & select models.
│       ├── model_info             # View model info, quantization, parameters, overall size, name, etc...
│       ├── presets                # List, create, clone, delete, activate presets per model.
│       ├── system_info            # Live system info: RAM, VRAM, CPU, backend from HWINFO & nvidia-smi.
│       ├── system_resources       # Live system resource info: CPU & GPU utilization, VRAM & RAM usage.
│       ├── server_status          # Server status, dynamic, reactive.
│       ├── load_parameters        # All load parameters selectable via radio buttons, drop-downs or sliders.
│       ├── inference_parameters   # All inference parameters selectable via radio buttons, drop-downs or sliders.
│       ├── resource_estimate      # Resource estimate using the selected mode + load parameters + system resources.
├── state/
│   ├── app_state                  # Runtime state: active model, server status, logs, system info
│   └── config_state               # Loaded config/settings, owns all config structs
│
├── config/
│   ├── config_manager             # Load/save all JSON config files via nlohmann
│   ├── model_config               # Struct: model path, context, batch size, rope, lora, etc.
│   ├── inference_config           # Struct: temp, top-p, top-k, penalties, seed, etc.
│   ├── server_config              # Struct: host, port, threads, gpu layers, etc.
│   ├── ui_config                  # Struct: active theme name, layout prefs
│   ├── theme_config               # Struct: full color palette, borders, highlight styles
│   └── advanced_config            # Struct: mmap, mlock, numa, flash attn, kv cache type, etc.
│
├── server/
│   ├── server_manager             # Spawns/kills llama.cpp process, owns its lifecycle
│   └── server_monitor             # Thread: watches process health, captures stdout/stderr
│
├── http/
│   ├── llama_client               # All httplib calls, single source of truth for API interaction
│   └── endpoints                  # URL constants, request builders, response parsers (nlohmann)
│
├── system/
│   ├── system_info                # Owns hwinfo queries, exposes static SystemInfo struct
│   ├── nvidia_monitor             # Spawns nvidia-smi loop process, parses stdout, pushes to AppState
│   └── gpu_vendor                 # Detects GPU vendor at startup, determines monitoring strategy
│
└── workers/
    ├── request_worker             # Thread: one-shot requests (load, unload, model info, etc.)
    └── poll_worker                # Thread: periodic polling of llama.cpp health + API metrics
```

---

## System Info Strategy

Two separate sources, each with a clearly defined responsibility:

### hwinfo (static, queried once at startup)
Handles all hardware identity and static info — no polling needed, this data doesn't change at runtime.

| Data | Notes |
|---|---|
| GPU vendor, model name | Used to determine monitoring strategy |
| GPU total VRAM | Static capacity figure |
| CPU vendor, model, core count, frequency | Displayed on system screen |
| Total system RAM | Capacity figure |
| OS name, version, architecture | Displayed on system screen |

### nvidia-smi loop (dynamic, NVIDIA only)
`NvidiaMonitor` spawns `nvidia-smi --query-gpu=... --format=csv,noheader,nounits -l 1` as a child process and reads its stdout line by line on a dedicated thread. Each line is parsed and pushed into `AppState`.

| Data | Field |
|---|---|
| VRAM used / free / total | `memory.used`, `memory.free`, `memory.total` |
| GPU core utilization % | `utilization.gpu` |
| Memory bus utilization % | `utilization.memory` |
| GPU temperature °C | `temperature.gpu` |
| Fan speed % | `fan.speed` |
| Power draw / limit (Watts) | `power.draw`, `power.limit` |
| Performance state | `pstate` |
| Per-process VRAM usage | Separate `--query-compute-apps` call, shows llama.cpp process specifically |

`gpu_vendor` detects GPU vendor at startup via hwinfo and sets a flag in `AppState`. `NvidiaMonitor` only starts if vendor is NVIDIA. Non-NVIDIA users see total VRAM from hwinfo and a note that live monitoring is unavailable.

### llama.cpp API (runtime inference metrics, polled every ~2s)
`PollWorker` handles this separately from system info.

| Data | Endpoint |
|---|---|
| Server health / ready state | `GET /health` |
| Loaded model info | `GET /v1/models` |
| KV cache state, context usage | `GET /metrics` |
| Inference throughput (tokens/sec) | `GET /metrics` |
| Slot state (busy/idle) | `GET /slots` |
| Server config, model metadata | `GET /props` |

---

## Config Files (JSON via nlohmann)

```
~/.inferix/
├── models/
│   ├── registry.json              # Index of all known models + active preset per model
│   └── <model-id>/
│       ├── model.json             # Model-specific loading params
│       └── presets/
│           ├── default.json       # Always exists, protected from deletion
│           ├── creative.json      # User preset
│           └── precise.json       # User preset
├── server.json                    # Server launch parameters
├── ui.json                        # Layout and UI preferences
├── themes/
│   ├── default.json
│   ├── dracula.json
│   └── user_custom.json
└── advanced.json                  # Low-level llama.cpp flags
```

### registry.json
- Index of all registered models: id, display name, file path, date added
- Per-model active preset (defaults to `"default"` if untouched)

### model.json (per model)
- File path, context size, batch size, RoPE config, LoRA path + scale, system prompt, prompt template

### preset.json (per preset, per model)
- Temperature, Top-P, Top-K, Min-P, repeat penalty, seed, max tokens, Mirostat, grammar, stop strings

### Preset Rules
- `default.json` is created automatically on model registration with sane defaults
- `default` cannot be deleted or renamed
- Missing active preset on load → falls back to `default`, logs a warning
- Presets are model-scoped but can be copied to another model

---

## Dependencies

| Library | Purpose | Integration |
|---|---|---|
| **FTXUI** | TUI rendering and event loop | CMake subdirectory |
| **cpp-httplib** | HTTP client for llama.cpp API | Single header |
| **nlohmann/json** | JSON building and parsing | Single header |
| **lfreist/hwinfo** | Static hardware info (CPU, GPU, RAM, OS) | CMake subdirectory, link only `hwinfo::cpu hwinfo::gpu hwinfo::ram hwinfo::os` |

nvidia-smi is not a library dependency — it is a child process spawned at runtime, present on any system with NVIDIA drivers installed.

## Server Lifecycle

```
User triggers "Start Server"
        │
        ▼
ServerManager builds argv[]
  ← ServerConfig (host, port, threads, gpu layers)
  ← ModelConfig of active model (path, context, batch, rope, lora)
  ← AdvancedConfig (mmap, mlock, flash attn, etc.)
  ← any extra CLI args passthrough
        │
        ▼
Process spawned (fork/exec or popen)
        │
        ├── ServerMonitor thread → captures stdout/stderr → AppState::server_logs
        ├── PollWorker begins hitting /health every 2s → AppState::server_status
        └── On /health returning ready → AppState::server_status = READY
                                       → screen.PostEvent(Custom)

User triggers "Stop Server"
        │
        ▼
ServerManager sends SIGTERM
        └── ServerMonitor detects exit → AppState::server_status = STOPPED
```

---

## State Ownership

| State | Owned By | Written By | Protected By |
|---|---|---|---|
| Server status | AppState | PollWorker, ServerMonitor | atomic enum |
| Server logs | AppState | ServerMonitor | mutex |
| Static hardware info | AppState | system_info (once at startup) | read-only after init |
| Live GPU metrics | AppState | NvidiaMonitor | mutex |
| System RAM (live) | AppState | PollWorker | mutex |
| System metrics (RAM, VRAM, slots) | AppState | PollWorker | mutex |
| Loaded model info | AppState | RequestWorker (on-demand) | mutex |
| All configs/settings | ConfigState | ConfigManager, Settings UI | mutex |

---

## Key Design Rules

- **llama.cpp is the single source of truth** for all runtime and system data — no OS calls
- **ConfigManager** is the only thing that touches disk
- **ServerManager** is the only thing that spawns or kills the llama.cpp process
- **LlamaClient** is the only thing that touches httplib
- **PollWorker** is the only thing that runs periodic requests — UI never polls directly
- **AppState and ConfigState** are the only shared mutable data — always mutex-guarded
- Settings changes are staged in the UI and only committed on explicit "Apply" or "Save"
- Theme changes can be previewed live, committed on "Apply"
