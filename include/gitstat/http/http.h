#ifndef GITSTAT_HTTP_HTTP_H
#define GITSTAT_HTTP_HTTP_H

#include "gitstat/error.h"

[[nodiscard]] gitstat_error_t gitstat_http_global_init(void);
void gitstat_http_global_cleanup(void);

#endif
