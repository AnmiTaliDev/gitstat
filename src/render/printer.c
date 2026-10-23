#include "gitstat/render/printer.h"
#include <stdarg.h>

void gitstat_print_error(FILE *stream, const gitstat_theme_t *theme, const char *fmt, ...) {
    if (stream == nullptr || fmt == nullptr) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    if (theme != nullptr && theme->error[0] != '\0') {
        fputs(theme->error, stream);
    }
    vfprintf(stream, fmt, args);
    if (theme != nullptr && theme->reset[0] != '\0') {
        fputs(theme->reset, stream);
    }
    fputc('\n', stream);
    va_end(args);
}

void gitstat_print_user_not_found(FILE *stream, const gitstat_theme_t *theme, const char *username) {
    if (stream == nullptr) {
        return;
    }
    const char *user = (username != nullptr && username[0] != '\0') ? username : "unknown";
    if (theme != nullptr && theme->error[0] != '\0') {
        fprintf(stream, "%sUser %s%s%s%s not found or API error.%s\n",
            theme->error,
            theme->bold,
            user,
            theme->reset,
            theme->error,
            theme->reset
        );
    } else {
        fprintf(stream, "User %s not found or API error.\n", user);
    }
}

void gitstat_print_repo_not_found(FILE *stream, const gitstat_theme_t *theme, const char *repo) {
    if (stream == nullptr) {
        return;
    }
    const char *r = (repo != nullptr && repo[0] != '\0') ? repo : "unknown";
    if (theme != nullptr && theme->error[0] != '\0') {
        fprintf(stream, "%sRepository %s%s%s%s not found or API error.%s\n",
            theme->error,
            theme->bold,
            r,
            theme->reset,
            theme->error,
            theme->reset
        );
    } else {
        fprintf(stream, "Repository %s not found or API error.\n", r);
    }
}
