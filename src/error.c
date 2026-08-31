#include "gitstat/error.h"

const char *gitstat_error_to_string(gitstat_error_t err) {
    switch (err) {
        case GITSTAT_OK:
            return "Success";
        case GITSTAT_ERR_NOMEM:
            return "Out of memory";
        case GITSTAT_ERR_INVALID_ARG:
            return "Invalid argument";
        case GITSTAT_ERR_IO:
            return "I/O error";
        case GITSTAT_ERR_HTTP:
            return "HTTP request failed";
        case GITSTAT_ERR_JSON_PARSE:
            return "JSON parsing error";
        case GITSTAT_ERR_NOT_FOUND:
            return "User or resource not found";
        case GITSTAT_ERR_AUTH:
            return "Authentication required or invalid token";
        case GITSTAT_ERR_PROVIDER:
            return "Provider error";
        case GITSTAT_ERR_CONFIG:
            return "Configuration error";
        case GITSTAT_ERR_GENERIC:
        default:
            return "Unknown error";
    }
}
