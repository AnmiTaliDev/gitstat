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

    if (ctx->config.username == nullptr || ctx->config.username[0] == '\0') {
        gitstat_print_error(stderr, &theme, "Error: No target username provided and none could be detected.");
        gitstat_cli_print_help("gitstat");
        return 1;
    }

    gitstat_error_t err = gitstat_http_global_init();
    if (err != GITSTAT_OK) {
        gitstat_print_error(stderr, &theme, "Error: Failed to initialize HTTP subsystem (%s).", gitstat_error_to_string(err));
        return 1;
    }

    gitstat_provider_t provider = gitstat_provider_create(ctx->config.provider);
    gitstat_user_stats_t stats;
    gitstat_user_stats_init(&stats);

    err = gitstat_provider_fetch(&provider, &ctx->config, &stats);
    if (err != GITSTAT_OK) {
        gitstat_print_user_not_found(stdout, &theme, ctx->config.username);
        gitstat_user_stats_free(&stats);
        gitstat_http_global_cleanup();
        return 1;
    }

    gitstat_buffer_t output;
    gitstat_buffer_init(&output);

    if (ctx->cli.dump_json) {
        err = gitstat_format_stats_json(&stats, &output);
    } else {
        err = gitstat_format_stats(&stats, &ctx->config, &theme, &output);
    }

    if (err == GITSTAT_OK && output.data != nullptr) {
        fputs(output.data, stdout);
    }

    gitstat_buffer_free(&output);
    gitstat_user_stats_free(&stats);
    gitstat_http_global_cleanup();

    return 0;
}
