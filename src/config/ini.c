#include "gitstat/config/ini.h"
#include "gitstat/util/string.h"
#include "gitstat/util/memory.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

gitstat_error_t gitstat_ini_parse_string(const char *content, gitstat_ini_handler_t handler, void *user_data) {
    if (content == nullptr || handler == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    char section[128] = "";
    const char *line_start = content;

    while (*line_start != '\0') {
        const char *line_end = strchr(line_start, '\n');
        size_t line_len = line_end ? (size_t)(line_end - line_start) : strlen(line_start);

        char *line = gitstat_strndup(line_start, line_len);
        if (line == nullptr) {
            return GITSTAT_ERR_NOMEM;
        }

        gitstat_str_trim(line);

        if (line[0] == '\0' || line[0] == ';' || line[0] == '#') {
            gitstat_free(line);
            if (line_end == nullptr) break;
            line_start = line_end + 1;
            continue;
        }

        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            size_t sec_len = strlen(line) - 2;
            if (sec_len < sizeof(section)) {
                strncpy(section, line + 1, sec_len);
                section[sec_len] = '\0';
                gitstat_str_trim(section);
            }
        } else {
            char *eq = strchr(line, '=');
            if (eq != nullptr) {
                *eq = '\0';
                char *key = line;
                char *val = eq + 1;
                gitstat_str_trim(key);
                gitstat_str_trim(val);
                handler(user_data, section, key, val);
            }
        }

        gitstat_free(line);
        if (line_end == nullptr) break;
        line_start = line_end + 1;
    }

    return GITSTAT_OK;
}

gitstat_error_t gitstat_ini_parse_file(const char *filename, gitstat_ini_handler_t handler, void *user_data) {
    if (filename == nullptr || handler == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    FILE *f = fopen(filename, "rb");
    if (f == nullptr) {
        return GITSTAT_ERR_IO;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        return GITSTAT_ERR_IO;
    }

    char *buf = (char *)gitstat_alloc((size_t)size + 1);
    if (buf == nullptr) {
        fclose(f);
        return GITSTAT_ERR_NOMEM;
    }

    size_t read_bytes = fread(buf, 1, (size_t)size, f);
    buf[read_bytes] = '\0';
    fclose(f);

    gitstat_error_t err = gitstat_ini_parse_string(buf, handler, user_data);
    gitstat_free(buf);
    return err;
}
