#ifndef GITSTAT_MODEL_REPOSITORY_H
#define GITSTAT_MODEL_REPOSITORY_H

typedef struct gitstat_repository {
    char *name;
    char *description;
    char *updated_at;
    char *url;
} gitstat_repository_t;

void gitstat_repository_init(gitstat_repository_t *repo);
void gitstat_repository_free(gitstat_repository_t *repo);

#endif
