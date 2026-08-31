#include "gitstat/provider/provider.h"
#include "gitstat/provider/github.h"
#include "gitstat/provider/gitlab.h"

gitstat_provider_t gitstat_provider_create(gitstat_provider_type_t type) {
    if (type == GITSTAT_PROVIDER_GITLAB) {
        return gitstat_provider_gitlab_create();
    }
    return gitstat_provider_github_create();
}

gitstat_error_t gitstat_provider_fetch(const gitstat_provider_t *provider, const gitstat_config_t *cfg, gitstat_user_stats_t *out_stats) {
    if (provider == nullptr || provider->fetch_stats == nullptr || cfg == nullptr || out_stats == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }
    return provider->fetch_stats(provider, cfg, out_stats);
}
