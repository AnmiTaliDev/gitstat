#include "gitstat/util/time.h"
#include <string.h>

void gitstat_time_format_iso_to_date(const char *iso_date, char *output, size_t output_size) {
    if (output == nullptr || output_size == 0) {
        return;
    }
    output[0] = '\0';
    if (iso_date == nullptr || iso_date[0] == '\0') {
        return;
    }

    const char *t_pos = strchr(iso_date, 'T');
    if (t_pos != nullptr) {
        size_t len = (size_t)(t_pos - iso_date);
        if (len >= output_size) {
            len = output_size - 1;
        }
        strncpy(output, iso_date, len);
        output[len] = '\0';
    } else {
        strncpy(output, iso_date, output_size - 1);
        output[output_size - 1] = '\0';
    }
}
