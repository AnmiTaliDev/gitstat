#ifndef GITSTAT_OS_TERMINAL_H
#define GITSTAT_OS_TERMINAL_H

#include <stdbool.h>
#include <stdio.h>
#include "gitstat/types.h"

void gitstat_terminal_init(void);
[[nodiscard]] bool gitstat_terminal_is_interactive(FILE *stream);
[[nodiscard]] bool gitstat_terminal_supports_color(FILE *stream, gitstat_color_mode_t mode);

#endif
