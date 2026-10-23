#include "gitstat/cli/args.h"
#include "gitstat/version.h"
#include "gitstat/util/string.h"
#include "gitstat/util/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void add_target(gitstat_cli_options_t *opts, const char *val, bool is_repo) {
    if (opts == nullptr || val == nullptr || val[0] == '\0') {
        return;
    }
    gitstat_target_t *new_targets = (gitstat_target_t *)gitstat_realloc(
        opts->targets, (opts->targets_count + 1) * sizeof(gitstat_target_t));
    if (new_targets == nullptr) {
        return;
    }
    opts->targets = new_targets;
    opts->targets[opts->targets_count].value = gitstat_strdup(val);
    opts->targets[opts->targets_count].is_repo = is_repo;
    opts->targets_count++;
}

static void add_targets_split(gitstat_cli_options_t *opts, const char *input, bool force_repo) {
    if (opts == nullptr || input == nullptr) {
        return;
    }
    char *copy = gitstat_strdup(input);
    if (copy == nullptr) {
        return;
    }
    char *token = strtok(copy, ",");
    while (token != nullptr) {
        gitstat_str_trim(token);
        if (token[0] != '\0') {
            bool is_repo = force_repo || (strchr(token, '/') != nullptr);
            add_target(opts, token, is_repo);
        }
        token = strtok(nullptr, ",");
    }
    gitstat_free(copy);
}

void gitstat_cli_options_init(gitstat_cli_options_t *opts) {
    if (opts == nullptr) {
        return;
    }
    opts->targets = nullptr;
    opts->targets_count = 0;
    opts->username = nullptr;
    opts->repo = nullptr;
    opts->provider_str = nullptr;
    opts->token = nullptr;
    opts->url = nullptr;
    opts->config_file = nullptr;
    opts->repo_limit = -1;
    opts->color_mode = GITSTAT_COLOR_AUTO;
    opts->show_help = false;
    opts->show_version = false;
    opts->dump_json = false;
}

void gitstat_cli_options_free(gitstat_cli_options_t *opts) {
    if (opts == nullptr) {
        return;
    }
    if (opts->targets != nullptr) {
        for (size_t i = 0; i < opts->targets_count; ++i) {
            gitstat_free(opts->targets[i].value);
        }
        gitstat_free(opts->targets);
        opts->targets = nullptr;
        opts->targets_count = 0;
    }
    gitstat_free(opts->username);
    gitstat_free(opts->repo);
    gitstat_free(opts->provider_str);
    gitstat_free(opts->token);
    gitstat_free(opts->url);
    gitstat_free(opts->config_file);
    opts->username = nullptr;
    opts->repo = nullptr;
    opts->provider_str = nullptr;
    opts->token = nullptr;
    opts->url = nullptr;
    opts->config_file = nullptr;
}

void gitstat_cli_print_version(void) {
    printf("gitstat version %s\n", gitstat_get_version_string());
}

void gitstat_cli_print_help(const char *prog_name) {
    const char *name = (prog_name != nullptr && prog_name[0] != '\0') ? prog_name : "gitstat";
    printf("Usage: %s [OPTIONS] [USER_OR_REPO...]\n\n", name);
    printf("Options:\n");
    printf("  -u, --user <name>       Target user login\n");
    printf("  -r, --repo <owner/repo> Target repository\n");
    printf("  -p, --provider <name>   Provider: github or gitlab\n");
    printf("  -t, --token <token>     API authentication token\n");
    printf("      --url <url>         API endpoint URL\n");
    printf("  -l, --limit <n>         Number of recent repositories to show\n");
    printf("  -c, --config <file>     Path to configuration file\n");
    printf("      --color <mode>      Color mode: auto, always, never\n");
    printf("      --json              Output raw JSON stats\n");
    printf("  -h, --help              Show help information\n");
    printf("  -v, --version           Show version information\n");
}

gitstat_error_t gitstat_cli_parse(int argc, char **argv, gitstat_cli_options_t *opts) {
    if (opts == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            opts->show_help = true;
            return GITSTAT_OK;
        }
        if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            opts->show_version = true;
            return GITSTAT_OK;
        }
        if (strcmp(arg, "--json") == 0) {
            opts->dump_json = true;
            continue;
        }

        if (strcmp(arg, "-u") == 0 || strcmp(arg, "--user") == 0) {
            if (i + 1 < argc) {
                const char *val = argv[++i];
                if (opts->username == nullptr) {
                    opts->username = gitstat_strdup(val);
                }
                add_targets_split(opts, val, false);
            }
            continue;
        }

        if (strcmp(arg, "-r") == 0 || strcmp(arg, "--repo") == 0) {
            if (i + 1 < argc) {
                const char *val = argv[++i];
                if (opts->repo == nullptr) {
                    opts->repo = gitstat_strdup(val);
                }
                add_targets_split(opts, val, true);
            }
            continue;
        }

        if (strcmp(arg, "-p") == 0 || strcmp(arg, "--provider") == 0) {
            if (i + 1 < argc) {
                gitstat_free(opts->provider_str);
                opts->provider_str = gitstat_strdup(argv[++i]);
            }
            continue;
        }

        if (strcmp(arg, "-t") == 0 || strcmp(arg, "--token") == 0) {
            if (i + 1 < argc) {
                gitstat_free(opts->token);
                opts->token = gitstat_strdup(argv[++i]);
            }
            continue;
        }

        if (strcmp(arg, "--url") == 0) {
            if (i + 1 < argc) {
                gitstat_free(opts->url);
                opts->url = gitstat_strdup(argv[++i]);
            }
            continue;
        }

        if (strcmp(arg, "-l") == 0 || strcmp(arg, "--limit") == 0) {
            if (i + 1 < argc) {
                opts->repo_limit = atoi(argv[++i]);
            }
            continue;
        }

        if (strcmp(arg, "-c") == 0 || strcmp(arg, "--config") == 0) {
            if (i + 1 < argc) {
                gitstat_free(opts->config_file);
                opts->config_file = gitstat_strdup(argv[++i]);
            }
            continue;
        }

        if (strcmp(arg, "--color") == 0) {
            if (i + 1 < argc) {
                const char *val = argv[++i];
                if (gitstat_str_equals_ignore_case(val, "always")) {
                    opts->color_mode = GITSTAT_COLOR_ALWAYS;
                } else if (gitstat_str_equals_ignore_case(val, "never")) {
                    opts->color_mode = GITSTAT_COLOR_NEVER;
                } else {
                    opts->color_mode = GITSTAT_COLOR_AUTO;
                }
            }
            continue;
        }

        if (arg[0] != '-') {
            bool is_repo = (strchr(arg, '/') != nullptr);
            if (is_repo && opts->repo == nullptr) {
                opts->repo = gitstat_strdup(arg);
            } else if (!is_repo && opts->username == nullptr) {
                opts->username = gitstat_strdup(arg);
            }
            add_targets_split(opts, arg, is_repo);
        }
    }

    return GITSTAT_OK;
}
