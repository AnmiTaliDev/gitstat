#ifndef GITSTAT_CONFIG_INI_H
#define GITSTAT_CONFIG_INI_H

#include <stdbool.h>
#include "gitstat/error.h"

typedef void (*gitstat_ini_handler_t)(void *user_data, const char *section, const char *key, const char *value);

[[nodiscard]] gitstat_error_t gitstat_ini_parse_file(const char *filename, gitstat_ini_handler_t handler, void *user_data);
[[nodiscard]] gitstat_error_t gitstat_ini_parse_string(const char *content, gitstat_ini_handler_t handler, void *user_data);

#endif
