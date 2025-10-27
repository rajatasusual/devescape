#include <iostream>
#include <SDL2/SDL.h>

// Forward declaration of GameLoop class
namespace devescape {
    class GameLoop;
}

// Declare the GameLoop run function
extern void runDevEscapeFramework();

int main(int argc, char* argv[]) {
    std::cout << "DevEscape Framework v1.0\n";
    std::cout << "Developer-Centric Escape Room Platform\n";
    std::cout << "======================================\n\n";

    runDevEscapeFramework();

    return 0;
}

// Implementation using GameLoop
#include "framework/PluginManager.h"
#include "framework/StateManager.h"
#include "framework/AudioManager.h"
#include "framework/TerminalRenderer.h"
#include "framework/TimerSystem.h"
#include "framework/IEscapeRoom.h"
#include <thread>
#include <chrono>
#include <memory>

namespace devescape {

class DevEscapeFramework {
public:
    DevEscapeFramework()
        : pluginManager_("./plugins")
        , stateManager_("./data/checkpoints")
        , timerSystem_(nullptr)
        , currentRoom_(nullptr) {
    }

    void run() {
        if (!initialize()) {
            return;
        }

        displayMenu();

        cleanup();
    }

private:
    bool initialize() {
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL initialization failed\n";
            return false;
        }

        if (!audioManager_.initialize()) {
            std::cerr << "Audio initialization failed\n";
            return false;
        }

        pluginManager_.scanForPlugins();
        return true;
    }

    void cleanup() {
        TerminalControl::restoreTerminalMode();
        audioManager_.cleanup();
        SDL_Quit();
    }

    void displayMenu() {
        auto plugins = pluginManager_.getAvailablePlugins();

        if (plugins.empty()) {
            std::cout << "No escape rooms found!\n";
            std::cout << "Please build the Production Incident plugin.\n";
            return;
        }

        std::cout << "Available Rooms:\n";
        for (size_t i = 0; i < plugins.size(); ++i) {
            std::cout << "  " << (i+1) << ". " << plugins[i].name << "\n";
        }

        std::cout << "\nSelect room: ";
        size_t choice;
        std::cin >> choice;

        if (choice > 0 && choice <= plugins.size()) {
            startRoom(plugins[choice - 1].name);
        }
    }

    void startRoom(const std::string& roomName) {
        std::cout << "Loading " << roomName << "...\n";

        currentRoom_ = pluginManager_.loadRoom(roomName);
        if (!currentRoom_) {
            std::cerr << "Failed to load room\n";
            return;
        }

        FrameworkContext context;
        context.audioManager = &audioManager_;
        context.stateManager = &stateManager_;

        currentRoom_->initialize(context);

        std::cout << "Room loaded successfully!\n";
        std::cout << "Duration: " << currentRoom_->getTotalDurationSeconds() << " seconds\n";
        std::cout << "Description: " << currentRoom_->getDescription() << "\n\n";
        std::cout << "Press ENTER to start...";
        std::cin.ignore();
        std::cin.get();

        // Would run full game loop here
        // For now, just demonstrate the room is loaded

        currentRoom_->cleanup();
        pluginManager_.unloadRoom(currentRoom_, roomName);
    }

    PluginManager pluginManager_;
    StateManager stateManager_;
    AudioManager audioManager_;
    std::unique_ptr<TimerSystem> timerSystem_;
    IEscapeRoom* currentRoom_;
};

} // namespace devescape

void runDevEscapeFramework() {
    devescape::DevEscapeFramework framework;
    framework.run();
}
