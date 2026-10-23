#include "gitstat/model/repository.h"
#include "gitstat/util/memory.h"

void gitstat_repository_init(gitstat_repository_t *repo) {
    if (repo == nullptr) {
        return;
    }
    repo->owner = nullptr;
    repo->name = nullptr;
    repo->full_name = nullptr;
    repo->description = nullptr;
    repo->language = nullptr;
    repo->license = nullptr;
    repo->default_branch = nullptr;
    repo->updated_at = nullptr;
    repo->url = nullptr;
    repo->stars_count = 0;
    repo->forks_count = 0;
    repo->open_issues_count = 0;
    repo->watchers_count = 0;
    repo->is_fork = false;
    repo->is_private = false;
}

void gitstat_repository_free(gitstat_repository_t *repo) {
    if (repo == nullptr) {
        return;
    }
    gitstat_free(repo->owner);
    gitstat_free(repo->name);
    gitstat_free(repo->full_name);
    gitstat_free(repo->description);
    gitstat_free(repo->language);
    gitstat_free(repo->license);
    gitstat_free(repo->default_branch);
    gitstat_free(repo->updated_at);
    gitstat_free(repo->url);
    repo->owner = nullptr;
    repo->name = nullptr;
    repo->full_name = nullptr;
    repo->description = nullptr;
    repo->language = nullptr;
    repo->license = nullptr;
    repo->default_branch = nullptr;
    repo->updated_at = nullptr;
    repo->url = nullptr;
    repo->stars_count = 0;
    repo->forks_count = 0;
    repo->open_issues_count = 0;
    repo->watchers_count = 0;
    repo->is_fork = false;
    repo->is_private = false;
}
