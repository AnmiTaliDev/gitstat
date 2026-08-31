#ifndef GITSTAT_MODEL_USER_STATS_H
#define GITSTAT_MODEL_USER_STATS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gitstat/model/repository.h"

typedef struct gitstat_user_stats {
    char *login;
    char *name;
    int64_t total_contributions;
    int64_t total_commits;
    int64_t total_pull_requests;
    int64_t restricted_contributions;
    int64_t public_repos_count;
    gitstat_repository_t *recent_repos;
    size_t recent_repos_count;
    bool has_contributions_data;
} gitstat_user_stats_t;

void gitstat_user_stats_init(gitstat_user_stats_t *stats);
void gitstat_user_stats_free(gitstat_user_stats_t *stats);

#endif
