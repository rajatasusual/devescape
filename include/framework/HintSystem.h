#pragma once

#include <string>
#include <vector>

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

struct HintTier {
    std::string text;
    int minFailedAttempts;
    float minTimeRemainingPercent;
};

class HintSystem {
public:
    HintSystem();

    void addHint(const std::string& text, int minFailedAttempts = 0, float minTimePercent = 1.0f);

    bool canRequestHint(int currentHintLevel, int failedAttempts, float timeRemainingPercent) const;
    std::string getHint(int hintLevel) const;
    int getMaxHintLevel() const { return static_cast<int>(hints_.size()); }

private:
    std::vector<HintTier> hints_;
};

} // namespace devescape
