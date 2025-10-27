#include "framework/TerminalRenderer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace devescape {

TerminalRenderer::TerminalRenderer() : width_(80), height_(24) {
    initialize();
}

TerminalRenderer::~TerminalRenderer() = default;

void TerminalRenderer::initialize() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width_ = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height_ = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    width_ = w.ws_col;
    height_ = w.ws_row;
#endif

    if (width_ < 80) width_ = 80;
    if (height_ < 24) height_ = 24;

    initializeBuffer();
}

void TerminalRenderer::initializeBuffer() {
    screenBuffer_.clear();
    screenBuffer_.resize(height_, std::string(width_, ' '));
}

std::string TerminalRenderer::getColorCode(ColorType color, bool bold) const {
    std::string code = "\033[";
    if (bold) code += "1;";

    switch (color) {
        case ColorType::ACCENT:      code += "36"; break; // Cyan
        case ColorType::ALERT:       code += "31"; break; // Red
        case ColorType::SUCCESS:     code += "32"; break; // Green
        case ColorType::WARNING:     code += "33"; break; // Yellow
        case ColorType::ERROR_COLOR: code += "35"; break; // Magenta
        case ColorType::STATUS:      code += "37"; break; // White
        case ColorType::PENDING:     code += "33"; break; // Yellow
        default:                     code += "0";  break; // Reset
    }

    code += "m";
    return code;
}

void TerminalRenderer::clearScreen() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0, 0};
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
#else
    std::cout << "\033[2J\033[H";
#endif
    initializeBuffer();
}

void TerminalRenderer::drawBox(int x, int y, int w, int h, const std::string& title) {
    if (x < 0 || y < 0 || x + w > width_ || y + h > height_) return;
    if (w < 2 || h < 2) return;

    // Top border with title
    std::string topLine = "+";
    if (!title.empty() && title.length() + 4 < static_cast<size_t>(w)) {
        topLine += "- " + title + " ";
        int remaining = w - static_cast<int>(title.length()) - 5;
        topLine += std::string(std::max<int>(0, remaining), '-');
    } else {
        topLine += std::string(w - 2, '-');
    }
    topLine += "+";

    if (y < height_ && static_cast<size_t>(x + topLine.length()) <= static_cast<size_t>(width_)) {
        screenBuffer_[y].replace(x, topLine.length(), topLine);
    }

    // Sides
    for (int i = 1; i < h - 1; ++i) {
        if (y + i < height_) {
            if (x < width_) screenBuffer_[y + i][x] = '|';
            if (x + w - 1 < width_) screenBuffer_[y + i][x + w - 1] = '|';
        }
    }

    // Bottom border
    std::string bottomLine = "+" + std::string(w - 2, '-') + "+";
    if (y + h - 1 < height_ && static_cast<size_t>(x + bottomLine.length()) <= static_cast<size_t>(width_)) {
        screenBuffer_[y + h - 1].replace(x, bottomLine.length(), bottomLine);
    }
}

void TerminalRenderer::drawText(int x, int y, const std::string& text, ColorType color, bool bold) {
    if (y < 0 || y >= height_ || x < 0) return;

    std::string coloredText = getColorCode(color, bold) + text + getColorCode(ColorType::DEFAULT);

    // Simple text insertion (note: ANSI codes don't count toward visible width)
    if (x < width_) {
        size_t remainingSpace = width_ - x;
        if (text.length() <= remainingSpace) {
            // For simplicity on Windows, just insert raw text without ANSI for now
            // Full ANSI support on Windows requires VT100 mode enabled
            screenBuffer_[y].replace(x, text.length(), text);
        }
    }
}

void TerminalRenderer::drawProgressBar(int x, int y, float percent, int width, ColorType color) {
    if (y < 0 || y >= height_ || x < 0) return;

    int filled = static_cast<int>(width * std::max<float>(0.0f, std::min<float>(1.0f, percent)));
    std::string bar = "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
    drawText(x, y, bar, color);
}

void TerminalRenderer::drawTimer(int x, int y, int secondsRemaining, PressureLevel pressure) {
    ColorType color;
    bool bold = false;

    switch (pressure) {
        case PressureLevel::LOW:      color = ColorType::SUCCESS; break;
        case PressureLevel::MEDIUM:   color = ColorType::WARNING; break;
        case PressureLevel::HIGH:     color = ColorType::ALERT; bold = true; break;
        case PressureLevel::CRITICAL: color = ColorType::ALERT; bold = true; break;
    }

    std::string timeStr = formatTime(secondsRemaining);
    drawText(x, y, timeStr, color, bold);
}

std::string TerminalRenderer::formatTime(int seconds) const {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes << ":" 
        << std::setfill('0') << std::setw(2) << secs;
    return oss.str();
}

void TerminalRenderer::render() {
    // Move cursor to top-left
#ifdef _WIN32
    COORD coordScreen = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coordScreen);
#else
    std::cout << "\033[H";
#endif

    for (const auto& line : screenBuffer_) {
        std::cout << line << "\n";
    }
    std::cout << std::flush;
}

void TerminalRenderer::setCursorVisible(bool visible) {
#ifdef _WIN32
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
#else
    if (visible) {
        std::cout << "\033[?25h"; // Show cursor
    } else {
        std::cout << "\033[?25l"; // Hide cursor
    }
#endif
}

} // namespace devescape
