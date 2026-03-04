# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Inferix is a TUI (Terminal User Interface) frontend for llama.cpp, built with C++20, FTXUI, nlohmann/json, and cpp-httplib. It provides real-time system resource monitoring including CPU, RAM, and NVIDIA GPU statistics.

## Build System

### Building (Inside Docker)

All builds are done inside Docker using `docker-compose run`:

```bash
# Linux (default)
docker-compose run --rm cpp-app ./build.sh

# Windows (cross-compile using mingw-w64)
docker-compose run --rm cpp-app ./build.sh --windows

# Both Linux and Windows (parallel build)
docker-compose run --rm cpp-app ./build.sh --all

# Clean build
docker-compose run --rm cpp-app ./build.sh --clean --all
```

### Build Architecture

- **CMake** manages dependencies via FetchContent (FTXUI v6.1.9)
- **cpp-httplib** is included as a header-only interface library
- Platform-specific source files are auto-selected by CMake based on `CMAKE_SYSTEM_NAME`
- Build outputs: `build_linux/Inferix` or `build_win/Inferix`
- Code formatting: `clang-format` runs automatically on `src/` and `include/` before each build

## Architecture

### Core Design Patterns

1. **Singleton Monitors**: `CpuMonitor`, `MemoryMonitor`, and `GpuMonitor` are thread-safe singletons that poll hardware stats
2. **Platform Dispatch**: Monitors use private `update_linux()`, `update_windows()`, `update_unknown()` methods for platform-specific implementations
3. **Stateless Panels**: UI panels (e.g., `SystemResourcesPanel`) are pure rendering classes with only static methods
4. **Background Polling**: `SystemMonitorRunner` manages a background thread that polls monitors every 250ms and triggers FTXUI redraws

### Directory Structure

```
src/
  main.cpp                    # Entry point, initializes FTXUI screen and monitors
  app.cpp                     # (placeholder - currently empty)
  system/
    cpu.cpp                   # CpuMonitor implementation (platform dispatch)
    cpu_linux.cpp             # Linux-specific CPU stats via /proc/stat
    cpu_windows.cpp           # Windows-specific CPU stats via Win32 API
    cpu_monitor.cpp           # Monitor runner integration
    gpu_monitor.cpp           # NVIDIA GPU stats via nvidia-smi parsing
    ram_linux.cpp             # Linux-specific RAM stats via /proc/meminfo
    ram_windows.cpp           # Windows-specific RAM stats via Win32 API
    ram_monitor.cpp           # MemoryMonitor implementation (platform dispatch)
    system_info*.cpp          # System info utilities
    system_monitor_runner.cpp # Background polling thread management
  ui/panels/
    system_resources_panel.cpp # Main resource gauge and table rendering
    system_info_panel.cpp      # Static system information display
    server_info_panel.cpp      # llama.cpp server status panel

include/
  models/
    memory_stats.h             # MemoryStats struct (MiB-based, for RAM/VRAM)
    processor_stats.h          # ProcessorStats struct (usage percentage)
  system/
    cpu_monitor.h              # Thread-safe CPU singleton
    gpu_monitor.h              # NVIDIA GPU singleton (nvidia-smi based)
    ram_monitor.h              # Thread-safe RAM singleton
    system_monitor_runner.h    # Background polling thread
  panels/
    system_resources_panel.h   # Main resource display panel
    system_info_panel.h        # Static system info panel
    server_info_panel.h        # Server status panel
  server/
    httplib.h                  # cpp-httplib header-only library
```

### Data Flow

1. `main()` creates `ScreenInteractive` and initializes monitors
2. `SystemMonitorRunner` starts a background thread that:
   - Calls `CpuMonitor::instance().update()` every 250ms
   - Calls `MemoryMonitor::instance().update()` every 250ms
   - Calls `GpuMonitor::instance().update()` every 250ms
   - Triggers FTXUI screen redraw via `PostEvent`
3. `SystemResourcesPanel::Render()` queries monitor singletons for latest stats and composes FTXUI elements
4. Platform-specific implementations read from OS sources:
   - Linux: `/proc/stat`, `/proc/meminfo`
   - Windows: Win32 API (GetProcessTimes, GlobalMemoryStatusEx)
   - GPU: `nvidia-smi --query-gpu=... --format=csv`

### Key Constraints

- All monitor singletons use `std::mutex` for thread-safe stat access
- Monitors are **not copyable** (deleted copy constructors/assignment operators)
- `SystemMonitorRunner` owns its thread and must be destroyed before FTXUI screen
- GPU monitoring requires `nvidia-smi` to be available in PATH
- Cross-compilation for Windows requires `x86_64-w64-mingw32-gcc-posix` compiler
