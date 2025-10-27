#pragma once

#include "framework/DataTypes.h"
#include <chrono>

namespace devescape {

class AudioManager;

class TimerSystem {
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
