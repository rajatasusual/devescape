#pragma once

#include "framework/DataTypes.h"
#include "framework/TerminalRenderer.h"
#include <string>
#include <cstdint>

namespace devescape {

/**
 * Base interface that all escape room plugins must implement
 */
class DEVESCAPE_API IEscapeRoom {
public:
    virtual ~IEscapeRoom() = default;

    // Metadata
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getAuthor() const = 0;
    virtual std::string getDescription() const = 0;
    virtual uint32_t getTotalDurationSeconds() const = 0;

    // Lifecycle
    virtual void initialize(const FrameworkContext& context) = 0;
    virtual void cleanup() = 0;

    // State management
    virtual GameState getCurrentState() const = 0;
    virtual bool isCompleted() const = 0;
    virtual bool isFailed() const = 0;
    virtual int getCompletionPercentage() const = 0;

    // Interaction - called every frame
    virtual ProcessResult processInput(const std::string& command) = 0;
    virtual void render(TerminalRenderer& renderer) const = 0;
    virtual void update(float deltaTimeSeconds) = 0;

    // Persistence
    virtual std::string serializeState() const = 0;
    virtual bool deserializeState(const std::string& data) = 0;

    // Hints
    virtual std::string getHint(int hintLevel) = 0;
    virtual int getMaxHintLevel() const = 0;
    virtual bool canUseHint() const = 0;

    // Timeout handling
    virtual void onSessionTimeout() = 0;
};

} // namespace devescape

// Plugin export macros
#ifdef _WIN32
    #define PLUGIN_EXPORT __declspec(dllexport)
#else
    #define PLUGIN_EXPORT
#endif

// Required plugin exports
extern "C" {
    PLUGIN_EXPORT devescape::IEscapeRoom* createRoom();
    PLUGIN_EXPORT void destroyRoom(devescape::IEscapeRoom* room);
    PLUGIN_EXPORT uint32_t getPluginVersion();
    PLUGIN_EXPORT const char* getPluginName();
}
