#include "gitstat/model/user_stats.h"
#include "gitstat/util/memory.h"

void gitstat_user_stats_init(gitstat_user_stats_t *stats) {
    if (stats == nullptr) {
        return;
    }
    stats->login = nullptr;
    stats->name = nullptr;
    stats->total_contributions = 0;
    stats->total_commits = 0;
    stats->total_pull_requests = 0;
    stats->restricted_contributions = 0;
    stats->public_repos_count = 0;
    stats->recent_repos = nullptr;
    stats->recent_repos_count = 0;
    stats->has_contributions_data = false;
}

void gitstat_user_stats_free(gitstat_user_stats_t *stats) {
    if (stats == nullptr) {
        return;
    }
    gitstat_free(stats->login);
    gitstat_free(stats->name);
    if (stats->recent_repos != nullptr) {
        for (size_t i = 0; i < stats->recent_repos_count; ++i) {
            gitstat_repository_free(&stats->recent_repos[i]);
        }
        gitstat_free(stats->recent_repos);
    }
    stats->login = nullptr;
    stats->name = nullptr;
    stats->recent_repos = nullptr;
    stats->recent_repos_count = 0;
}
