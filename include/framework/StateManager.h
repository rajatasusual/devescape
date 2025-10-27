#pragma once

#include "framework/DataTypes.h"
#include <string>
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

struct GameSession {
    SessionMetadata metadata;
    GameState currentRoomState;
    int timeRemainingSeconds;
};

class DEVESCAPE_API StateManager {
public:
    StateManager(const std::string& checkpointDir);
    ~StateManager();

    // Checkpoint management
    void createAutoCheckpoint(const GameSession& session);
    bool loadAutoCheckpoint(GameSession& session);

    // Session management
    void saveSession(const GameSession& session, const std::string& filename);
    bool loadSession(GameSession& session, const std::string& filename);

    // Session queries
    bool hasRecentCheckpoint() const;
    std::vector<std::string> listRecentCheckpoints(int maxCount) const;

    // Serialization
    std::string serializeSession(const GameSession& session) const;
    bool deserializeSession(const std::string& jsonData, GameSession& session) const;

private:
    std::string checkpointDirectory_;
    std::chrono::steady_clock::time_point lastCheckpointTime_;

    std::string getCheckpointPath(const std::string& sessionId) const;
    std::string generateSessionId() const;
};

} // namespace devescape
