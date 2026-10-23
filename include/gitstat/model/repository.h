#ifndef GITSTAT_MODEL_REPOSITORY_H
#define GITSTAT_MODEL_REPOSITORY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct gitstat_repository {
    char *owner;
    char *name;
    char *full_name;
    char *description;
    char *language;
    char *license;
    char *default_branch;
    char *updated_at;
    char *url;
    int64_t stars_count;
    int64_t forks_count;
    int64_t open_issues_count;
    int64_t watchers_count;
    bool is_fork;
    bool is_private;
} gitstat_repository_t;

void gitstat_repository_init(gitstat_repository_t *repo);
void gitstat_repository_free(gitstat_repository_t *repo);

#endif
