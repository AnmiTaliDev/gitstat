#ifndef GITSTAT_ERROR_H
#define GITSTAT_ERROR_H

typedef enum gitstat_error {
    GITSTAT_OK = 0,
    GITSTAT_ERR_GENERIC = -1,
    GITSTAT_ERR_NOMEM = -2,
    GITSTAT_ERR_INVALID_ARG = -3,
    GITSTAT_ERR_IO = -4,
    GITSTAT_ERR_HTTP = -5,
    GITSTAT_ERR_JSON_PARSE = -6,
    GITSTAT_ERR_NOT_FOUND = -7,
    GITSTAT_ERR_AUTH = -8,
    GITSTAT_ERR_PROVIDER = -9,
    GITSTAT_ERR_CONFIG = -10,
} gitstat_error_t;

[[nodiscard]] const char *gitstat_error_to_string(gitstat_error_t err);

#endif
