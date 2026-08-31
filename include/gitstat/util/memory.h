#ifndef GITSTAT_UTIL_MEMORY_H
#define GITSTAT_UTIL_MEMORY_H

#include <stddef.h>

[[nodiscard]] void *gitstat_alloc(size_t size);
[[nodiscard]] void *gitstat_calloc(size_t count, size_t size);
[[nodiscard]] void *gitstat_realloc(void *ptr, size_t size);
void gitstat_free(void *ptr);

#endif
