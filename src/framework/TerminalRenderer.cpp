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

#ifndef _WIN32
bool TerminalControl::originalModeStored_ = false;
struct termios TerminalControl::originalTermios_;
#endif

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
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
    initializeBuffer();
}

void TerminalRenderer::drawBox(int x, int y, int w, int h, const std::string& title) {
    if (x < 0 || y < 0 || x + w >= width_ || y + h >= height_) return;

    // Top border with title
    std::string topLine = "┌";
    if (!title.empty()) {
        topLine += "─ " + title + " ";
        int remaining = w - title.length() - 4;
        topLine += std::string(std::max<int>(0, remaining), '-');
    } else {
        topLine += std::string(w - 2, '─');
    }
    topLine += "┐";

    if (y < height_) {
        screenBuffer_[y] = screenBuffer_[y].substr(0, x) + topLine + 
                          screenBuffer_[y].substr(std::min<int>(x + w, width_));
    }

    // Sides
    for (int i = 1; i < h - 1; ++i) {
        if (y + i < height_) {
            screenBuffer_[y + i][x] = '│';
            screenBuffer_[y + i][x + w - 1] = '│';
        }
    }

    // Bottom border
    std::string bottomLine = "└" + std::string(w - 2, '─') + "┘";
    if (y + h - 1 < height_) {
        screenBuffer_[y + h - 1] = screenBuffer_[y + h - 1].substr(0, x) + bottomLine +
                                   screenBuffer_[y + h - 1].substr(std::min<int>(x + w, width_));
    }
}

void TerminalRenderer::drawText(int x, int y, const std::string& text, ColorType color, bool bold) {
    if (y < 0 || y >= height_) return;

    std::string coloredText = getColorCode(color, bold) + text + getColorCode(ColorType::DEFAULT);

    // Insert text into buffer (accounting for ANSI codes)
    if (x >= 0 && x < width_) {
        std::string& line = screenBuffer_[y];
        line = line.substr(0, x) + coloredText + line.substr(std::min<int>(x + static_cast<int>(text.length()), width_));
    }
}

void TerminalRenderer::drawProgressBar(int x, int y, float percent, int width, ColorType color) {
    if (y < 0 || y >= height_) return;

    int filled = static_cast<int>(width * percent);
    std::string bar = "[" + std::string(filled, '█') + std::string(width - filled, '░') + "]";
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
    clearScreen();
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

// Terminal Control Implementation
#ifndef _WIN32
void TerminalControl::disableEcho() {
    if (!originalModeStored_) {
        tcgetattr(STDIN_FILENO, &originalTermios_);
        originalModeStored_ = true;
    }

    struct termios newTermios = originalTermios_;
    newTermios.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
}

void TerminalControl::enableEcho() {
    if (originalModeStored_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
    }
}

void TerminalControl::setRawMode() {
    if (!originalModeStored_) {
        tcgetattr(STDIN_FILENO, &originalTermios_);
        originalModeStored_ = true;
    }

    struct termios raw = originalTermios_;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void TerminalControl::restoreTerminalMode() {
    if (originalModeStored_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
    }
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

std::string TerminalControl::readInputNonBlocking() {
    std::string input;
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        if (c == '\n' || c == '\r') {
            return input;
        }
        input += c;
    }
    return input;
}
#endif

} // namespace devescape
