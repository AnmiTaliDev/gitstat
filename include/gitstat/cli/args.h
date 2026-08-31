#ifndef GITSTAT_CLI_ARGS_H
#define GITSTAT_CLI_ARGS_H

#include "gitstat/error.h"
#include "gitstat/cli/options.h"

void gitstat_cli_options_init(gitstat_cli_options_t *opts);
void gitstat_cli_options_free(gitstat_cli_options_t *opts);
[[nodiscard]] gitstat_error_t gitstat_cli_parse(int argc, char **argv, gitstat_cli_options_t *opts);
void gitstat_cli_print_help(const char *prog_name);
void gitstat_cli_print_version(void);

#endif
