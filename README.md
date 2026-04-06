# Workbench
A TUI frontend for llama.cpp, built with FTXUI, nlohmann/json, cpp-httplib.

## Building

Builds are done inside Docker using `docker-compose run`.

### Linux (default)

```bash
docker-compose run --rm cpp-app ./build.sh
```

### Windows (cross-compile)

```bash
docker-compose run --rm cpp-app ./build.sh --windows
```

### Both Linux and Windows

```bash
docker-compose run --rm cpp-app ./build.sh --all
```

### Clean build

Add `--clean` (or `-c`) to delete build directories before building:

```bash
docker-compose run --rm cpp-app ./build.sh --clean --all
```

### Build tests and coverage for Liunx

```bash
docker-compose run --rm cpp-app ./build.sh --test
```

### Options

| Flag | Short | Description |
|------|-------|-------------|
| `--clean` | `-c` | Delete build directories before building |
| `--windows` | `-w` | Cross-compile for Windows |
| `--all` | `-a` | Build for both Linux and Windows |
| `--test` | `-t` | Builds tests and coverage in Linux |
| `--help` | | Show usage information |
