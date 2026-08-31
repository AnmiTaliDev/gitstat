#ifndef GITSTAT_PROVIDER_PROVIDER_H
#define GITSTAT_PROVIDER_PROVIDER_H

#include "gitstat/types.h"
#include "gitstat/error.h"
#include "gitstat/config/config.h"
#include "gitstat/model/user_stats.h"

typedef struct gitstat_provider {
    const char *name;
    gitstat_provider_type_t type;
    gitstat_error_t (*fetch_stats)(const struct gitstat_provider *self, const gitstat_config_t *cfg, gitstat_user_stats_t *out_stats);
    void *ctx;
} gitstat_provider_t;

[[nodiscard]] gitstat_provider_t gitstat_provider_create(gitstat_provider_type_t type);
[[nodiscard]] gitstat_error_t gitstat_provider_fetch(const gitstat_provider_t *provider, const gitstat_config_t *cfg, gitstat_user_stats_t *out_stats);

#endif
