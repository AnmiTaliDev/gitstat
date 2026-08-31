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

    return GITSTAT_OK;
}

gitstat_provider_t gitstat_provider_gitlab_create(void) {
    return (gitstat_provider_t){
        .name = "gitlab",
        .type = GITSTAT_PROVIDER_GITLAB,
        .fetch_stats = gitlab_fetch_stats,
        .ctx = nullptr
    };
}
