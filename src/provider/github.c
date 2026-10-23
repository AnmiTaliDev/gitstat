#include "gitstat/provider/github.h"
#include "gitstat/http/client.h"
#include "gitstat/json/json.h"
#include "gitstat/os/process.h"
#include "gitstat/util/buffer.h"
#include "gitstat/util/memory.h"
#include "gitstat/util/string.h"
#include "gitstat/util/time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *GITHUB_GRAPHQL_QUERY =
"query($login: String!, $limit: Int!) {"
"  user(login: $login) {"
"    login"
"    name"
"    organizations(first: 10) {"
"      nodes {"
"        login"
"      }"
"    }"
"    repositories(first: $limit, ownerAffiliations: OWNER, orderBy: {field: UPDATED_AT, direction: DESC}) {"
"      totalCount"
"      nodes {"
"        name"
"        description"
"        updatedAt"
"        url"
"      }"
"    }"
"    repositoriesContributedTo(first: $limit, contributionTypes: [COMMIT, PULL_REQUEST, REPOSITORY], orderBy: {field: PUSHED_AT, direction: DESC}) {"
"      nodes {"
"        name"
"        nameWithOwner"
"        description"
"        updatedAt"
"        url"
"      }"
"    }"
"    publicRepositories: repositories(privacy: PUBLIC, ownerAffiliations: OWNER) {"
"      totalCount"
"    }"
"    contributionsCollection {"
"      contributionCalendar {"
"        totalContributions"
"      }"
"      totalCommitContributions"
"      totalPullRequestContributions"
"      restrictedContributionsCount"
"    }"
"  }"
"}";

static char *detect_github_token(const gitstat_config_t *cfg) {
    if (cfg->github_token != nullptr && cfg->github_token[0] != '\0') {
        return gitstat_strdup(cfg->github_token);
    }
    char token_buf[512] = {0};
    if (gitstat_process_run_capture("gh auth token", token_buf, sizeof(token_buf)) == GITSTAT_OK) {
        gitstat_str_trim(token_buf);
        if (token_buf[0] != '\0') {
            return gitstat_strdup(token_buf);
        }
    }
    return nullptr;
}

static gitstat_error_t parse_github_graphql_response(const char *json_str, gitstat_user_stats_t *out_stats) {
    gitstat_error_t err = GITSTAT_OK;
    json_value_t *root = json_parse(json_str, 0, &err);
    if (root == nullptr) {
        return GITSTAT_ERR_JSON_PARSE;
    }

    json_value_t *data = json_object_get(root, "data");
    if (data == nullptr) {
        json_value_free(root);
        return GITSTAT_ERR_NOT_FOUND;
    }

    json_value_t *user = json_object_get(data, "user");
    if (user == nullptr || json_is_null(user)) {
        json_value_free(root);
        return GITSTAT_ERR_NOT_FOUND;
    }

    const char *login = json_object_get_string(user, "login");
    const char *name = json_object_get_string(user, "name");
    out_stats->login = gitstat_strdup(login ? login : "");
    out_stats->name = gitstat_strdup(name ? name : (login ? login : ""));

    json_value_t *orgs = json_object_get(user, "organizations");
    if (orgs != nullptr) {
        json_value_t *org_nodes = json_object_get(orgs, "nodes");
        if (org_nodes != nullptr && json_get_type(org_nodes) == JSON_TYPE_ARRAY) {
            size_t o_count = json_array_length(org_nodes);
            if (o_count > 0) {
                out_stats->organizations = (char **)gitstat_calloc(o_count, sizeof(char *));
                out_stats->organizations_count = o_count;
                for (size_t i = 0; i < o_count; ++i) {
                    json_value_t *o_node = json_array_get(org_nodes, i);
                    const char *o_login = json_object_get_string(o_node, "login");
                    out_stats->organizations[i] = gitstat_strdup(o_login ? o_login : "");
                }
            }
        }
    }

    json_value_t *public_repos = json_object_get(user, "publicRepositories");
    if (public_repos != nullptr) {
        out_stats->public_repos_count = json_object_get_int64(public_repos, "totalCount");
    }

    json_value_t *contrib = json_object_get(user, "contributionsCollection");
    if (contrib != nullptr && !json_is_null(contrib)) {
        out_stats->has_contributions_data = true;
        json_value_t *calendar = json_object_get(contrib, "contributionCalendar");
        if (calendar != nullptr) {
            out_stats->total_contributions = json_object_get_int64(calendar, "totalContributions");
        }
        out_stats->total_commits = json_object_get_int64(contrib, "totalCommitContributions");
        out_stats->total_pull_requests = json_object_get_int64(contrib, "totalPullRequestContributions");
        out_stats->restricted_contributions = json_object_get_int64(contrib, "restrictedContributionsCount");
    }

    json_value_t *contrib_repos = json_object_get(user, "repositoriesContributedTo");
    if (contrib_repos != nullptr) {
        json_value_t *c_nodes = json_object_get(contrib_repos, "nodes");
        if (c_nodes != nullptr && json_get_type(c_nodes) == JSON_TYPE_ARRAY) {
            size_t c_count = json_array_length(c_nodes);
            if (c_count > 0) {
                out_stats->contributed_repos = (gitstat_repository_t *)gitstat_calloc(c_count, sizeof(gitstat_repository_t));
                out_stats->contributed_repos_count = c_count;
                for (size_t i = 0; i < c_count; ++i) {
                    json_value_t *node = json_array_get(c_nodes, i);
                    if (node != nullptr) {
                        gitstat_repository_init(&out_stats->contributed_repos[i]);
                        const char *r_name = json_object_get_string(node, "name");
                        const char *r_full = json_object_get_string(node, "nameWithOwner");
                        const char *r_desc = json_object_get_string(node, "description");
                        const char *r_updated = json_object_get_string(node, "updatedAt");
                        const char *r_url = json_object_get_string(node, "url");

                        out_stats->contributed_repos[i].name = gitstat_strdup(r_name ? r_name : "");
                        out_stats->contributed_repos[i].full_name = gitstat_strdup(r_full ? r_full : (r_name ? r_name : ""));
                        out_stats->contributed_repos[i].description = gitstat_strdup(r_desc ? r_desc : "No description provided.");

                        char date_buf[64] = {0};
                        if (r_updated != nullptr) {
                            gitstat_time_format_iso_to_date(r_updated, date_buf, sizeof(date_buf));
                        }
                        out_stats->contributed_repos[i].updated_at = gitstat_strdup(date_buf);
                        out_stats->contributed_repos[i].url = gitstat_strdup(r_url ? r_url : "");
                    }
                }
            }
        }
    }

    json_value_t *repos = json_object_get(user, "repositories");
    if (repos != nullptr) {
        json_value_t *nodes = json_object_get(repos, "nodes");
        if (nodes != nullptr && json_get_type(nodes) == JSON_TYPE_ARRAY) {
            size_t count = json_array_length(nodes);
            out_stats->recent_repos = (gitstat_repository_t *)gitstat_calloc(count, sizeof(gitstat_repository_t));
            out_stats->recent_repos_count = count;

            for (size_t i = 0; i < count; ++i) {
                json_value_t *node = json_array_get(nodes, i);
                if (node != nullptr) {
                    gitstat_repository_init(&out_stats->recent_repos[i]);
                    const char *r_name = json_object_get_string(node, "name");
                    const char *r_desc = json_object_get_string(node, "description");
                    const char *r_updated = json_object_get_string(node, "updatedAt");
                    const char *r_url = json_object_get_string(node, "url");

                    out_stats->recent_repos[i].name = gitstat_strdup(r_name ? r_name : "");
                    out_stats->recent_repos[i].description = gitstat_strdup(r_desc ? r_desc : "No description provided.");
                    
                    char date_buf[64] = {0};
                    gitstat_time_format_iso_to_date(r_updated, date_buf, sizeof(date_buf));
                    out_stats->recent_repos[i].updated_at = gitstat_strdup(date_buf);
                    out_stats->recent_repos[i].url = gitstat_strdup(r_url ? r_url : "");
                }
            }
        }
    }

    json_value_free(root);
    return GITSTAT_OK;
}

static gitstat_error_t fetch_github_rest_fallback(const gitstat_config_t *cfg, const char *token, gitstat_user_stats_t *out_stats) {
    char url[512];
    snprintf(url, sizeof(url), "https://api.github.com/users/%s", cfg->username);

    gitstat_http_request_t user_req = {
        .url = url,
        .method = "GET",
        .post_data = nullptr,
        .token = token,
        .user_agent = "gitstat/1.0",
        .timeout_seconds = cfg->timeout_seconds
    };

    gitstat_http_response_t user_res;
    gitstat_http_response_init(&user_res);

    gitstat_error_t err = gitstat_http_execute(&user_req, &user_res);
    if (err != GITSTAT_OK || user_res.status_code != 200 || user_res.body.data == nullptr) {
        gitstat_http_response_free(&user_res);
        return (user_res.status_code == 404) ? GITSTAT_ERR_NOT_FOUND : GITSTAT_ERR_HTTP;
    }

    json_value_t *user_json = json_parse(user_res.body.data, user_res.body.size, &err);
    gitstat_http_response_free(&user_res);

    if (user_json == nullptr) {
        return GITSTAT_ERR_JSON_PARSE;
    }

    const char *login = json_object_get_string(user_json, "login");
    const char *name = json_object_get_string(user_json, "name");
    out_stats->login = gitstat_strdup(login ? login : cfg->username);
    out_stats->name = gitstat_strdup(name ? name : (login ? login : cfg->username));
    out_stats->public_repos_count = json_object_get_int64(user_json, "public_repos");
    out_stats->has_contributions_data = false;

    json_value_free(user_json);

    snprintf(url, sizeof(url), "https://api.github.com/users/%s/repos?sort=updated&per_page=%zu", cfg->username, cfg->repo_limit);

    gitstat_http_request_t repos_req = {
        .url = url,
        .method = "GET",
        .post_data = nullptr,
        .token = token,
        .user_agent = "gitstat/1.0",
        .timeout_seconds = cfg->timeout_seconds
    };

    gitstat_http_response_t repos_res;
    gitstat_http_response_init(&repos_res);

    err = gitstat_http_execute(&repos_req, &repos_res);
    if (err == GITSTAT_OK && repos_res.status_code == 200 && repos_res.body.data != nullptr) {
        json_value_t *repos_json = json_parse(repos_res.body.data, repos_res.body.size, &err);
        if (repos_json != nullptr && json_get_type(repos_json) == JSON_TYPE_ARRAY) {
            size_t count = json_array_length(repos_json);
            out_stats->recent_repos = (gitstat_repository_t *)gitstat_calloc(count, sizeof(gitstat_repository_t));
            out_stats->recent_repos_count = count;

            for (size_t i = 0; i < count; ++i) {
                json_value_t *node = json_array_get(repos_json, i);
                if (node != nullptr) {
                    gitstat_repository_init(&out_stats->recent_repos[i]);
                    const char *r_name = json_object_get_string(node, "name");
                    const char *r_desc = json_object_get_string(node, "description");
                    const char *r_updated = json_object_get_string(node, "updated_at");
                    const char *r_url = json_object_get_string(node, "html_url");

                    out_stats->recent_repos[i].name = gitstat_strdup(r_name ? r_name : "");
                    out_stats->recent_repos[i].description = gitstat_strdup(r_desc ? r_desc : "No description provided.");

                    char date_buf[64] = {0};
                    gitstat_time_format_iso_to_date(r_updated, date_buf, sizeof(date_buf));
                    out_stats->recent_repos[i].updated_at = gitstat_strdup(date_buf);
                    out_stats->recent_repos[i].url = gitstat_strdup(r_url ? r_url : "");
                }
            }
            json_value_free(repos_json);
        }
    }
    gitstat_http_response_free(&repos_res);

    snprintf(url, sizeof(url), "https://api.github.com/users/%s/orgs", cfg->username);
    repos_req.url = url;
    gitstat_http_response_init(&repos_res);
    err = gitstat_http_execute(&repos_req, &repos_res);
    if (err == GITSTAT_OK && repos_res.status_code == 200 && repos_res.body.data != nullptr) {
        json_value_t *orgs_json = json_parse(repos_res.body.data, repos_res.body.size, &err);
        if (orgs_json != nullptr && json_get_type(orgs_json) == JSON_TYPE_ARRAY) {
            size_t o_count = json_array_length(orgs_json);
            if (o_count > 0) {
                out_stats->organizations = (char **)gitstat_calloc(o_count, sizeof(char *));
                out_stats->organizations_count = o_count;
                for (size_t i = 0; i < o_count; ++i) {
                    json_value_t *o_node = json_array_get(orgs_json, i);
                    const char *o_login = json_object_get_string(o_node, "login");
                    out_stats->organizations[i] = gitstat_strdup(o_login ? o_login : "");
                }
            }
            json_value_free(orgs_json);
        }
    }
    gitstat_http_response_free(&repos_res);

    snprintf(url, sizeof(url), "https://api.github.com/users/%s/events/public?per_page=30", cfg->username);
    repos_req.url = url;
    gitstat_http_response_init(&repos_res);
    err = gitstat_http_execute(&repos_req, &repos_res);
    if (err == GITSTAT_OK && repos_res.status_code == 200 && repos_res.body.data != nullptr) {
        json_value_t *events_json = json_parse(repos_res.body.data, repos_res.body.size, &err);
        if (events_json != nullptr && json_get_type(events_json) == JSON_TYPE_ARRAY) {
            size_t e_count = json_array_length(events_json);
            size_t added = 0;
            size_t max_contrib = cfg->repo_limit > 0 ? cfg->repo_limit : 3;
            gitstat_repository_t *c_repos = (gitstat_repository_t *)gitstat_calloc(max_contrib, sizeof(gitstat_repository_t));
            for (size_t i = 0; i < e_count && added < max_contrib; ++i) {
                json_value_t *ev = json_array_get(events_json, i);
                json_value_t *r_obj = json_object_get(ev, "repo");
                if (r_obj == nullptr) continue;
                const char *r_full = json_object_get_string(r_obj, "name");
                if (r_full == nullptr || r_full[0] == '\0') continue;
                bool already = false;
                for (size_t j = 0; j < added; ++j) {
                    if (c_repos[j].full_name && strcmp(c_repos[j].full_name, r_full) == 0) {
                        already = true;
                        break;
                    }
                }
                if (already) continue;

                gitstat_repository_init(&c_repos[added]);
                c_repos[added].full_name = gitstat_strdup(r_full);
                const char *slash = strchr(r_full, '/');
                c_repos[added].name = gitstat_strdup(slash ? slash + 1 : r_full);
                c_repos[added].description = gitstat_strdup("Contributed repository");
                char r_url[256];
                snprintf(r_url, sizeof(r_url), "https://github.com/%s", r_full);
                c_repos[added].url = gitstat_strdup(r_url);
                const char *created = json_object_get_string(ev, "created_at");
                char date_buf[64] = {0};
                if (created != nullptr) {
                    gitstat_time_format_iso_to_date(created, date_buf, sizeof(date_buf));
                }
                c_repos[added].updated_at = gitstat_strdup(date_buf);
                added++;
            }
            if (added > 0) {
                out_stats->contributed_repos = c_repos;
                out_stats->contributed_repos_count = added;
            } else {
                gitstat_free(c_repos);
            }
            json_value_free(events_json);
        }
    }
    gitstat_http_response_free(&repos_res);

    return GITSTAT_OK;
}

static gitstat_error_t github_fetch_stats(const gitstat_provider_t *self, const gitstat_config_t *cfg, gitstat_user_stats_t *out_stats) {
    (void)self;
    if (cfg == nullptr || cfg->username == nullptr || out_stats == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    char *token = detect_github_token(cfg);

    char *escaped_login = gitstat_str_escape_json(cfg->username);
    char *escaped_query = gitstat_str_escape_json(GITHUB_GRAPHQL_QUERY);

    gitstat_buffer_t payload;
    gitstat_buffer_init(&payload);
    gitstat_buffer_append_fmt(&payload,
        "{\"query\":\"%s\",\"variables\":{\"login\":\"%s\",\"limit\":%zu}}",
        escaped_query, escaped_login, cfg->repo_limit);

    gitstat_free(escaped_login);
    gitstat_free(escaped_query);

    gitstat_http_request_t req = {
        .url = cfg->github_url ? cfg->github_url : "https://api.github.com/graphql",
        .method = "POST",
        .post_data = payload.data,
        .token = token,
        .user_agent = "gitstat/1.0",
        .timeout_seconds = cfg->timeout_seconds
    };

    gitstat_http_response_t res;
    gitstat_http_response_init(&res);

    gitstat_error_t err = gitstat_http_execute(&req, &res);
    gitstat_buffer_free(&payload);

    if (err == GITSTAT_OK && res.status_code == 200 && res.body.data != nullptr) {
        err = parse_github_graphql_response(res.body.data, out_stats);
        if (err == GITSTAT_OK) {
            gitstat_http_response_free(&res);
            gitstat_free(token);
            return GITSTAT_OK;
        }
    }

    gitstat_http_response_free(&res);

    err = fetch_github_rest_fallback(cfg, token, out_stats);
    gitstat_free(token);
    return err;
}

static gitstat_error_t github_fetch_repo_stats(const gitstat_provider_t *self, const gitstat_config_t *cfg, const char *repo, gitstat_repository_t *out_repo) {
    (void)self;
    if (cfg == nullptr || repo == nullptr || out_repo == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    char *token = detect_github_token(cfg);
    char url[512];
    snprintf(url, sizeof(url), "https://api.github.com/repos/%s", repo);

    gitstat_http_request_t req = {
        .url = url,
        .method = "GET",
        .post_data = nullptr,
        .token = token,
        .user_agent = "gitstat/1.0",
        .timeout_seconds = cfg->timeout_seconds
    };

    gitstat_http_response_t res;
    gitstat_http_response_init(&res);

    gitstat_error_t err = gitstat_http_execute(&req, &res);
    gitstat_free(token);

    if (err != GITSTAT_OK || res.status_code != 200 || res.body.data == nullptr) {
        gitstat_error_t result = (res.status_code == 404) ? GITSTAT_ERR_NOT_FOUND : GITSTAT_ERR_HTTP;
        gitstat_http_response_free(&res);
        return result;
    }

    json_value_t *root = json_parse(res.body.data, res.body.size, &err);
    gitstat_http_response_free(&res);
    if (root == nullptr) {
        return GITSTAT_ERR_JSON_PARSE;
    }

    const char *name = json_object_get_string(root, "name");
    const char *full_name = json_object_get_string(root, "full_name");
    const char *desc = json_object_get_string(root, "description");
    const char *html_url = json_object_get_string(root, "html_url");
    const char *lang = json_object_get_string(root, "language");
    const char *def_branch = json_object_get_string(root, "default_branch");
    const char *updated = json_object_get_string(root, "updated_at");

    out_repo->name = gitstat_strdup(name ? name : "");
    out_repo->full_name = gitstat_strdup(full_name ? full_name : "");
    out_repo->description = gitstat_strdup(desc ? desc : "No description provided.");
    out_repo->url = gitstat_strdup(html_url ? html_url : "");
    out_repo->language = gitstat_strdup(lang ? lang : "");
    out_repo->default_branch = gitstat_strdup(def_branch ? def_branch : "");

    char date_buf[64] = {0};
    if (updated != nullptr) {
        gitstat_time_format_iso_to_date(updated, date_buf, sizeof(date_buf));
    }
    out_repo->updated_at = gitstat_strdup(date_buf);

    out_repo->stars_count = json_object_get_int64(root, "stargazers_count");
    out_repo->forks_count = json_object_get_int64(root, "forks_count");
    out_repo->open_issues_count = json_object_get_int64(root, "open_issues_count");
    int64_t subscribers = json_object_get_int64(root, "subscribers_count");
    out_repo->watchers_count = subscribers > 0 ? subscribers : json_object_get_int64(root, "watchers_count");
    out_repo->is_fork = json_object_get_bool(root, "fork");
    out_repo->is_private = json_object_get_bool(root, "private");

    json_value_t *owner_obj = json_object_get(root, "owner");
    if (owner_obj != nullptr) {
        const char *owner_login = json_object_get_string(owner_obj, "login");
        out_repo->owner = gitstat_strdup(owner_login ? owner_login : "");
    }

    json_value_t *license_obj = json_object_get(root, "license");
    if (license_obj != nullptr && !json_is_null(license_obj)) {
        const char *spdx = json_object_get_string(license_obj, "spdx_id");
        const char *lic_name = json_object_get_string(license_obj, "name");
        if (spdx != nullptr && spdx[0] != '\0' && strcmp(spdx, "NOASSERTION") != 0) {
            out_repo->license = gitstat_strdup(spdx);
        } else if (lic_name != nullptr && lic_name[0] != '\0') {
            out_repo->license = gitstat_strdup(lic_name);
        }
    }

    json_value_free(root);
    return GITSTAT_OK;
}

gitstat_provider_t gitstat_provider_github_create(void) {
    return (gitstat_provider_t){
        .name = "github",
        .type = GITSTAT_PROVIDER_GITHUB,
        .fetch_stats = github_fetch_stats,
        .fetch_repo_stats = github_fetch_repo_stats,
        .ctx = nullptr
    };
}
