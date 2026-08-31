#include "gitstat/util/buffer.h"
#include "gitstat/util/memory.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void gitstat_buffer_init(gitstat_buffer_t *buf) {
    if (buf == nullptr) {
        return;
    }
    buf->data = nullptr;
    buf->size = 0;
    buf->capacity = 0;
}

void gitstat_buffer_free(gitstat_buffer_t *buf) {
    if (buf == nullptr) {
        return;
    }
    gitstat_free(buf->data);
    buf->data = nullptr;
    buf->size = 0;
    buf->capacity = 0;
}

void gitstat_buffer_clear(gitstat_buffer_t *buf) {
    if (buf == nullptr) {
        return;
    }
    buf->size = 0;
    if (buf->data != nullptr && buf->capacity > 0) {
        buf->data[0] = '\0';
    }
}

static gitstat_error_t gitstat_buffer_ensure_capacity(gitstat_buffer_t *buf, size_t needed) {
    if (buf->capacity >= needed) {
        return GITSTAT_OK;
    }
    size_t new_cap = buf->capacity == 0 ? 64 : buf->capacity * 2;
    while (new_cap < needed) {
        new_cap *= 2;
    }
    char *new_data = (char *)gitstat_realloc(buf->data, new_cap);
    if (new_data == nullptr) {
        return GITSTAT_ERR_NOMEM;
    }
    buf->data = new_data;
    buf->capacity = new_cap;
    return GITSTAT_OK;
}

gitstat_error_t gitstat_buffer_append(gitstat_buffer_t *buf, const char *data, size_t length) {
    if (buf == nullptr || data == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }
    if (length == 0) {
        return GITSTAT_OK;
    }
    gitstat_error_t err = gitstat_buffer_ensure_capacity(buf, buf->size + length + 1);
    if (err != GITSTAT_OK) {
        return err;
    }
    memcpy(buf->data + buf->size, data, length);
    buf->size += length;
    buf->data[buf->size] = '\0';
    return GITSTAT_OK;
}

gitstat_error_t gitstat_buffer_append_str(gitstat_buffer_t *buf, const char *str) {
    if (str == nullptr) {
        return GITSTAT_OK;
    }
    return gitstat_buffer_append(buf, str, strlen(str));
}

gitstat_error_t gitstat_buffer_append_char(gitstat_buffer_t *buf, char c) {
    return gitstat_buffer_append(buf, &c, 1);
}

gitstat_error_t gitstat_buffer_append_fmt(gitstat_buffer_t *buf, const char *fmt, ...) {
    if (buf == nullptr || fmt == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }
    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (len < 0) {
        va_end(args_copy);
        return GITSTAT_ERR_GENERIC;
    }

    gitstat_error_t err = gitstat_buffer_ensure_capacity(buf, buf->size + (size_t)len + 1);
    if (err != GITSTAT_OK) {
        va_end(args_copy);
        return err;
    }

    vsnprintf(buf->data + buf->size, (size_t)len + 1, fmt, args_copy);
    va_end(args_copy);
    buf->size += (size_t)len;
    return GITSTAT_OK;
}
