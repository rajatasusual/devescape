# DevEscape Framework

**Version**: 1.0.0  
**Developer-Centric Escape Room Platform**

## Overview

DevEscape is a terminal-native escape room platform designed for software developers. It combines psychological immersion with real-world engineering challenges through:

- **Plugin Architecture**: Extensible escape room system via shared libraries
- **8-bit Audio**: Dynamic chiptune synthesis that adapts to game state
- **Terminal UI**: Pure ASCII/ANSI rendering with no external graphics
- **State Persistence**: Auto-save and session resume
- **No-Exit Mechanism**: Soft lock-in for psychological commitment

## Building

### Requirements

- **C++17** compiler (GCC 8+, Clang 10+, MSVC 19.14+)
- **CMake** 3.16+
- **SDL2** (for audio)
- **Boost** (filesystem)
- **nlohmann/json** library

### Ubuntu/Debian

```bash
sudo apt-get install build-essential cmake libsdl2-dev libboost-filesystem-dev nlohmann-json3-dev
```

### macOS

```bash
brew install cmake sdl2 boost nlohmann-json
```

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Running

```bash
./devescape
```

The framework will:
1. Scan for plugin files in `./plugins/`
2. Display available escape rooms
3. Allow you to select and play

## First Escape Room: Production Incident

A database connection pool crisis simulation:
- **Duration**: 45 minutes
- **Difficulty**: Intermediate
- **Skills**: Log analysis, metrics navigation, capacity planning, deployment

### Puzzle Flow

1. **Alert Analysis**: Identify root cause from system alerts
2. **Metrics Navigation**: Navigate nested metrics to find bottleneck
3. **Pool Optimization**: Calculate optimal connection pool size using Little's Law
4. **Config Deployment**: Deploy the fix through proper change control

## Project Structure

```
devescape/
├── src/
│   ├── main.cpp                      # Application entry point
│   └── framework/                    # Core framework
│       ├── PluginManager.cpp         # Dynamic library loading
│       ├── AudioManager.cpp          # 8-bit synthesis
│       ├── StateManager.cpp          # Persistence
│       ├── TerminalRenderer.cpp      # UI rendering
│       ├── TimerSystem.cpp           # Countdown timer
│       └── HintSystem.cpp            # Progressive hints
├── include/
│   └── framework/                    # Public headers
│       ├── IEscapeRoom.h            # Plugin interface
│       ├── DataTypes.h              # Shared types
│       └── ...
├── plugins/
│   └── production_incident/         # First escape room
│       ├── ProductionIncidentRoom.cpp
│       └── puzzles/
└── data/
    └── checkpoints/                 # Auto-save files
```

## Creating Custom Escape Rooms

1. Implement `IEscapeRoom` interface
2. Export required functions: `createRoom()`, `destroyRoom()`, etc.
3. Build as shared library (.so/.dll)
4. Place in `./plugins/` directory

See `plugins/production_incident/` for a complete example.

## Architecture

### Core Components

- **Plugin Manager**: Discovers and loads escape room plugins dynamically
- **Audio Manager**: NES-style 4-channel chiptune synthesizer
- **State Manager**: JSON-based session serialization
- **Terminal Renderer**: ANSI color + ASCII art rendering
- **Timer System**: Real-time countdown with pressure escalation
- **Hint System**: 3-tier progressive hint delivery

### Plugin Interface

All escape rooms implement `IEscapeRoom`:

```cpp
class IEscapeRoom {
    virtual void initialize(const FrameworkContext& context) = 0;
    virtual ProcessResult processInput(const std::string& command) = 0;
    virtual void render(TerminalRenderer& renderer) const = 0;
    virtual void update(float deltaTimeSeconds) = 0;
    // ... plus state management, hints, metadata
};
```

## Features

### No-Exit Mechanism
Players cannot casually exit - they must explicitly surrender (type "I SURRENDER" three times), creating psychological commitment similar to physical escape rooms.

### Dynamic Audio
8-bit chiptune music adapts in real-time:
- Tempo increases as time runs low
- Themes change per puzzle type
- Tension builds with player struggles

### Auto-Save
Session state automatically checkpointed every 30 seconds. Resume on restart.

### Progressive Hints
3-tier hint system:
1. **Nudge**: Gentle guidance
2. **Clarify**: More specific direction
3. **Solution**: Complete answer

Hints unlock based on failed attempts or time pressure.

## Performance

- **60 FPS** rendering
- **<100ms** input latency
- **Real-time audio** synthesis
- **Minimal memory** footprint (~50MB)

## Platforms

- ✅ Linux (tested on Ubuntu 20.04+)
- ✅ macOS (tested on 10.15+)
- ✅ Windows (tested on Windows 10+)

## License

MIT License - See LICENSE file for details

## Credits

Developed as part of the DevEscape Framework project.  
Inspired by real-world software engineering challenges and escape room psychology.
