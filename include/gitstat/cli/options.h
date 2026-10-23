#ifndef GITSTAT_CLI_OPTIONS_H
#define GITSTAT_CLI_OPTIONS_H

#include <stddef.h>
#include <stdbool.h>
#include "gitstat/types.h"

typedef struct gitstat_cli_options {
    char *username;
    char *repo;
    char *provider_str;
    char *token;
    char *url;
    char *config_file;
    int repo_limit;
    gitstat_color_mode_t color_mode;
    bool show_help;
    bool show_version;
    bool dump_json;
} gitstat_cli_options_t;

#endif
