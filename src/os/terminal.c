#include "gitstat/os/terminal.h"
#include "gitstat/os/platform.h"
#include <stdlib.h>
#include <string.h>

#if defined(GITSTAT_OS_WINDOWS)
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

void gitstat_terminal_init(void) {
#if defined(GITSTAT_OS_WINDOWS)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

bool gitstat_terminal_is_interactive(FILE *stream) {
    if (stream == nullptr) {
        return false;
    }
#if defined(GITSTAT_OS_WINDOWS)
    int fd = _fileno(stream);
    if (fd < 0) {
        return false;
    }
    return _isatty(fd) != 0;
#else
    int fd = fileno(stream);
    if (fd < 0) {
        return false;
    }
    return isatty(fd) == 1;
#endif
}

bool gitstat_terminal_supports_color(FILE *stream, gitstat_color_mode_t mode) {
    if (mode == GITSTAT_COLOR_ALWAYS) {
        return true;
    }
    if (mode == GITSTAT_COLOR_NEVER) {
        return false;
    }

    const char *no_color = getenv("NO_COLOR");
    if (no_color != nullptr && no_color[0] != '\0') {
        return false;
    }

    const char *force_color = getenv("CLICOLOR_FORCE");
    if (force_color != nullptr && force_color[0] != '0') {
        return true;
    }

    const char *term = getenv("TERM");
    if (term != nullptr && (term[0] == '\0' || strcmp(term, "dumb") == 0)) {
        return false;
    }

    return gitstat_terminal_is_interactive(stream);
}
