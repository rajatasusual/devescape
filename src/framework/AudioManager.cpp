#include "framework/AudioManager.h"
#include <cmath>
#include <iostream>

namespace devescape {

// Note frequency table (A4 = 440Hz)
static const float NOTE_FREQUENCIES[] = {
    261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f,  // C4-F4
    369.99f, 392.00f, 415.30f, 440.00f, 466.16f, 493.88f,  // F#4-B4
    523.25f, 554.37f, 587.33f, 622.25f, 659.25f, 698.46f   // C5-F5
};

AudioManager::AudioManager()
    : audioDevice_(0)
    , sampleRate_(44100)
    , masterVolume_(0.3f)
    , currentTheme_(ThemeType::AMBIENT)
    , tensionLevel_(0.0f) {

    for (int i = 0; i < 4; ++i) {
        channels_[i].type = static_cast<Channel::Type>(i);
        channels_[i].frequency = 440.0f;
        channels_[i].duty_cycle = 0.5f;
        channels_[i].waveform_phase = 0;
        channels_[i].volume = 0.25f;
        channels_[i].enabled = false;
    }
}

AudioManager::~AudioManager() {
    cleanup();
}

bool AudioManager::initialize() {
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);

    desiredSpec.freq = sampleRate_;
    desiredSpec.format = AUDIO_F32SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = audioCallback;
    desiredSpec.userdata = this;

    audioDevice_ = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &audioSpec_, 0);

    if (audioDevice_ == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_PauseAudioDevice(audioDevice_, 0);  // Start playback
    return true;
}

void AudioManager::cleanup() {
    if (audioDevice_ != 0) {
        SDL_CloseAudioDevice(audioDevice_);
        audioDevice_ = 0;
    }
}

void AudioManager::playTheme(const std::string& themeName, ThemeType type) {
    currentTheme_ = type;
    loadThemeParameters(type);
}

void AudioManager::stopMusic() {
    for (int i = 0; i < 4; ++i) {
        channels_[i].enabled = false;
    }
}

void AudioManager::setMusicVolume(float volume) {
    masterVolume_ = std::max(0.0f, std::min(1.0f, volume));
}

void AudioManager::updateTensionLevel(float percentTimeRemaining) {
    tensionLevel_ = 1.0f - percentTimeRemaining;

    // Adjust tempo based on tension
    float tempoMultiplier = 1.0f + (tensionLevel_ * 0.5f);  // Up to 50% faster

    for (int i = 0; i < 4; ++i) {
        channels_[i].frequency *= tempoMultiplier;
    }
}

void AudioManager::updateForPuzzleType(PuzzleType type) {
    // Adjust music characteristics based on puzzle type
    switch (type) {
        case PuzzleType::LOG_ANALYSIS:
            playTheme("crisis", ThemeType::CRISIS);
            break;
        case PuzzleType::METRICS_NAVIGATION:
            playTheme("focus", ThemeType::FOCUS);
            break;
        case PuzzleType::ALGORITHM:
            playTheme("complex", ThemeType::COMPLEX);
            break;
        case PuzzleType::CONFIGURATION:
            playTheme("focus", ThemeType::FOCUS);
            break;
        default:
            playTheme("ambient", ThemeType::AMBIENT);
            break;
    }
}

void AudioManager::playSoundEffect(const std::string& effectName) {
    // Trigger one-shot sound effect on noise channel
    channels_[3].enabled = true;
    channels_[3].volume = 0.4f;
}

void AudioManager::audioCallback(void* userdata, uint8_t* stream, int len) {
    AudioManager* manager = static_cast<AudioManager*>(userdata);
    float* fstream = reinterpret_cast<float*>(stream);
    int frameCount = len / sizeof(float);

    manager->generateAudioFrame(fstream, frameCount);
}

void AudioManager::generateAudioFrame(float* buffer, int frameCount) {
    for (int i = 0; i < frameCount; ++i) {
        float sample = 0.0f;

        // Mix all enabled channels
        if (channels_[0].enabled) {
            sample += generateSquareWave(channels_[0]) * channels_[0].volume;
        }
        if (channels_[1].enabled) {
            sample += generateSquareWave(channels_[1]) * channels_[1].volume;
        }
        if (channels_[2].enabled) {
            sample += generateTriangleWave(channels_[2]) * channels_[2].volume;
        }
        if (channels_[3].enabled) {
            sample += generateNoiseWave(channels_[3]) * channels_[3].volume;
        }

        buffer[i] = sample * masterVolume_;

        // Advance waveform phases
        for (int ch = 0; ch < 4; ++ch) {
            channels_[ch].waveform_phase = (channels_[ch].waveform_phase + 1) % 8192;
        }
    }
}

float AudioManager::generateSquareWave(const Channel& ch) {
    float phase = static_cast<float>(ch.waveform_phase) / 8192.0f;
    return (phase < ch.duty_cycle) ? 1.0f : -1.0f;
}

float AudioManager::generateTriangleWave(const Channel& ch) {
    float phase = static_cast<float>(ch.waveform_phase) / 8192.0f;
    return 4.0f * std::abs(phase - 0.5f) - 1.0f;
}

float AudioManager::generateNoiseWave(const Channel& ch) {
    // Simple white noise using LFSR-like approach
    static uint32_t lfsr = 0xACE1u;
    lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
    return (lfsr & 1) ? 1.0f : -1.0f;
}

void AudioManager::loadThemeParameters(ThemeType theme) {
    // Define musical themes
    switch (theme) {
        case ThemeType::CRISIS:
            // Staccato, urgent
            channels_[0].enabled = true;
            channels_[0].frequency = NOTE_FREQUENCIES[11];  // B4
            channels_[0].duty_cycle = 0.125f;
            channels_[1].enabled = true;
            channels_[1].frequency = NOTE_FREQUENCIES[7];   // G#4
            channels_[1].duty_cycle = 0.25f;
            channels_[2].enabled = true;
            channels_[2].frequency = NOTE_FREQUENCIES[2];   // D4 bass
            channels_[3].enabled = true;
            break;

        case ThemeType::FOCUS:
            // Methodical, steady
            channels_[0].enabled = true;
            channels_[0].frequency = NOTE_FREQUENCIES[12];  // C5 melody
            channels_[0].duty_cycle = 0.5f;
            channels_[1].enabled = true;
            channels_[1].frequency = NOTE_FREQUENCIES[7];   // G4 harmony
            channels_[1].duty_cycle = 0.5f;
            channels_[2].enabled = true;
            channels_[2].frequency = NOTE_FREQUENCIES[0];   // C4 bass
            channels_[3].enabled = false;
            break;

        case ThemeType::COMPLEX:
            // Arpeggios, layered
            channels_[0].enabled = true;
            channels_[0].frequency = NOTE_FREQUENCIES[12];  // C5
            channels_[0].duty_cycle = 0.5f;
            channels_[1].enabled = true;
            channels_[1].frequency = NOTE_FREQUENCIES[16];  // E5
            channels_[1].duty_cycle = 0.25f;
            channels_[2].enabled = true;
            channels_[2].frequency = NOTE_FREQUENCIES[0];   // C4
            channels_[3].enabled = true;
            break;

        case ThemeType::VICTORY:
            // Triumphant, ascending
            channels_[0].enabled = true;
            channels_[0].frequency = NOTE_FREQUENCIES[17];  // F5
            channels_[0].duty_cycle = 0.5f;
            channels_[1].enabled = true;
            channels_[1].frequency = NOTE_FREQUENCIES[14];  // D5
            channels_[1].duty_cycle = 0.5f;
            channels_[2].enabled = true;
            channels_[2].frequency = NOTE_FREQUENCIES[0];   // C4
            channels_[3].enabled = true;
            break;

        default:  // AMBIENT
            // Sparse, calm
            channels_[0].enabled = true;
            channels_[0].frequency = NOTE_FREQUENCIES[0];   // C4
            channels_[0].duty_cycle = 0.5f;
            channels_[1].enabled = false;
            channels_[2].enabled = true;
            channels_[2].frequency = NOTE_FREQUENCIES[0];   // C4
            channels_[3].enabled = false;
            break;
    }
}

} // namespace devescape
