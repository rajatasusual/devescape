#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

#if defined(_WIN32)
  #if defined(DEVESCAPE_EXPORTS)
    #define DEVESCAPE_API __declspec(dllexport)
  #else
    #define DEVESCAPE_API __declspec(dllimport)
  #endif
#else
  #define DEVESCAPE_API
#endif

namespace devescape {

class TerminalRenderer;
class AudioManager;
class StateManager;

enum class ColorType {
    DEFAULT,
    ACCENT,
    ALERT,
    SUCCESS,
    WARNING,
    STATUS,
    ERROR_COLOR,
    PENDING
};

enum class PuzzleType {
    LOG_ANALYSIS,
    METRICS_NAVIGATION,
    ALGORITHM,
    CONFIGURATION,
    DEBUGGING,
    CUSTOM
};

enum class PressureLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

enum class ThemeType {
    AMBIENT,
    FOCUS,
    COMPLEX,
    CRISIS,
    VICTORY,
    FAILURE
};

struct DEVESCAPE_API ProcessResult {
    std::string outputText;
    bool sessionEnded = false;
    bool success = false;
    bool invalidCommand = false;
};

struct DEVESCAPE_API PuzzleState {
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

struct DEVESCAPE_API GameState {
    std::map<std::string, PuzzleState> puzzles;
    std::map<std::string, std::string> inventory;
    std::vector<std::string> discoveredClues;
    int completedPuzzleCount = 0;
    std::vector<std::string> eventLog;

    void addEvent(const std::string& event);
};

struct DEVESCAPE_API SessionMetadata {
    std::string id;
    std::string roomName;
    std::string playerName;
    std::chrono::system_clock::time_point startedAt;
    std::chrono::system_clock::time_point checkpointedAt;
    int totalTimeSeconds;
    int timeElapsedSeconds = 0;
    std::string status;
};

struct DEVESCAPE_API FrameworkContext {
    AudioManager* audioManager = nullptr;
    StateManager* stateManager = nullptr;
    std::string dataDirectory;
    std::string checkpointDirectory;
};

} // namespace devescape
