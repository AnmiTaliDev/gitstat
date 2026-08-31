#include "gitstat/util/memory.h"
#include <stdlib.h>

void *gitstat_alloc(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    return malloc(size);
}

void *gitstat_calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) {
        return nullptr;
    }
    return calloc(count, size);
}

void *gitstat_realloc(void *ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return nullptr;
    }
    return realloc(ptr, size);
}

void gitstat_free(void *ptr) {
    free(ptr);
}
