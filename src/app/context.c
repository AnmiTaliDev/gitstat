#include "gitstat/app/context.h"
#include "gitstat/cli/args.h"
#include "gitstat/os/terminal.h"
#include "gitstat/os/env.h"
#include "gitstat/os/process.h"
#include "gitstat/util/string.h"
#include "gitstat/util/memory.h"
#include <stdio.h>
#include <string.h>

void gitstat_context_init(gitstat_context_t *ctx) {
    if (ctx == nullptr) {
        return;
    }
    gitstat_cli_options_init(&ctx->cli);
    gitstat_config_init(&ctx->config);
    ctx->use_color = false;
}

void gitstat_context_free(gitstat_context_t *ctx) {
    if (ctx == nullptr) {
        return;
    }
    gitstat_cli_options_free(&ctx->cli);
    gitstat_config_free(&ctx->config);
}

static char *detect_git_user(void) {
    char buf[256] = {0};
    if (gitstat_process_run_capture("git config user.name", buf, sizeof(buf)) == GITSTAT_OK) {
        gitstat_str_trim(buf);
        if (buf[0] != '\0') {
            return gitstat_strdup(buf);
        }
    }
    return nullptr;
}

gitstat_error_t gitstat_context_bootstrap(gitstat_context_t *ctx, int argc, char **argv) {
    if (ctx == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    gitstat_terminal_init();

    gitstat_error_t err = gitstat_cli_parse(argc, argv, &ctx->cli);
    if (err != GITSTAT_OK) {
        return err;
    }

    if (ctx->cli.show_help || ctx->cli.show_version) {
        return GITSTAT_OK;
    }

    err = gitstat_config_load(&ctx->config, ctx->cli.config_file);
    if (err != GITSTAT_OK && err != GITSTAT_ERR_NOT_FOUND) {
        return err;
    }

    if (ctx->cli.provider_str != nullptr) {
        if (gitstat_str_equals_ignore_case(ctx->cli.provider_str, "gitlab")) {
            ctx->config.provider = GITSTAT_PROVIDER_GITLAB;
        } else {
            ctx->config.provider = GITSTAT_PROVIDER_GITHUB;
        }
    }

    if (ctx->cli.token != nullptr) {
        if (ctx->config.provider == GITSTAT_PROVIDER_GITLAB) {
            gitstat_free(ctx->config.gitlab_token);
            ctx->config.gitlab_token = gitstat_strdup(ctx->cli.token);
        } else {
            gitstat_free(ctx->config.github_token);
            ctx->config.github_token = gitstat_strdup(ctx->cli.token);
        }
    }

    if (ctx->cli.url != nullptr) {
        if (ctx->config.provider == GITSTAT_PROVIDER_GITLAB) {
            gitstat_free(ctx->config.gitlab_url);
            ctx->config.gitlab_url = gitstat_strdup(ctx->cli.url);
        } else {
            gitstat_free(ctx->config.github_url);
            ctx->config.github_url = gitstat_strdup(ctx->cli.url);
        }
    }

    if (ctx->cli.repo_limit >= 0) {
        ctx->config.repo_limit = (size_t)ctx->cli.repo_limit;
    }

    if (ctx->cli.color_mode != GITSTAT_COLOR_AUTO) {
        ctx->config.color_mode = ctx->cli.color_mode;
    }

    if (ctx->cli.username != nullptr && ctx->cli.username[0] != '\0') {
        gitstat_free(ctx->config.username);
        ctx->config.username = gitstat_strdup(ctx->cli.username);
    } else if (ctx->config.username == nullptr || ctx->config.username[0] == '\0') {
        const char *env_user = gitstat_env_detect_username();
        if (env_user != nullptr && env_user[0] != '\0') {
            gitstat_free(ctx->config.username);
            ctx->config.username = gitstat_strdup(env_user);
        } else {
            char *git_user = detect_git_user();
            if (git_user != nullptr) {
                gitstat_free(ctx->config.username);
                ctx->config.username = git_user;
            }
        }
    }

    ctx->use_color = gitstat_terminal_supports_color(stdout, ctx->config.color_mode);

    return GITSTAT_OK;
}
