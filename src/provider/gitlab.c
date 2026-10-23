#include "gitstat/provider/gitlab.h"
#include "gitstat/http/client.h"
#include "gitstat/json/json.h"
#include "gitstat/os/env.h"
#include "gitstat/util/buffer.h"
#include "gitstat/util/memory.h"
#include "gitstat/util/string.h"
#include "gitstat/util/time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static gitstat_error_t gitlab_fetch_stats(const gitstat_provider_t *self, const gitstat_config_t *cfg, gitstat_user_stats_t *out_stats) {
    (void)self;
    if (cfg == nullptr || cfg->username == nullptr || out_stats == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    const char *base_url = "https://gitlab.com";
    char base_buf[256] = {0};
    if (cfg->gitlab_url != nullptr && cfg->gitlab_url[0] != '\0') {
        const char *api_pos = strstr(cfg->gitlab_url, "/api");
        if (api_pos != nullptr) {
            size_t blen = (size_t)(api_pos - cfg->gitlab_url);
            if (blen < sizeof(base_buf)) {
                strncpy(base_buf, cfg->gitlab_url, blen);
                base_buf[blen] = '\0';
                base_url = base_buf;
            }
        } else {
            base_url = cfg->gitlab_url;
        }
    }

    char url[512];
    snprintf(url, sizeof(url), "%s/api/v4/users?username=%s", base_url, cfg->username);

    gitstat_http_request_t req = {
        .url = url,
        .method = "GET",
        .post_data = nullptr,
        .token = cfg->gitlab_token,
        .user_agent = "gitstat/1.0",
        .timeout_seconds = cfg->timeout_seconds
    };

    gitstat_http_response_t res;
    gitstat_http_response_init(&res);

    gitstat_error_t err = gitstat_http_execute(&req, &res);
    if (err != GITSTAT_OK || res.status_code != 200 || res.body.data == nullptr) {
        gitstat_http_response_free(&res);
        return GITSTAT_ERR_NOT_FOUND;
    }

    json_value_t *users_json = json_parse(res.body.data, res.body.size, &err);
    gitstat_http_response_free(&res);

    if (users_json == nullptr || json_get_type(users_json) != JSON_TYPE_ARRAY || json_array_length(users_json) == 0) {
        if (users_json) json_value_free(users_json);
        return GITSTAT_ERR_NOT_FOUND;
    }

    json_value_t *user = json_array_get(users_json, 0);
    int64_t user_id = json_object_get_int64(user, "id");
    const char *uname = json_object_get_string(user, "username");
    const char *name = json_object_get_string(user, "name");

    out_stats->login = gitstat_strdup(uname ? uname : cfg->username);
    out_stats->name = gitstat_strdup(name ? name : (uname ? uname : cfg->username));

    json_value_free(users_json);

    snprintf(url, sizeof(url), "%s/api/v4/users/%lld/projects?order_by=updated_at&sort=desc&per_page=%zu",
             base_url, (long long)user_id, cfg->repo_limit);

    req.url = url;
    gitstat_http_response_init(&res);

    err = gitstat_http_execute(&req, &res);
    if (err == GITSTAT_OK && res.status_code == 200 && res.body.data != nullptr) {
        json_value_t *projects = json_parse(res.body.data, res.body.size, &err);
        if (projects != nullptr && json_get_type(projects) == JSON_TYPE_ARRAY) {
            size_t count = json_array_length(projects);
            out_stats->recent_repos = (gitstat_repository_t *)gitstat_calloc(count, sizeof(gitstat_repository_t));
            out_stats->recent_repos_count = count;
            out_stats->public_repos_count = (int64_t)count;

            for (size_t i = 0; i < count; ++i) {
                json_value_t *proj = json_array_get(projects, i);
                if (proj != nullptr) {
                    gitstat_repository_init(&out_stats->recent_repos[i]);
                    const char *p_name = json_object_get_string(proj, "name");
                    const char *p_desc = json_object_get_string(proj, "description");
                    const char *p_updated = json_object_get_string(proj, "last_activity_at");
                    const char *p_url = json_object_get_string(proj, "web_url");

                    out_stats->recent_repos[i].name = gitstat_strdup(p_name ? p_name : "");
                    out_stats->recent_repos[i].description = gitstat_strdup(p_desc ? p_desc : "No description provided.");

                    char date_buf[64] = {0};
                    gitstat_time_format_iso_to_date(p_updated, date_buf, sizeof(date_buf));
                    out_stats->recent_repos[i].updated_at = gitstat_strdup(date_buf);
                    out_stats->recent_repos[i].url = gitstat_strdup(p_url ? p_url : "");
                }
            }
            json_value_free(projects);
        }
    }
    gitstat_http_response_free(&res);

    snprintf(url, sizeof(url), "%s/api/v4/users/%lld/contributed_projects?order_by=updated_at&sort=desc&per_page=%zu",
             base_url, (long long)user_id, cfg->repo_limit);

    req.url = url;
    gitstat_http_response_init(&res);

    err = gitstat_http_execute(&req, &res);
    if (err == GITSTAT_OK && res.status_code == 200 && res.body.data != nullptr) {
        json_value_t *c_projects = json_parse(res.body.data, res.body.size, &err);
        if (c_projects != nullptr && json_get_type(c_projects) == JSON_TYPE_ARRAY) {
            size_t c_count = json_array_length(c_projects);
            if (c_count > 0) {
                out_stats->contributed_repos = (gitstat_repository_t *)gitstat_calloc(c_count, sizeof(gitstat_repository_t));
                out_stats->contributed_repos_count = c_count;

                for (size_t i = 0; i < c_count; ++i) {
                    json_value_t *proj = json_array_get(c_projects, i);
                    if (proj != nullptr) {
                        gitstat_repository_init(&out_stats->contributed_repos[i]);
                        const char *p_name = json_object_get_string(proj, "name");
                        const char *p_full = json_object_get_string(proj, "path_with_namespace");
                        const char *p_desc = json_object_get_string(proj, "description");
                        const char *p_updated = json_object_get_string(proj, "last_activity_at");
                        const char *p_url = json_object_get_string(proj, "web_url");

                        out_stats->contributed_repos[i].name = gitstat_strdup(p_name ? p_name : "");
                        out_stats->contributed_repos[i].full_name = gitstat_strdup(p_full ? p_full : (p_name ? p_name : ""));
                        out_stats->contributed_repos[i].description = gitstat_strdup(p_desc ? p_desc : "No description provided.");

                        char date_buf[64] = {0};
                        if (p_updated != nullptr) {
                            gitstat_time_format_iso_to_date(p_updated, date_buf, sizeof(date_buf));
                        }
                        out_stats->contributed_repos[i].updated_at = gitstat_strdup(date_buf);
                        out_stats->contributed_repos[i].url = gitstat_strdup(p_url ? p_url : "");

                        json_value_t *ns = json_object_get(proj, "namespace");
                        if (ns != nullptr) {
                            const char *kind = json_object_get_string(ns, "kind");
                            const char *ns_path = json_object_get_string(ns, "full_path");
                            if (kind != nullptr && strcmp(kind, "group") == 0 && ns_path != nullptr && ns_path[0] != '\0') {
                                bool exists = false;
                                for (size_t g = 0; g < out_stats->organizations_count; ++g) {
                                    if (strcmp(out_stats->organizations[g], ns_path) == 0) {
                                        exists = true;
                                        break;
                                    }
                                }
                                if (!exists) {
                                    char **new_orgs = (char **)gitstat_realloc(out_stats->organizations, (out_stats->organizations_count + 1) * sizeof(char *));
                                    if (new_orgs != nullptr) {
                                        out_stats->organizations = new_orgs;
                                        out_stats->organizations[out_stats->organizations_count] = gitstat_strdup(ns_path);
                                        out_stats->organizations_count++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            json_value_free(c_projects);
        }
    }
    gitstat_http_response_free(&res);

    return GITSTAT_OK;
}

static char *gitlab_url_encode_path(const char *path) {
    if (path == nullptr) return nullptr;
    gitstat_buffer_t buf;
    gitstat_buffer_init(&buf);
    for (size_t i = 0; path[i] != '\0'; ++i) {
        if (path[i] == '/') {
            gitstat_buffer_append_str(&buf, "%2F");
        } else {
            gitstat_buffer_append_char(&buf, path[i]);
        }
    }
    char *res = gitstat_strdup(buf.data ? buf.data : "");
    gitstat_buffer_free(&buf);
    return res;
}

static gitstat_error_t gitlab_fetch_repo_stats(const gitstat_provider_t *self, const gitstat_config_t *cfg, const char *repo, gitstat_repository_t *out_repo) {
    (void)self;
    if (cfg == nullptr || repo == nullptr || out_repo == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    const char *base_url = "https://gitlab.com";
    char base_buf[256] = {0};
    if (cfg->gitlab_url != nullptr && cfg->gitlab_url[0] != '\0') {
        const char *api_pos = strstr(cfg->gitlab_url, "/api");
        if (api_pos != nullptr) {
            size_t blen = (size_t)(api_pos - cfg->gitlab_url);
            if (blen < sizeof(base_buf)) {
                strncpy(base_buf, cfg->gitlab_url, blen);
                base_buf[blen] = '\0';
                base_url = base_buf;
            }
        } else {
            base_url = cfg->gitlab_url;
        }
    }

    char *enc_repo = gitlab_url_encode_path(repo);
    if (enc_repo == nullptr) {
        return GITSTAT_ERR_NOMEM;
    }

    char url[512];
    snprintf(url, sizeof(url), "%s/api/v4/projects/%s", base_url, enc_repo);

    gitstat_http_request_t req = {
        .url = url,
        .method = "GET",
        .post_data = nullptr,
        .token = cfg->gitlab_token,
        .user_agent = "gitstat/1.0",
        .timeout_seconds = cfg->timeout_seconds
    };

    gitstat_http_response_t res;
    gitstat_http_response_init(&res);

    gitstat_error_t err = gitstat_http_execute(&req, &res);
    if (err != GITSTAT_OK || res.status_code != 200 || res.body.data == nullptr) {
        gitstat_error_t result = (res.status_code == 404) ? GITSTAT_ERR_NOT_FOUND : GITSTAT_ERR_HTTP;
        gitstat_http_response_free(&res);
        gitstat_free(enc_repo);
        return result;
    }

    json_value_t *root = json_parse(res.body.data, res.body.size, &err);
    gitstat_http_response_free(&res);
    if (root == nullptr) {
        gitstat_free(enc_repo);
        return GITSTAT_ERR_JSON_PARSE;
    }

    const char *name = json_object_get_string(root, "name");
    const char *path_ns = json_object_get_string(root, "path_with_namespace");
    const char *desc = json_object_get_string(root, "description");
    const char *web_url = json_object_get_string(root, "web_url");
    const char *def_branch = json_object_get_string(root, "default_branch");
    const char *updated = json_object_get_string(root, "last_activity_at");
    const char *vis = json_object_get_string(root, "visibility");

    out_repo->name = gitstat_strdup(name ? name : "");
    out_repo->full_name = gitstat_strdup(path_ns ? path_ns : (name ? name : ""));
    out_repo->description = gitstat_strdup(desc ? desc : "No description provided.");
    out_repo->url = gitstat_strdup(web_url ? web_url : "");
    out_repo->default_branch = gitstat_strdup(def_branch ? def_branch : "");

    char date_buf[64] = {0};
    if (updated != nullptr) {
        gitstat_time_format_iso_to_date(updated, date_buf, sizeof(date_buf));
    }
    out_repo->updated_at = gitstat_strdup(date_buf);

    out_repo->stars_count = json_object_get_int64(root, "star_count");
    out_repo->forks_count = json_object_get_int64(root, "forks_count");
    out_repo->open_issues_count = json_object_get_int64(root, "open_issues_count");
    out_repo->is_private = (vis != nullptr && strcmp(vis, "private") == 0);
    out_repo->is_fork = (json_object_get(root, "forked_from_project") != nullptr);

    json_value_t *ns_obj = json_object_get(root, "namespace");
    if (ns_obj != nullptr) {
        const char *ns_path = json_object_get_string(ns_obj, "full_path");
        if (ns_path == nullptr) {
            ns_path = json_object_get_string(ns_obj, "path");
        }
        out_repo->owner = gitstat_strdup(ns_path ? ns_path : "");
    }

    json_value_free(root);

    snprintf(url, sizeof(url), "%s/api/v4/projects/%s/languages", base_url, enc_repo);
    gitstat_free(enc_repo);
    req.url = url;
    gitstat_http_response_init(&res);

    err = gitstat_http_execute(&req, &res);
    if (err == GITSTAT_OK && res.status_code == 200 && res.body.data != nullptr) {
        json_value_t *lang_root = json_parse(res.body.data, res.body.size, &err);
        if (lang_root != nullptr) {
            if (json_get_type(lang_root) == JSON_TYPE_OBJECT && lang_root->as.object_value.head != nullptr) {
                out_repo->language = gitstat_strdup(lang_root->as.object_value.head->key);
            }
            json_value_free(lang_root);
        }
    }
    gitstat_http_response_free(&res);

    return GITSTAT_OK;
}

gitstat_provider_t gitstat_provider_gitlab_create(void) {
    return (gitstat_provider_t){
        .name = "gitlab",
        .type = GITSTAT_PROVIDER_GITLAB,
        .fetch_stats = gitlab_fetch_stats,
        .fetch_repo_stats = gitlab_fetch_repo_stats,
        .ctx = nullptr
    };
}
