#include "framework/HintSystem.h"

namespace devescape {

HintSystem::HintSystem() = default;

void HintSystem::addHint(const std::string& text, int minFailedAttempts, float minTimePercent) {
    HintTier tier;
    tier.text = text;
    tier.minFailedAttempts = minFailedAttempts;
    tier.minTimeRemainingPercent = minTimePercent;
    hints_.push_back(tier);
}

bool HintSystem::canRequestHint(int currentHintLevel, int failedAttempts, float timeRemainingPercent) const {
    if (currentHintLevel >= static_cast<int>(hints_.size())) {
        return false;  // No more hints available
    }

    const auto& hint = hints_[currentHintLevel];

    // Check if conditions are met
    bool attemptsConditionMet = failedAttempts >= hint.minFailedAttempts;
    bool timeConditionMet = timeRemainingPercent <= hint.minTimeRemainingPercent;

    return attemptsConditionMet || timeConditionMet;
}

std::string HintSystem::getHint(int hintLevel) const {
    if (hintLevel < 0 || hintLevel >= static_cast<int>(hints_.size())) {
        return "No more hints available.";
    }
    return hints_[hintLevel].text;
}

} // namespace devescape
