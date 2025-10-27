#pragma once

#include "framework/DataTypes.h"
#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <vector>

namespace devescape {

class DEVESCAPE_API AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool initialize();
    void cleanup();

    void playTheme(const std::string& themeName, ThemeType type);
    void stopMusic();
    void setMusicVolume(float volume);

    void updateTensionLevel(float percentTimeRemaining);
    void updateForPuzzleType(PuzzleType type);

    void playSoundEffect(const std::string& effectName);

private:
    struct Channel {
        enum Type { SQUARE1, SQUARE2, TRIANGLE, NOISE };
        Type type;
        float frequency;
        float duty_cycle;
        int waveform_phase;
        float volume;
        bool enabled;
    };

    SDL_AudioDeviceID audioDevice_;
    SDL_AudioSpec audioSpec_;
    Channel channels_[4];
    float masterVolume_;
    int sampleRate_;

    ThemeType currentTheme_;
    float tensionLevel_;

    static void audioCallback(void* userdata, uint8_t* stream, int len);
    void generateAudioFrame(float* buffer, int frameCount);
    float generateSquareWave(const Channel& ch);
    float generateTriangleWave(const Channel& ch);
    float generateNoiseWave(const Channel& ch);

    void loadThemeParameters(ThemeType theme);
};

} // namespace devescape
