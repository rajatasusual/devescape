#pragma once

#include "framework/DataTypes.h"
#include <string>
#include <vector>

namespace devescape {

class TerminalRenderer {
public:
    TerminalRenderer();
    ~TerminalRenderer();

    // Initialize terminal dimensions
    void initialize();

    // Drawing primitives
    void drawBox(int x, int y, int w, int h, const std::string& title);
    void drawText(int x, int y, const std::string& text, ColorType color = ColorType::DEFAULT, bool bold = false);
    void drawProgressBar(int x, int y, float percent, int width, ColorType color);
    void drawTimer(int x, int y, int secondsRemaining, PressureLevel pressure);

    // Screen management
    void clearScreen();
    void render();  // Write buffer to terminal
    void setCursorVisible(bool visible);

    // Terminal properties
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // ANSI color codes
    std::string getColorCode(ColorType color, bool bold = false) const;

private:
    int width_;
    int height_;
    std::vector<std::string> screenBuffer_;

    void initializeBuffer();
    std::string formatTime(int seconds) const;
};

// Terminal control functions
class TerminalControl {
public:
    static void disableEcho();
    static void enableEcho();
    static void disableCtrlC();
    static void enableCtrlC();
    static void setRawMode();
    static void restoreTerminalMode();
    static std::string readInputNonBlocking();

private:
    static bool originalModeStored_;
#ifdef _WIN32
    static void* originalMode_;
#else
    static struct termios originalTermios_;
#endif
};

} // namespace devescape
