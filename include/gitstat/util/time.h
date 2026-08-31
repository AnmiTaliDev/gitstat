#ifndef GITSTAT_UTIL_TIME_H
#define GITSTAT_UTIL_TIME_H

#include <stddef.h>

void gitstat_time_format_iso_to_date(const char *iso_date, char *output, size_t output_size);

#endif
