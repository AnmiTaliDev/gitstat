#include "gitstat/model/repository.h"
#include "gitstat/util/memory.h"

void gitstat_repository_init(gitstat_repository_t *repo) {
    if (repo == nullptr) {
        return;
    }
    repo->name = nullptr;
    repo->description = nullptr;
    repo->updated_at = nullptr;
    repo->url = nullptr;
}

void gitstat_repository_free(gitstat_repository_t *repo) {
    if (repo == nullptr) {
        return;
    }
    gitstat_free(repo->name);
    gitstat_free(repo->description);
    gitstat_free(repo->updated_at);
    gitstat_free(repo->url);
    repo->name = nullptr;
    repo->description = nullptr;
    repo->updated_at = nullptr;
    repo->url = nullptr;
}
