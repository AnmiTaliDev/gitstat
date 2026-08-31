#ifndef GITSTAT_UTIL_STRING_H
#define GITSTAT_UTIL_STRING_H

#include <stddef.h>
#include <stdbool.h>

[[nodiscard]] char *gitstat_strdup(const char *str);
[[nodiscard]] char *gitstat_strndup(const char *str, size_t n);
[[nodiscard]] bool gitstat_str_is_empty(const char *str);
[[nodiscard]] bool gitstat_str_equals_ignore_case(const char *a, const char *b);
void gitstat_str_trim(char *str);
[[nodiscard]] char *gitstat_str_escape_json(const char *input);

#endif
