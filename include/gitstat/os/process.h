#ifndef GITSTAT_OS_PROCESS_H
#define GITSTAT_OS_PROCESS_H

#include <stddef.h>
#include "gitstat/error.h"

[[nodiscard]] gitstat_error_t gitstat_process_run_capture(const char *command, char *output, size_t output_size);

#endif
