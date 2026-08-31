#include "gitstat/os/process.h"
#include "gitstat/os/platform.h"
#include <stdio.h>
#include <string.h>

gitstat_error_t gitstat_process_run_capture(const char *command, char *output, size_t output_size) {
    if (command == nullptr || output == nullptr || output_size == 0) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    output[0] = '\0';

#if defined(GITSTAT_OS_WINDOWS)
    FILE *fp = _popen(command, "r");
#else
    FILE *fp = popen(command, "r");
#endif

    if (fp == nullptr) {
        return GITSTAT_ERR_IO;
    }

    size_t total_read = 0;
    while (total_read + 1 < output_size && fgets(output + total_read, (int)(output_size - total_read), fp) != nullptr) {
        total_read = strlen(output);
    }

#if defined(GITSTAT_OS_WINDOWS)
    int status = _pclose(fp);
#else
    int status = pclose(fp);
#endif

    if (status != 0) {
        return GITSTAT_ERR_IO;
    }

    while (total_read > 0 && (output[total_read - 1] == '\n' || output[total_read - 1] == '\r')) {
        output[--total_read] = '\0';
    }

    return GITSTAT_OK;
}
