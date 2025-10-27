#include "framework/IEscapeRoom.h"
#include "framework/TerminalRenderer.h"
#include "framework/AudioManager.h"
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace production_incident {

// Forward declarations
class AlertAnalysisPuzzle;
class MetricsNavigationPuzzle;
class PoolOptimizationPuzzle;
class ConfigDeploymentPuzzle;

class ProductionIncidentRoom : public devescape::IEscapeRoom {
public:
    ProductionIncidentRoom();
    ~ProductionIncidentRoom() override;

    // Metadata
    std::string getName() const override { return "Production Incident"; }
    std::string getVersion() const override { return "1.0.0"; }
    std::string getAuthor() const override { return "DevEscape Team"; }
    std::string getDescription() const override {
        return "Payment service is down. Database connections exhausted. "
               "You have 45 minutes before the CEO demands answers.";
    }
    uint32_t getTotalDurationSeconds() const override { return 45 * 60; }

    // Lifecycle
    void initialize(const devescape::FrameworkContext& context) override;
    void cleanup() override;

    // State
    devescape::GameState getCurrentState() const override { return gameState_; }
    bool isCompleted() const override;
    bool isFailed() const override { return false; }  // Can't fail, only timeout
    int getCompletionPercentage() const override;

    // Interaction
    devescape::ProcessResult processInput(const std::string& command) override;
    void render(devescape::TerminalRenderer& renderer) const override;
    void update(float deltaTimeSeconds) override;

    // Persistence
    std::string serializeState() const override;
    bool deserializeState(const std::string& data) override;

    // Hints
    std::string getHint(int hintLevel) override;
    int getMaxHintLevel() const override { return 3; }
    bool canUseHint() const override { return currentHintLevel_ < getMaxHintLevel(); }

    // Timeout
    void onSessionTimeout() override;

private:
    void setupPuzzles();
    void unlockNextPuzzle();
    std::string getCurrentPuzzleId() const;

    devescape::FrameworkContext context_;
    devescape::GameState gameState_;
    int currentHintLevel_;
    float timeInCurrentPuzzle_;

    enum class Phase {
        ALERT_ANALYSIS,
        METRICS_NAVIGATION,
        POOL_OPTIMIZATION,
        CONFIG_DEPLOYMENT,
        COMPLETED
    };

    Phase currentPhase_;
};

ProductionIncidentRoom::ProductionIncidentRoom()
    : currentHintLevel_(0)
    , timeInCurrentPuzzle_(0.0f)
    , currentPhase_(Phase::ALERT_ANALYSIS) {
}

ProductionIncidentRoom::~ProductionIncidentRoom() = default;

void ProductionIncidentRoom::initialize(const devescape::FrameworkContext& context) {
    context_ = context;
    setupPuzzles();

    gameState_.addEvent("Production incident started");
    gameState_.addEvent("Payment service reporting critical errors");

    // Set initial music
    if (context_.audioManager) {
        context_.audioManager->playTheme("crisis", devescape::ThemeType::CRISIS);
    }
}

void ProductionIncidentRoom::cleanup() {
    // Nothing to clean up
}

void ProductionIncidentRoom::setupPuzzles() {
    // Initialize puzzle states
    devescape::PuzzleState alertPuzzle;
    alertPuzzle.id = "alert_analysis";
    alertPuzzle.title = "Alert Analysis";
    alertPuzzle.locked = false;
    gameState_.puzzles["alert_analysis"] = alertPuzzle;

    devescape::PuzzleState metricsPuzzle;
    metricsPuzzle.id = "metrics_navigation";
    metricsPuzzle.title = "Metrics Navigation";
    metricsPuzzle.locked = true;
    gameState_.puzzles["metrics_navigation"] = metricsPuzzle;

    devescape::PuzzleState poolPuzzle;
    poolPuzzle.id = "pool_optimization";
    poolPuzzle.title = "Pool Optimization";
    poolPuzzle.locked = true;
    gameState_.puzzles["pool_optimization"] = poolPuzzle;

    devescape::PuzzleState deployPuzzle;
    deployPuzzle.id = "config_deployment";
    deployPuzzle.title = "Configuration Deployment";
    deployPuzzle.locked = true;
    gameState_.puzzles["config_deployment"] = deployPuzzle;
}

bool ProductionIncidentRoom::isCompleted() const {
    return currentPhase_ == Phase::COMPLETED;
}

int ProductionIncidentRoom::getCompletionPercentage() const {
    int completed = 0;
    for (const auto& [id, puzzle] : gameState_.puzzles) {
        if (puzzle.solved) completed++;
    }
    return (completed * 100) / 4;
}

devescape::ProcessResult ProductionIncidentRoom::processInput(const std::string& command) {
    devescape::ProcessResult result;

    // Parse command
    if (command == "help") {
        result.outputText = "Commands: examine logs, navigate metrics, calculate pool, deploy config, hint";
        return result;
    }

    if (command == "hint") {
        result.outputText = getHint(currentHintLevel_);
        currentHintLevel_++;
        return result;
    }

    // Route to appropriate puzzle handler based on current phase
    switch (currentPhase_) {
        case Phase::ALERT_ANALYSIS:
            if (command.find("examine") != std::string::npos || 
                command.find("filter") != std::string::npos ||
                command.find("identify") != std::string::npos) {

                if (command.find("database") != std::string::npos) {
                    gameState_.puzzles["alert_analysis"].solved = true;
                    gameState_.addEvent("Identified database as root cause");
                    result.outputText = "Correct! Database connection failures are the root cause.";
                    currentPhase_ = Phase::METRICS_NAVIGATION;
                    gameState_.puzzles["metrics_navigation"].locked = false;

                    if (context_.audioManager) {
                        context_.audioManager->playTheme("focus", devescape::ThemeType::FOCUS);
                    }
                } else {
                    gameState_.puzzles["alert_analysis"].wrongAttempts++;
                    result.outputText = "Not quite. Look for common patterns in the errors.";
                }
            } else {
                result.outputText = "Try: examine logs, filter logs ERROR, identify root_cause";
            }
            break;

        case Phase::METRICS_NAVIGATION:
            if (command.find("navigate") != std::string::npos) {
                if (command.find("database") != std::string::npos && 
                    command.find("pool") != std::string::npos) {
                    gameState_.puzzles["metrics_navigation"].solved = true;
                    gameState_.addEvent("Discovered connection pool exhaustion");
                    result.outputText = "Connection Pool Status:\n"
                                      "  Active: 20/20 (EXHAUSTED!)\n"
                                      "  Waiting: 847 requests\n"
                                      "  Avg wait time: 15000ms\n";
                    currentPhase_ = Phase::POOL_OPTIMIZATION;
                    gameState_.puzzles["pool_optimization"].locked = false;
                } else {
                    result.outputText = "Navigate deeper: try 'navigate metrics payment-api dependencies database connection_pool'";
                }
            } else {
                result.outputText = "Use: navigate metrics [path]";
            }
            break;

        case Phase::POOL_OPTIMIZATION:
            if (command.find("calculate") != std::string::npos || 
                command.find("submit") != std::string::npos) {

                // Extract number from command
                size_t pos = command.find_first_of("0123456789");
                if (pos != std::string::npos) {
                    int value = std::stoi(command.substr(pos));
                    if (value >= 50 && value <= 75) {
                        gameState_.puzzles["pool_optimization"].solved = true;
                        gameState_.puzzles["pool_optimization"].playerAnswer = std::to_string(value);
                        gameState_.addEvent("Calculated optimal pool size: " + std::to_string(value));
                        result.outputText = "Correct! Pool size of " + std::to_string(value) + 
                                          " will handle the load.";
                        currentPhase_ = Phase::CONFIG_DEPLOYMENT;
                        gameState_.puzzles["config_deployment"].locked = false;
                    } else {
                        gameState_.puzzles["pool_optimization"].wrongAttempts++;
                        result.outputText = "That won't handle the load. Use Little's Law: "
                                          "L = λ × W × safety_factor";
                    }
                } else {
                    result.outputText = "Use: submit solution [number]";
                }
            } else {
                result.outputText = "Calculate the optimal pool size. Current: 20, Request rate: 100/sec, "
                                  "Service time: 0.5sec";
            }
            break;

        case Phase::CONFIG_DEPLOYMENT:
            if (command == "deploy config" || command == "deploy db_pool_size 60") {
                gameState_.puzzles["config_deployment"].solved = true;
                gameState_.addEvent("Configuration deployed successfully");
                result.outputText = "Deploying configuration...\n"
                                  "Pool size: 20 → 60\n"
                                  "Monitoring metrics...\n"
                                  "Response time: 2847ms → 142ms\n"
                                  "Error rate: 23.4% → 0%\n"
                                  "INCIDENT RESOLVED!";
                result.sessionEnded = true;
                result.success = true;
                currentPhase_ = Phase::COMPLETED;

                if (context_.audioManager) {
                    context_.audioManager->playTheme("victory", devescape::ThemeType::VICTORY);
                }
            } else {
                result.outputText = "Use: deploy config";
            }
            break;

        case Phase::COMPLETED:
            result.outputText = "Incident resolved!";
            result.sessionEnded = true;
            result.success = true;
            break;
    }

    return result;
}

void ProductionIncidentRoom::render(devescape::TerminalRenderer& renderer) const {
    renderer.clearScreen();

    // Header
    renderer.drawBox(0, 0, 80, 3, "PRODUCTION INCIDENT");
    renderer.drawText(5, 1, "Payment Service DOWN | Database Connection Pool EXHAUSTED", 
                     devescape::ColorType::ALERT, true);

    // Progress
    renderer.drawProgressBar(5, 4, getCompletionPercentage() / 100.0f, 60, 
                            devescape::ColorType::ACCENT);
    renderer.drawText(67, 4, std::to_string(getCompletionPercentage()) + "%", 
                     devescape::ColorType::STATUS);

    // Current puzzle
    renderer.drawBox(0, 6, 80, 15, getCurrentPuzzleId().c_str());

    switch (currentPhase_) {
        case Phase::ALERT_ANALYSIS:
            renderer.drawText(3, 8, "[CRITICAL] Payment API returned 500", devescape::ColorType::ALERT);
            renderer.drawText(3, 9, "[ERROR] Connection timeout: db-prod-01", devescape::ColorType::ERROR_COLOR);
            renderer.drawText(3, 10, "[ERROR] Circuit breaker opened for database", devescape::ColorType::ERROR_COLOR);
            renderer.drawText(3, 12, "Commands: examine logs, filter logs ERROR, identify root_cause", 
                            devescape::ColorType::STATUS);
            break;

        case Phase::METRICS_NAVIGATION:
            renderer.drawText(3, 8, "Navigate: services → payment-api → dependencies → database", 
                            devescape::ColorType::ACCENT);
            renderer.drawText(3, 10, "Hint: Look for connection_pool metrics", devescape::ColorType::WARNING);
            break;

        case Phase::POOL_OPTIMIZATION:
            renderer.drawText(3, 8, "Current pool size: 20 connections", devescape::ColorType::STATUS);
            renderer.drawText(3, 9, "Request rate: 100 req/sec", devescape::ColorType::STATUS);
            renderer.drawText(3, 10, "Service time: 0.5 seconds", devescape::ColorType::STATUS);
            renderer.drawText(3, 12, "Calculate optimal pool size using Little's Law", 
                            devescape::ColorType::ACCENT);
            break;

        case Phase::CONFIG_DEPLOYMENT:
            renderer.drawText(3, 8, "Ready to deploy new configuration", devescape::ColorType::SUCCESS);
            renderer.drawText(3, 9, "New pool size: 60 connections", devescape::ColorType::ACCENT);
            renderer.drawText(3, 11, "Command: deploy config", devescape::ColorType::WARNING);
            break;

        case Phase::COMPLETED:
            renderer.drawText(3, 8, "INCIDENT RESOLVED!", devescape::ColorType::SUCCESS, true);
            renderer.drawText(3, 10, "System back online. Well done!", devescape::ColorType::SUCCESS);
            break;
    }

    // Command prompt
    renderer.drawText(0, 22, "> _", devescape::ColorType::ACCENT);
}

void ProductionIncidentRoom::update(float deltaTimeSeconds) {
    timeInCurrentPuzzle_ += deltaTimeSeconds;

    // Update current puzzle time
    std::string currentPuzzle = getCurrentPuzzleId();
    if (gameState_.puzzles.count(currentPuzzle)) {
        gameState_.puzzles[currentPuzzle].timeSpentSeconds = static_cast<int>(timeInCurrentPuzzle_);
    }
}

std::string ProductionIncidentRoom::serializeState() const {
    json j;
    j["phase"] = static_cast<int>(currentPhase_);
    j["hint_level"] = currentHintLevel_;
    return j.dump();
}

bool ProductionIncidentRoom::deserializeState(const std::string& data) {
    try {
        json j = json::parse(data);
        currentPhase_ = static_cast<Phase>(j["phase"].get<int>());
        currentHintLevel_ = j["hint_level"];
        return true;
    } catch (...) {
        return false;
    }
}

std::string ProductionIncidentRoom::getHint(int hintLevel) {
    switch (currentPhase_) {
        case Phase::ALERT_ANALYSIS:
            if (hintLevel == 0) return "Look for [ERROR] level entries.";
            if (hintLevel == 1) return "Which component appears in multiple error messages?";
            return "The database connection timeouts are blocking all requests.";

        case Phase::METRICS_NAVIGATION:
            if (hintLevel == 0) return "Navigate through: services → payment-api → dependencies";
            if (hintLevel == 1) return "Look at the database connection_pool metrics";
            return "Check: active connections vs max connections";

        case Phase::POOL_OPTIMIZATION:
            if (hintLevel == 0) return "Use Little's Law: L = λ × W";
            if (hintLevel == 1) return "L = 100 req/sec × 0.5 sec × 1.2 (safety factor)";
            return "Answer: 60 connections (100 × 0.5 × 1.2 = 60)";

        case Phase::CONFIG_DEPLOYMENT:
            return "Type: deploy config";

        default:
            return "No hints available.";
    }
}

void ProductionIncidentRoom::onSessionTimeout() {
    gameState_.addEvent("Session timeout - incident unresolved");
}

std::string ProductionIncidentRoom::getCurrentPuzzleId() const {
    switch (currentPhase_) {
        case Phase::ALERT_ANALYSIS: return "Alert Analysis";
        case Phase::METRICS_NAVIGATION: return "Metrics Navigation";
        case Phase::POOL_OPTIMIZATION: return "Pool Optimization";
        case Phase::CONFIG_DEPLOYMENT: return "Configuration Deployment";
        case Phase::COMPLETED: return "Completed";
        default: return "Unknown";
    }
}

} // namespace production_incident

// Plugin exports
extern "C" {
    PLUGIN_EXPORT devescape::IEscapeRoom* createRoom() {
        return new production_incident::ProductionIncidentRoom();
    }

    PLUGIN_EXPORT void destroyRoom(devescape::IEscapeRoom* room) {
        delete room;
    }

    PLUGIN_EXPORT uint32_t getPluginVersion() {
        return 0x010000;  // v1.0.0
    }

    PLUGIN_EXPORT const char* getPluginName() {
        return "Production Incident";
    }
}
