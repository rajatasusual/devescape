#include "framework/TerminalRenderer.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <iostream>

namespace devescape {

bool TerminalControl::originalModeStored_ = false;
#ifdef _WIN32
static HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
static DWORD originalConsoleMode;
#endif

void TerminalControl::disableEcho() {
#ifdef _WIN32
    if (!originalModeStored_) {
        GetConsoleMode(hStdin, &originalConsoleMode);
        originalModeStored_ = true;
    }
    DWORD newMode = originalConsoleMode;
    newMode &= ~ENABLE_ECHO_INPUT;
    SetConsoleMode(hStdin, newMode);
#else
    if (!originalModeStored_) {
        tcgetattr(STDIN_FILENO, &originalTermios_);
        originalModeStored_ = true;
    }
    struct termios newTermios = originalTermios_;
    newTermios.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
#endif
}

void TerminalControl::enableEcho() {
#ifdef _WIN32
    if (originalModeStored_) {
        SetConsoleMode(hStdin, originalConsoleMode);
    }
#else
    if (originalModeStored_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
    }
#endif
}

void TerminalControl::setRawMode() {
#ifdef _WIN32
    if (!originalModeStored_) {
        GetConsoleMode(hStdin, &originalConsoleMode);
        originalModeStored_ = true;
    }
    DWORD newMode = originalConsoleMode;
    newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(hStdin, newMode);
#else
    if (!originalModeStored_) {
        tcgetattr(STDIN_FILENO, &originalTermios_);
        originalModeStored_ = true;
    }
    struct termios raw = originalTermios_;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
#endif
}

void TerminalControl::restoreTerminalMode() {
#ifdef _WIN32
    if (originalModeStored_) {
        SetConsoleMode(hStdin, originalConsoleMode);
    }
#else
    if (originalModeStored_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
#endif
}

std::string TerminalControl::readInputNonBlocking() {
    std::string input;
#ifdef _WIN32
    DWORD available = 0;
    if (GetNumberOfConsoleInputEvents(hStdin, &available) && available > 0) {
        INPUT_RECORD record;
        DWORD read = 0;
        while (ReadConsoleInput(hStdin, &record, 1, &read)) {
            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
                char c = record.Event.KeyEvent.uChar.AsciiChar;
                if (c == '\n' || c == '\n') {
                    return input;
                }
                if (c) input += c;
            }
        }
    }
#else
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        if (c == '\n' || c == '\n') {
            return input;
        }
        input += c;
    }
#endif
    return input;
}

} // namespace devescape
