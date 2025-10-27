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
static HANDLE hStdin = INVALID_HANDLE_VALUE;
static DWORD originalConsoleMode = 0;

void initializeConsoleHandle() {
    if (hStdin == INVALID_HANDLE_VALUE) {
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
    }
}
#else
struct termios TerminalControl::originalTermios_;
#endif

void TerminalControl::disableEcho() {
#ifdef _WIN32
    initializeConsoleHandle();
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
    initializeConsoleHandle();
    if (originalModeStored_) {
        SetConsoleMode(hStdin, originalConsoleMode);
    }
#else
    if (originalModeStored_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
    }
#endif
}

void TerminalControl::disableCtrlC() {
    // Not implemented - would require signal handling
}

void TerminalControl::enableCtrlC() {
    // Not implemented - would require signal handling
}

void TerminalControl::setRawMode() {
#ifdef _WIN32
    initializeConsoleHandle();
    if (!originalModeStored_) {
        GetConsoleMode(hStdin, &originalConsoleMode);
        originalModeStored_ = true;
    }
    DWORD newMode = originalConsoleMode;
    newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    newMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    SetConsoleMode(hStdin, newMode);

    // Enable VT100 for output as well
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD outMode = 0;
    GetConsoleMode(hStdout, &outMode);
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, outMode);
#else
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
#endif
}

void TerminalControl::restoreTerminalMode() {
#ifdef _WIN32
    initializeConsoleHandle();
    if (originalModeStored_ && hStdin != INVALID_HANDLE_VALUE) {
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
    initializeConsoleHandle();
    DWORD numEvents = 0;
    if (GetNumberOfConsoleInputEvents(hStdin, &numEvents) && numEvents > 0) {
        INPUT_RECORD record;
        DWORD numRead = 0;
        while (PeekConsoleInput(hStdin, &record, 1, &numRead) && numRead > 0) {
            ReadConsoleInput(hStdin, &record, 1, &numRead);
            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
                char c = record.Event.KeyEvent.uChar.AsciiChar;
                if (c == '\r' || c == '\n') {
                    return input;
                }
                if (c != 0) {
                    input += c;
                }
            }
        }
    }
#else
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        if (c == '\n' || c == '\r') {
            return input;
        }
        input += c;
    }
#endif
    return input;
}

} // namespace devescape
