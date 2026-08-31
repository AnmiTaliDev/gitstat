#ifndef GITSTAT_UTIL_BUFFER_H
#define GITSTAT_UTIL_BUFFER_H

#include <stddef.h>
#include <stdbool.h>
#include "gitstat/error.h"

typedef struct gitstat_buffer {
    char *data;
    size_t size;
    size_t capacity;
} gitstat_buffer_t;

void gitstat_buffer_init(gitstat_buffer_t *buf);
void gitstat_buffer_free(gitstat_buffer_t *buf);
void gitstat_buffer_clear(gitstat_buffer_t *buf);
gitstat_error_t gitstat_buffer_append(gitstat_buffer_t *buf, const char *data, size_t length);
gitstat_error_t gitstat_buffer_append_str(gitstat_buffer_t *buf, const char *str);
gitstat_error_t gitstat_buffer_append_char(gitstat_buffer_t *buf, char c);
gitstat_error_t gitstat_buffer_append_fmt(gitstat_buffer_t *buf, const char *fmt, ...);

#endif
