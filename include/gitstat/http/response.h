#ifndef GITSTAT_HTTP_RESPONSE_H
#define GITSTAT_HTTP_RESPONSE_H

#include <stddef.h>
#include <stdint.h>
#include "gitstat/util/buffer.h"

typedef struct gitstat_http_response {
    long status_code;
    gitstat_buffer_t body;
} gitstat_http_response_t;

void gitstat_http_response_init(gitstat_http_response_t *res);
void gitstat_http_response_free(gitstat_http_response_t *res);

#endif
