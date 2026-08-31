#ifndef GITSTAT_HTTP_CLIENT_H
#define GITSTAT_HTTP_CLIENT_H

#include <stddef.h>
#include "gitstat/error.h"
#include "gitstat/http/response.h"

typedef struct gitstat_http_request {
    const char *url;
    const char *method;
    const char *post_data;
    const char *token;
    const char *user_agent;
    long timeout_seconds;
} gitstat_http_request_t;

[[nodiscard]] gitstat_error_t gitstat_http_execute(const gitstat_http_request_t *req, gitstat_http_response_t *res);

#endif
