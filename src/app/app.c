#include "gitstat/app/app.h"
#include "gitstat/cli/args.h"
#include "gitstat/http/http.h"
#include "gitstat/provider/provider.h"
#include "gitstat/render/formatter.h"
#include "gitstat/render/printer.h"
#include "gitstat/render/theme.h"
#include <stdio.h>

int gitstat_app_run(gitstat_context_t *ctx) {
    if (ctx == nullptr) {
        return 1;
    }

    if (ctx->cli.show_help) {
        gitstat_cli_print_help("gitstat");
        return 0;
    }

    if (ctx->cli.show_version) {
        gitstat_cli_print_version();
        return 0;
    }

    gitstat_theme_t theme = gitstat_theme_get(ctx->use_color);

    if (ctx->cli.targets_count == 0) {
        gitstat_print_error(stderr, &theme, "Error: No target provided and none could be detected.");
        gitstat_cli_print_help("gitstat");
        return 1;
    }

    gitstat_error_t err = gitstat_http_global_init();
    if (err != GITSTAT_OK) {
        gitstat_print_error(stderr, &theme, "Error: Failed to initialize HTTP subsystem (%s).", gitstat_error_to_string(err));
        return 1;
    }

    gitstat_provider_t provider = gitstat_provider_create(ctx->config.provider);

    if (ctx->cli.dump_json) {
        if (ctx->cli.targets_count > 1) {
            fputs("[\n", stdout);
        }
        for (size_t t = 0; t < ctx->cli.targets_count; ++t) {
            const gitstat_target_t *target = &ctx->cli.targets[t];
            gitstat_buffer_t output;
            gitstat_buffer_init(&output);

            if (target->is_repo) {
                gitstat_repository_t repo_stats;
                gitstat_repository_init(&repo_stats);
                err = gitstat_provider_fetch_repo(&provider, &ctx->config, target->value, &repo_stats);
                if (err == GITSTAT_OK) {
                    (void)gitstat_format_repo_stats_json(&repo_stats, &output);
                }
                gitstat_repository_free(&repo_stats);
            } else {
                gitstat_user_stats_t stats;
                gitstat_user_stats_init(&stats);
                char *old_user = ctx->config.username;
                ctx->config.username = target->value;
                err = gitstat_provider_fetch(&provider, &ctx->config, &stats);
                ctx->config.username = old_user;
                if (err == GITSTAT_OK) {
                    (void)gitstat_format_stats_json(&stats, &output);
                }
                gitstat_user_stats_free(&stats);
            }

            if (output.data != nullptr) {
                fputs(output.data, stdout);
                if (ctx->cli.targets_count > 1 && t + 1 < ctx->cli.targets_count) {
                    fputs(",\n", stdout);
                }
            }
            gitstat_buffer_free(&output);
        }
        if (ctx->cli.targets_count > 1) {
            fputs("\n]\n", stdout);
        }
        gitstat_http_global_cleanup();
        return 0;
    }

    int exit_code = 0;
    for (size_t t = 0; t < ctx->cli.targets_count; ++t) {
        const gitstat_target_t *target = &ctx->cli.targets[t];
        if (t > 0) {
            putchar('\n');
        }

        gitstat_buffer_t output;
        gitstat_buffer_init(&output);

        if (target->is_repo) {
            gitstat_repository_t repo_stats;
            gitstat_repository_init(&repo_stats);
            err = gitstat_provider_fetch_repo(&provider, &ctx->config, target->value, &repo_stats);
            if (err != GITSTAT_OK) {
                gitstat_print_repo_not_found(stdout, &theme, target->value);
                exit_code = 1;
            } else {
                (void)gitstat_format_repo_stats(&repo_stats, &theme, &output);
                if (output.data != nullptr) {
                    fputs(output.data, stdout);
                }
            }
            gitstat_repository_free(&repo_stats);
        } else {
            gitstat_user_stats_t stats;
            gitstat_user_stats_init(&stats);
            char *old_user = ctx->config.username;
            ctx->config.username = target->value;
            err = gitstat_provider_fetch(&provider, &ctx->config, &stats);
            ctx->config.username = old_user;
            if (err != GITSTAT_OK) {
                gitstat_print_user_not_found(stdout, &theme, target->value);
                exit_code = 1;
            } else {
                (void)gitstat_format_stats(&stats, &ctx->config, &theme, &output);
                if (output.data != nullptr) {
                    fputs(output.data, stdout);
                }
            }
            gitstat_user_stats_free(&stats);
        }
        gitstat_buffer_free(&output);
    }

    gitstat_http_global_cleanup();
    return exit_code;
}
