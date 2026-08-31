#ifndef GITSTAT_OS_ENV_H
#define GITSTAT_OS_ENV_H

#include <stddef.h>
#include <stdbool.h>

[[nodiscard]] const char *gitstat_env_get(const char *name);
[[nodiscard]] bool gitstat_env_get_string(const char *name, char *buffer, size_t buffer_size);
[[nodiscard]] const char *gitstat_env_detect_username(void);

#endif
