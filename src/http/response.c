#include "gitstat/http/response.h"

void gitstat_http_response_init(gitstat_http_response_t *res) {
    if (res == nullptr) {
        return;
    }
    res->status_code = 0;
    gitstat_buffer_init(&res->body);
}

void gitstat_http_response_free(gitstat_http_response_t *res) {
    if (res == nullptr) {
        return;
    }
    res->status_code = 0;
    gitstat_buffer_free(&res->body);
}
