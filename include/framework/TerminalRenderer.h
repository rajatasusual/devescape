#pragma once

#include "framework/DataTypes.h"
#include <string>
#include <vector>

namespace devescape {

class DEVESCAPE_API TerminalRenderer {
public:
    TerminalRenderer();
    ~TerminalRenderer();

    void initialize();

    void drawBox(int x, int y, int w, int h, const std::string& title);
    void drawText(int x, int y, const std::string& text, ColorType color = ColorType::DEFAULT, bool bold = false);
    void drawProgressBar(int x, int y, float percent, int width, ColorType color);
    void drawTimer(int x, int y, int secondsRemaining, PressureLevel pressure);

    void clearScreen();
    void render();
    void setCursorVisible(bool visible);

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    std::string getColorCode(ColorType color, bool bold = false) const;

private:
    int width_;
    int height_;
    std::vector<std::string> screenBuffer_;

    void initializeBuffer();
    std::string formatTime(int seconds) const;
};

class DEVESCAPE_API TerminalControl {
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
#ifndef _WIN32
    static struct termios originalTermios_;
#endif
};

} // namespace devescape
