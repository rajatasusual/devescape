#include "framework/TimerSystem.h"
#include "framework/AudioManager.h"

namespace devescape {

TimerSystem::TimerSystem(int totalSeconds, AudioManager* audioMgr)
    : totalSeconds_(totalSeconds)
    , secondsRemaining_(totalSeconds)
    , accumulatedTime_(0.0f)
    , pressureLevel_(PressureLevel::LOW)
    , audioManager_(audioMgr) {
}

void TimerSystem::start() {
    accumulatedTime_ = 0.0f;
    secondsRemaining_ = totalSeconds_;
}

void TimerSystem::update(float deltaTime) {
    accumulatedTime_ += deltaTime;

    if (accumulatedTime_ >= 1.0f) {
        secondsRemaining_ -= static_cast<int>(accumulatedTime_);
        accumulatedTime_ = accumulatedTime_ - static_cast<int>(accumulatedTime_);

        if (secondsRemaining_ < 0) {
            secondsRemaining_ = 0;
        }

        updatePressureLevel();
    }
}

bool TimerSystem::isExpired() const {
    return secondsRemaining_ <= 0;
}

float TimerSystem::getPercentRemaining() const {
    return static_cast<float>(secondsRemaining_) / static_cast<float>(totalSeconds_);
}

void TimerSystem::updatePressureLevel() {
    float percent = getPercentRemaining();

    PressureLevel oldLevel = pressureLevel_;

    if (percent > 0.5f) {
        pressureLevel_ = PressureLevel::LOW;
    } else if (percent > 0.25f) {
        pressureLevel_ = PressureLevel::MEDIUM;
    } else if (percent > 0.1f) {
        pressureLevel_ = PressureLevel::HIGH;
    } else {
        pressureLevel_ = PressureLevel::CRITICAL;
    }

    // Notify audio manager of tension changes
    if (pressureLevel_ != oldLevel && audioManager_) {
        audioManager_->updateTensionLevel(percent);
    }
}

} // namespace devescape
