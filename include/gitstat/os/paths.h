#ifndef GITSTAT_OS_PATHS_H
#define GITSTAT_OS_PATHS_H

#include <stddef.h>
#include <stdbool.h>
#include "gitstat/error.h"

[[nodiscard]] gitstat_error_t gitstat_paths_get_config_path(char *buffer, size_t buffer_size);
[[nodiscard]] bool gitstat_paths_file_exists(const char *path);

#endif
