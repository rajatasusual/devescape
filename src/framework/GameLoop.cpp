#include "framework/PluginManager.h"
#include "framework/StateManager.h"
#include "framework/AudioManager.h"
#include "framework/TerminalRenderer.h"
#include "framework/TimerSystem.h"
#include "framework/IEscapeRoom.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace devescape {

class GameLoop {
public:
    GameLoop(const std::string& pluginDir, const std::string& checkpointDir)
        : pluginManager_(pluginDir)
        , stateManager_(checkpointDir)
        , timerSystem_(nullptr)
        , currentRoom_(nullptr)
        , running_(false) {
    }

    ~GameLoop() {
        cleanup();
    }

    bool initialize() {
        // Initialize SDL for audio
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            return false;
        }

        // Initialize audio
        if (!audioManager_.initialize()) {
            return false;
        }

        // Scan for plugins
        pluginManager_.scanForPlugins();

        return true;
    }

    void cleanup() {
        if (currentRoom_) {
            currentRoom_->cleanup();
        }

        TerminalControl::restoreTerminalMode();
        audioManager_.cleanup();
        SDL_Quit();
    }

    void run() {
        // Display available rooms
        auto plugins = pluginManager_.getAvailablePlugins();
        if (plugins.empty()) {
            std::cout << "No escape rooms found. Please add plugins to the plugins/ directory.\n";
            return;
        }

        std::cout << "Available Escape Rooms:\n";
        for (size_t i = 0; i < plugins.size(); ++i) {
            std::cout << (i + 1) << ". " << plugins[i].name << "\n";
        }

        // Check for resume option
        if (stateManager_.hasRecentCheckpoint()) {
            std::cout << "\nR. Resume previous session\n";
        }

        std::cout << "\nSelect a room (or Q to quit): ";
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "Q" || choice == "q") {
            return;
        }

        GameSession session;
        bool resuming = false;

        if ((choice == "R" || choice == "r") && stateManager_.hasRecentCheckpoint()) {
            if (stateManager_.loadAutoCheckpoint(session)) {
                resuming = true;
                std::cout << "Resuming session: " << session.metadata.roomName << "\n";
                std::cout << "Time remaining: " << session.timeRemainingSeconds << " seconds\n";
            }
        }

        if (!resuming) {
            try {
                size_t idx = std::stoi(choice) - 1;
                if (idx < plugins.size()) {
                    session.metadata.id = "session_" + std::to_string(std::time(nullptr));
                    session.metadata.roomName = plugins[idx].name;
                    session.metadata.playerName = "player";
                    session.metadata.startedAt = std::chrono::system_clock::now();
                    session.metadata.status = "in_progress";
                } else {
                    std::cout << "Invalid selection.\n";
                    return;
                }
            } catch (...) {
                std::cout << "Invalid input.\n";
                return;
            }
        }

        // Load the room
        currentRoom_ = pluginManager_.loadRoom(session.metadata.roomName);
        if (!currentRoom_) {
            std::cout << "Failed to load room.\n";
            return;
        }

        // Get room duration
        session.metadata.totalTimeSeconds = currentRoom_->getTotalDurationSeconds();
        if (!resuming) {
            session.timeRemainingSeconds = session.metadata.totalTimeSeconds;
        }

        // Create timer
        timerSystem_ = std::make_unique<TimerSystem>(session.timeRemainingSeconds, &audioManager_);

        // Initialize framework context
        FrameworkContext context;
        context.audioManager = &audioManager_;
        context.stateManager = &stateManager_;
        context.dataDirectory = "./data";
        context.checkpointDirectory = "./data/checkpoints";

        // Initialize room
        currentRoom_->initialize(context);

        if (resuming) {
            // Restore room state
            std::string serializedState = "{}";  // Would come from session
            currentRoom_->deserializeState(serializedState);
        }

        // Set up terminal
        TerminalControl::setRawMode();

        // Start main game loop
        runGameLoop(session);

        // Cleanup
        pluginManager_.unloadRoom(currentRoom_, session.metadata.roomName);
        currentRoom_ = nullptr;
    }

private:
    void runGameLoop(GameSession& session) {
        using namespace std::chrono;

        const float FRAME_TIME = 1.0f / 60.0f;  // 60 FPS
        auto frameStart = steady_clock::now();

        timerSystem_->start();
        running_ = true;

        int frameCounter = 0;

        while (running_) {
            auto now = steady_clock::now();
            float deltaTime = duration<float>(now - frameStart).count();
            frameStart = now;

            // Update timer
            timerSystem_->update(deltaTime);

            if (timerSystem_->isExpired()) {
                currentRoom_->onSessionTimeout();
                running_ = false;
                break;
            }

            // Process input (non-blocking)
            std::string input = TerminalControl::readInputNonBlocking();
            if (!input.empty()) {
                if (input == "surrender") {
                    std::cout << "\nType 'I SURRENDER' three times to quit:\n";
                    // Simplified - would require actual surrender confirmation
                }

                ProcessResult result = currentRoom_->processInput(input);
                if (result.sessionEnded) {
                    running_ = false;
                    break;
                }
            }

            // Update room
            currentRoom_->update(deltaTime);

            // Render
            TerminalRenderer renderer;
            currentRoom_->render(renderer);
            renderer.drawTimer(70, 0, timerSystem_->getSecondsRemaining(), 
                             timerSystem_->getPressureLevel());
            renderer.render();

            // Auto-save every 30 seconds (1800 frames at 60 FPS)
            if (frameCounter % 1800 == 0) {
                session.timeRemainingSeconds = timerSystem_->getSecondsRemaining();
                session.metadata.timeElapsedSeconds = timerSystem_->getSecondsElapsed();
                session.metadata.checkpointedAt = system_clock::now();
                stateManager_.createAutoCheckpoint(session);
            }

            frameCounter++;

            // Frame limiting
            auto frameEnd = steady_clock::now();
            float frameDuration = duration<float>(frameEnd - frameStart).count();
            if (frameDuration < FRAME_TIME) {
                std::this_thread::sleep_for(duration<float>(FRAME_TIME - frameDuration));
            }
        }

        // Final save
        session.metadata.status = currentRoom_->isCompleted() ? "completed" : "failed";
        stateManager_.createAutoCheckpoint(session);
    }

    PluginManager pluginManager_;
    StateManager stateManager_;
    AudioManager audioManager_;
    std::unique_ptr<TimerSystem> timerSystem_;
    IEscapeRoom* currentRoom_;
    bool running_;
};

} // namespace devescape
