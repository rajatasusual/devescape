#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

namespace devescape {

// Forward declarations
class TerminalRenderer;
class AudioManager;
class StateManager;

// Color types for terminal rendering
enum class ColorType {
    DEFAULT,
    ACCENT,      // Cyan
    ALERT,       // Red
    SUCCESS,     // Green
    WARNING,     // Orange
    STATUS,      // Dim white
    ERROR_COLOR, // Magenta
    PENDING      // Yellow
};

// Puzzle types
enum class PuzzleType {
    LOG_ANALYSIS,
    METRICS_NAVIGATION,
    ALGORITHM,
    CONFIGURATION,
    DEBUGGING,
    CUSTOM
};

// Pressure levels for timer
enum class PressureLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

// Theme types for audio
enum class ThemeType {
    AMBIENT,
    FOCUS,
    COMPLEX,
    CRISIS,
    VICTORY,
    FAILURE
};

// Process result from input handling
struct ProcessResult {
    std::string outputText;
    bool sessionEnded = false;
    bool success = false;
    bool invalidCommand = false;

    ProcessResult() = default;
};

// Puzzle state
struct PuzzleState {
    std::string id;
    std::string title;
    bool solved = false;
    int completionPercent = 0;
    int hintsUsed = 0;
    int wrongAttempts = 0;
    int timeSpentSeconds = 0;
    std::string playerAnswer;
    std::string correctAnswer;
    bool locked = true;
};

// Game state
struct GameState {
    std::map<std::string, PuzzleState> puzzles;
    std::map<std::string, std::string> inventory;
    std::vector<std::string> discoveredClues;
    int completedPuzzleCount = 0;
    std::vector<std::string> eventLog;

    void addEvent(const std::string& event);
};

// Session metadata
struct SessionMetadata {
    std::string id;
    std::string roomName;
    std::string playerName;
    std::chrono::system_clock::time_point startedAt;
    std::chrono::system_clock::time_point checkpointedAt;
    int totalTimeSeconds;
    int timeElapsedSeconds = 0;
    std::string status; // "in_progress", "completed", "failed", "abandoned"
};

// Framework context passed to plugins
struct FrameworkContext {
    AudioManager* audioManager = nullptr;
    StateManager* stateManager = nullptr;
    std::string dataDirectory;
    std::string checkpointDirectory;
};

} // namespace devescape
