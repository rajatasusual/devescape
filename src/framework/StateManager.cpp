#include "framework/StateManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <chrono>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace devescape {

void GameState::addEvent(const std::string& event) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%dT%H:%M:%SZ") << ": " << event;
    eventLog.push_back(oss.str());
}

StateManager::StateManager(const std::string& checkpointDir) 
    : checkpointDirectory_(checkpointDir) {
    fs::create_directories(checkpointDir);
}

StateManager::~StateManager() = default;

std::string StateManager::generateSessionId() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << "session_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    return oss.str();
}

std::string StateManager::getCheckpointPath(const std::string& sessionId) const {
    return checkpointDirectory_ + "/" + sessionId + ".json";
}

std::string StateManager::serializeSession(const GameSession& session) const {
    json j;

    // Session metadata
    auto startTime = std::chrono::system_clock::to_time_t(session.metadata.startedAt);
    auto checkpointTime = std::chrono::system_clock::to_time_t(session.metadata.checkpointedAt);

    j["session"] = {
        {"id", session.metadata.id},
        {"room_name", session.metadata.roomName},
        {"player_name", session.metadata.playerName},
        {"started_at", std::ctime(&startTime)},
        {"checkpointed_at", std::ctime(&checkpointTime)},
        {"total_time_seconds", session.metadata.totalTimeSeconds},
        {"time_elapsed_seconds", session.metadata.timeElapsedSeconds},
        {"time_remaining_seconds", session.timeRemainingSeconds},
        {"status", session.metadata.status}
    };

    // Room state
    json puzzlesJson;
    for (const auto& [id, puzzle] : session.currentRoomState.puzzles) {
        puzzlesJson[id] = {
            {"status", puzzle.solved ? "solved" : (puzzle.locked ? "locked" : "in_progress")},
            {"completion_percent", puzzle.completionPercent},
            {"hints_used", puzzle.hintsUsed},
            {"wrong_attempts", puzzle.wrongAttempts},
            {"time_spent_seconds", puzzle.timeSpentSeconds},
            {"player_answer", puzzle.playerAnswer},
            {"correct_answer", puzzle.correctAnswer}
        };
    }

    j["room_state"] = {
        {"puzzles", puzzlesJson},
        {"inventory", session.currentRoomState.inventory},
        {"discovered_clues", session.currentRoomState.discoveredClues},
        {"event_log", session.currentRoomState.eventLog}
    };

    return j.dump(2);  // Pretty print with 2-space indent
}

bool StateManager::deserializeSession(const std::string& jsonData, GameSession& session) const {
    try {
        json j = json::parse(jsonData);

        session.metadata.id = j["session"]["id"];
        session.metadata.roomName = j["session"]["room_name"];
        session.metadata.playerName = j["session"]["player_name"];
        session.metadata.totalTimeSeconds = j["session"]["total_time_seconds"];
        session.metadata.timeElapsedSeconds = j["session"]["time_elapsed_seconds"];
        session.timeRemainingSeconds = j["session"]["time_remaining_seconds"];
        session.metadata.status = j["session"]["status"];

        // Deserialize puzzles
        for (auto& [id, puzzleJson] : j["room_state"]["puzzles"].items()) {
            PuzzleState puzzle;
            puzzle.id = id;
            std::string status = puzzleJson["status"];
            puzzle.solved = (status == "solved");
            puzzle.locked = (status == "locked");
            puzzle.completionPercent = puzzleJson["completion_percent"];
            puzzle.hintsUsed = puzzleJson["hints_used"];
            puzzle.wrongAttempts = puzzleJson["wrong_attempts"];
            puzzle.timeSpentSeconds = puzzleJson["time_spent_seconds"];
            puzzle.playerAnswer = puzzleJson["player_answer"];
            puzzle.correctAnswer = puzzleJson["correct_answer"];
            session.currentRoomState.puzzles[id] = puzzle;
        }

        session.currentRoomState.inventory = j["room_state"]["inventory"].get<std::map<std::string, std::string>>();
        session.currentRoomState.discoveredClues = j["room_state"]["discovered_clues"].get<std::vector<std::string>>();
        session.currentRoomState.eventLog = j["room_state"]["event_log"].get<std::vector<std::string>>();

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void StateManager::createAutoCheckpoint(const GameSession& session) {
    std::string path = getCheckpointPath(session.metadata.id);
    saveSession(session, path);
    lastCheckpointTime_ = std::chrono::steady_clock::now();
}

bool StateManager::loadAutoCheckpoint(GameSession& session) {
    auto checkpoints = listRecentCheckpoints(1);
    if (checkpoints.empty()) return false;

    return loadSession(session, getCheckpointPath(checkpoints[0]));
}

void StateManager::saveSession(const GameSession& session, const std::string& filename) {
    std::string jsonData = serializeSession(session);
    std::ofstream file(filename);
    if (file.is_open()) {
        file << jsonData;
        file.close();
    }
}

bool StateManager::loadSession(GameSession& session, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    return deserializeSession(buffer.str(), session);
}

bool StateManager::hasRecentCheckpoint() const {
    return !listRecentCheckpoints(1).empty();
}

std::vector<std::string> StateManager::listRecentCheckpoints(int maxCount) const {
    std::vector<std::string> checkpoints;

    try {
        for (const auto& entry : fs::directory_iterator(checkpointDirectory_)) {
            if (entry.path().extension() == ".json") {
                checkpoints.push_back(entry.path().stem().string());
            }
        }

        // Sort by modification time (most recent first)
        std::sort(checkpoints.begin(), checkpoints.end(), std::greater<std::string>());

        if (checkpoints.size() > static_cast<size_t>(maxCount)) {
            checkpoints.resize(maxCount);
        }
    } catch (...) {
        // Return empty on error
    }

    return checkpoints;
}

} // namespace devescape
