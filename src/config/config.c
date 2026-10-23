#include "gitstat/config/config.h"
#include "gitstat/config/defaults.h"
#include "gitstat/config/ini.h"
#include "gitstat/os/paths.h"
#include "gitstat/os/env.h"
#include "gitstat/util/memory.h"
#include "gitstat/util/string.h"
#include <stdlib.h>
#include <string.h>

void gitstat_config_init(gitstat_config_t *cfg) {
    if (cfg == nullptr) {
        return;
    }
    cfg->username = nullptr;
    cfg->repo = nullptr;
    cfg->provider = GITSTAT_PROVIDER_GITHUB;
    cfg->github_token = nullptr;
    cfg->gitlab_token = nullptr;
    cfg->github_url = gitstat_strdup(GITSTAT_DEFAULT_GITHUB_API_URL);
    cfg->gitlab_url = gitstat_strdup(GITSTAT_DEFAULT_GITLAB_API_URL);
    cfg->repo_limit = GITSTAT_DEFAULT_LIMIT;
    cfg->color_mode = GITSTAT_COLOR_AUTO;
    cfg->show_contributions = true;
    cfg->show_commits = true;
    cfg->show_pull_requests = true;
    cfg->show_restricted = true;
    cfg->show_public_repos = true;
    cfg->show_recent_repos = true;
    cfg->timeout_seconds = GITSTAT_DEFAULT_TIMEOUT;
}

void gitstat_config_free(gitstat_config_t *cfg) {
    if (cfg == nullptr) {
        return;
    }
    gitstat_free(cfg->username);
    gitstat_free(cfg->repo);
    gitstat_free(cfg->github_token);
    gitstat_free(cfg->gitlab_token);
    gitstat_free(cfg->github_url);
    gitstat_free(cfg->gitlab_url);
    cfg->username = nullptr;
    cfg->repo = nullptr;
    cfg->github_token = nullptr;
    cfg->gitlab_token = nullptr;
    cfg->github_url = nullptr;
    cfg->gitlab_url = nullptr;
}

static bool parse_bool(const char *val, bool def) {
    if (val == nullptr) return def;
    if (gitstat_str_equals_ignore_case(val, "true") ||
        gitstat_str_equals_ignore_case(val, "1") ||
        gitstat_str_equals_ignore_case(val, "yes") ||
        gitstat_str_equals_ignore_case(val, "on")) {
        return true;
    }
    if (gitstat_str_equals_ignore_case(val, "false") ||
        gitstat_str_equals_ignore_case(val, "0") ||
        gitstat_str_equals_ignore_case(val, "no") ||
        gitstat_str_equals_ignore_case(val, "off")) {
        return false;
    }
    return def;
}

static void ini_config_callback(void *user_data, const char *section, const char *key, const char *value) {
    gitstat_config_t *cfg = (gitstat_config_t *)user_data;

    if (strcmp(section, "general") == 0 || strcmp(section, "") == 0) {
        if (strcmp(key, "provider") == 0) {
            if (gitstat_str_equals_ignore_case(value, "gitlab")) {
                cfg->provider = GITSTAT_PROVIDER_GITLAB;
            } else {
                cfg->provider = GITSTAT_PROVIDER_GITHUB;
            }
        } else if (strcmp(key, "user") == 0 || strcmp(key, "username") == 0) {
            gitstat_free(cfg->username);
            cfg->username = gitstat_strdup(value);
        } else if (strcmp(key, "repo") == 0 || strcmp(key, "repository") == 0) {
            gitstat_free(cfg->repo);
            cfg->repo = gitstat_strdup(value);
        } else if (strcmp(key, "color") == 0) {
            if (gitstat_str_equals_ignore_case(value, "always")) {
                cfg->color_mode = GITSTAT_COLOR_ALWAYS;
            } else if (gitstat_str_equals_ignore_case(value, "never")) {
                cfg->color_mode = GITSTAT_COLOR_NEVER;
            } else {
                cfg->color_mode = GITSTAT_COLOR_AUTO;
            }
        } else if (strcmp(key, "repo_limit") == 0 || strcmp(key, "limit") == 0) {
            int lim = atoi(value);
            if (lim >= 0) {
                cfg->repo_limit = (size_t)lim;
            }
        } else if (strcmp(key, "timeout") == 0) {
            long t = atol(value);
            if (t > 0) {
                cfg->timeout_seconds = t;
            }
        }
    } else if (strcmp(section, "github") == 0) {
        if (strcmp(key, "token") == 0) {
            gitstat_free(cfg->github_token);
            cfg->github_token = gitstat_strdup(value);
        } else if (strcmp(key, "url") == 0) {
            gitstat_free(cfg->github_url);
            cfg->github_url = gitstat_strdup(value);
        }
    } else if (strcmp(section, "gitlab") == 0) {
        if (strcmp(key, "token") == 0) {
            gitstat_free(cfg->gitlab_token);
            cfg->gitlab_token = gitstat_strdup(value);
        } else if (strcmp(key, "url") == 0) {
            gitstat_free(cfg->gitlab_url);
            cfg->gitlab_url = gitstat_strdup(value);
        }
    } else if (strcmp(section, "display") == 0) {
        if (strcmp(key, "show_contributions") == 0) {
            cfg->show_contributions = parse_bool(value, cfg->show_contributions);
        } else if (strcmp(key, "show_commits") == 0) {
            cfg->show_commits = parse_bool(value, cfg->show_commits);
        } else if (strcmp(key, "show_pull_requests") == 0 || strcmp(key, "show_prs") == 0) {
            cfg->show_pull_requests = parse_bool(value, cfg->show_pull_requests);
        } else if (strcmp(key, "show_restricted") == 0) {
            cfg->show_restricted = parse_bool(value, cfg->show_restricted);
        } else if (strcmp(key, "show_public_repos") == 0 || strcmp(key, "show_repos") == 0) {
            cfg->show_public_repos = parse_bool(value, cfg->show_public_repos);
        } else if (strcmp(key, "show_recent_repos") == 0 || strcmp(key, "show_recent") == 0) {
            cfg->show_recent_repos = parse_bool(value, cfg->show_recent_repos);
        }
    }
}

gitstat_error_t gitstat_config_load(gitstat_config_t *cfg, const char *custom_path) {
    if (cfg == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    char path[1024];
    if (custom_path != nullptr && custom_path[0] != '\0') {
        strncpy(path, custom_path, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    } else {
        if (gitstat_paths_get_config_path(path, sizeof(path)) != GITSTAT_OK) {
            path[0] = '\0';
        }
    }

    if (path[0] != '\0' && gitstat_paths_file_exists(path)) {
        (void)gitstat_ini_parse_file(path, ini_config_callback, cfg);
    }

    const char *env_gh_token = gitstat_env_get("GITHUB_TOKEN");
    if (env_gh_token == nullptr) {
        env_gh_token = gitstat_env_get("GH_TOKEN");
    }
    if (env_gh_token != nullptr) {
        gitstat_free(cfg->github_token);
        cfg->github_token = gitstat_strdup(env_gh_token);
    }

    const char *env_gl_token = gitstat_env_get("GITLAB_TOKEN");
    if (env_gl_token == nullptr) {
        env_gl_token = gitstat_env_get("GL_TOKEN");
    }
    if (env_gl_token != nullptr) {
        gitstat_free(cfg->gitlab_token);
        cfg->gitlab_token = gitstat_strdup(env_gl_token);
    }

    const char *env_provider = gitstat_env_get("GITSTAT_PROVIDER");
    if (env_provider != nullptr) {
        if (gitstat_str_equals_ignore_case(env_provider, "gitlab")) {
            cfg->provider = GITSTAT_PROVIDER_GITLAB;
        } else {
            cfg->provider = GITSTAT_PROVIDER_GITHUB;
        }
    }

    const char *env_color = gitstat_env_get("GITSTAT_COLOR");
    if (env_color != nullptr) {
        if (gitstat_str_equals_ignore_case(env_color, "always")) {
            cfg->color_mode = GITSTAT_COLOR_ALWAYS;
        } else if (gitstat_str_equals_ignore_case(env_color, "never")) {
            cfg->color_mode = GITSTAT_COLOR_NEVER;
        } else {
            cfg->color_mode = GITSTAT_COLOR_AUTO;
        }
    }

    return GITSTAT_OK;
}
