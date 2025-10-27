#pragma once

#include "framework/DataTypes.h"
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

class AudioManager;

class DEVESCAPE_API TimerSystem {
public:
    TimerSystem(int totalSeconds, AudioManager* audioMgr);

    void start();
    void update(float deltaTime);
    bool isExpired() const;

    int getSecondsRemaining() const { return secondsRemaining_; }
    int getSecondsElapsed() const { return totalSeconds_ - secondsRemaining_; }
    float getPercentRemaining() const;
    PressureLevel getPressureLevel() const { return pressureLevel_; }

private:
    int totalSeconds_;
    int secondsRemaining_;
    float accumulatedTime_;
    PressureLevel pressureLevel_;
    AudioManager* audioManager_;

    void updatePressureLevel();
};

} // namespace devescape
