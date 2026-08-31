#ifndef GITSTAT_RENDER_PRINTER_H
#define GITSTAT_RENDER_PRINTER_H

#include <stdio.h>
#include "gitstat/render/theme.h"

void gitstat_print_error(FILE *stream, const gitstat_theme_t *theme, const char *fmt, ...);
void gitstat_print_user_not_found(FILE *stream, const gitstat_theme_t *theme, const char *username);

#endif
