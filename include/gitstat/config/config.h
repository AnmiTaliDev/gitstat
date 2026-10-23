#ifndef GITSTAT_CONFIG_CONFIG_H
#define GITSTAT_CONFIG_CONFIG_H

#include <stddef.h>
#include <stdbool.h>
#include "gitstat/types.h"
#include "gitstat/error.h"

typedef struct gitstat_config {
    char *username;
    char *repo;
    gitstat_provider_type_t provider;
    char *github_token;
    char *gitlab_token;
    char *github_url;
    char *gitlab_url;
    size_t repo_limit;
    gitstat_color_mode_t color_mode;
    bool show_contributions;
    bool show_commits;
    bool show_pull_requests;
    bool show_restricted;
    bool show_public_repos;
    bool show_recent_repos;
    long timeout_seconds;
} gitstat_config_t;

void gitstat_config_init(gitstat_config_t *cfg);
void gitstat_config_free(gitstat_config_t *cfg);
[[nodiscard]] gitstat_error_t gitstat_config_load(gitstat_config_t *cfg, const char *custom_path);

#endif
