#ifndef GITSTAT_TYPES_H
#define GITSTAT_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum gitstat_provider_type {
    GITSTAT_PROVIDER_GITHUB = 0,
    GITSTAT_PROVIDER_GITLAB = 1,
} gitstat_provider_type_t;

typedef enum gitstat_color_mode {
    GITSTAT_COLOR_AUTO = 0,
    GITSTAT_COLOR_ALWAYS = 1,
    GITSTAT_COLOR_NEVER = 2,
} gitstat_color_mode_t;

#endif
