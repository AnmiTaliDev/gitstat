#ifndef GITSTAT_APP_CONTEXT_H
#define GITSTAT_APP_CONTEXT_H

#include "gitstat/cli/options.h"
#include "gitstat/config/config.h"

typedef struct gitstat_context {
    gitstat_cli_options_t cli;
    gitstat_config_t config;
    bool use_color;
} gitstat_context_t;

void gitstat_context_init(gitstat_context_t *ctx);
void gitstat_context_free(gitstat_context_t *ctx);
[[nodiscard]] gitstat_error_t gitstat_context_bootstrap(gitstat_context_t *ctx, int argc, char **argv);

#endif
