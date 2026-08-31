#include "gitstat/render/formatter.h"
#include "gitstat/util/string.h"
#include "gitstat/util/memory.h"
#include <stdio.h>

gitstat_error_t gitstat_format_stats(const gitstat_user_stats_t *stats, const gitstat_config_t *cfg, const gitstat_theme_t *theme, gitstat_buffer_t *out_buf) {
    if (stats == nullptr || cfg == nullptr || theme == nullptr || out_buf == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    gitstat_buffer_append_fmt(out_buf, " %s%s%s %s(@%s)%s\n",
        theme->title,
        (stats->name && stats->name[0] != '\0') ? stats->name : stats->login,
        theme->reset,
        theme->subtitle,
        stats->login,
        theme->reset
    );

    gitstat_buffer_append_fmt(out_buf, "%s-- Stats (Past Year) --%s\n",
        theme->header,
        theme->reset
    );

    if (cfg->show_contributions && stats->has_contributions_data) {
        gitstat_buffer_append_fmt(out_buf, "  %s Total Contributions:%s %lld\n",
            theme->stat_contributions,
            theme->reset,
            (long long)stats->total_contributions
        );
    }

    if (cfg->show_commits && stats->has_contributions_data) {
        gitstat_buffer_append_fmt(out_buf, "  %s Commits:%s             %lld\n",
            theme->stat_commits,
            theme->reset,
            (long long)stats->total_commits
        );
    }

    if (cfg->show_pull_requests && stats->has_contributions_data) {
        gitstat_buffer_append_fmt(out_buf, "  %s Pull Requests:%s       %lld\n",
            theme->stat_prs,
            theme->reset,
            (long long)stats->total_pull_requests
        );
    }

    if (cfg->show_restricted && stats->has_contributions_data) {
        gitstat_buffer_append_fmt(out_buf, "  %s Restricted:%s          %lld\n",
            theme->stat_restricted,
            theme->reset,
            (long long)stats->restricted_contributions
        );
    }

    if (cfg->show_public_repos) {
        gitstat_buffer_append_fmt(out_buf, "  %s Own Public Repos:%s    %lld\n",
            theme->stat_repos,
            theme->reset,
            (long long)stats->public_repos_count
        );
    }

    if (cfg->show_recent_repos && stats->recent_repos_count > 0) {
        gitstat_buffer_append_fmt(out_buf, "\n%s-- Recently Updated Repos --%s\n",
            theme->header,
            theme->reset
        );

        for (size_t i = 0; i < stats->recent_repos_count; ++i) {
            const gitstat_repository_t *repo = &stats->recent_repos[i];
            gitstat_buffer_append_fmt(out_buf, "  %s %s%s %s[%s]%s\n",
                theme->repo_name,
                repo->name ? repo->name : "",
                theme->reset,
                theme->dim,
                repo->updated_at ? repo->updated_at : "",
                theme->reset
            );
            gitstat_buffer_append_fmt(out_buf, "    %s%s%s\n",
                theme->dim,
                repo->description ? repo->description : "No description provided.",
                theme->reset
            );
            gitstat_buffer_append_fmt(out_buf, "    %s%s%s\n",
                theme->repo_url,
                repo->url ? repo->url : "",
                theme->reset
            );
            if (i + 1 < stats->recent_repos_count) {
                gitstat_buffer_append_char(out_buf, '\n');
            }
        }
    }

    return GITSTAT_OK;
}

gitstat_error_t gitstat_format_stats_json(const gitstat_user_stats_t *stats, gitstat_buffer_t *out_buf) {
    if (stats == nullptr || out_buf == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    char *esc_login = gitstat_str_escape_json(stats->login ? stats->login : "");
    char *esc_name = gitstat_str_escape_json(stats->name ? stats->name : "");

    gitstat_buffer_append_fmt(out_buf, "{\n");
    gitstat_buffer_append_fmt(out_buf, "  \"login\": \"%s\",\n", esc_login);
    gitstat_buffer_append_fmt(out_buf, "  \"name\": \"%s\",\n", esc_name);
    gitstat_buffer_append_fmt(out_buf, "  \"totalContributions\": %lld,\n", (long long)stats->total_contributions);
    gitstat_buffer_append_fmt(out_buf, "  \"totalCommitContributions\": %lld,\n", (long long)stats->total_commits);
    gitstat_buffer_append_fmt(out_buf, "  \"totalPullRequestContributions\": %lld,\n", (long long)stats->total_pull_requests);
    gitstat_buffer_append_fmt(out_buf, "  \"restrictedContributionsCount\": %lld,\n", (long long)stats->restricted_contributions);
    gitstat_buffer_append_fmt(out_buf, "  \"publicRepositoriesCount\": %lld,\n", (long long)stats->public_repos_count);
    gitstat_buffer_append_fmt(out_buf, "  \"repositories\": [\n");

    gitstat_free(esc_login);
    gitstat_free(esc_name);

    for (size_t i = 0; i < stats->recent_repos_count; ++i) {
        const gitstat_repository_t *repo = &stats->recent_repos[i];
        char *r_name = gitstat_str_escape_json(repo->name ? repo->name : "");
        char *r_desc = gitstat_str_escape_json(repo->description ? repo->description : "");
        char *r_updated = gitstat_str_escape_json(repo->updated_at ? repo->updated_at : "");
        char *r_url = gitstat_str_escape_json(repo->url ? repo->url : "");

        gitstat_buffer_append_fmt(out_buf, "    {\n");
        gitstat_buffer_append_fmt(out_buf, "      \"name\": \"%s\",\n", r_name);
        gitstat_buffer_append_fmt(out_buf, "      \"description\": \"%s\",\n", r_desc);
        gitstat_buffer_append_fmt(out_buf, "      \"updatedAt\": \"%s\",\n", r_updated);
        gitstat_buffer_append_fmt(out_buf, "      \"url\": \"%s\"\n", r_url);
        gitstat_buffer_append_fmt(out_buf, "    }%s\n", (i + 1 < stats->recent_repos_count) ? "," : "");

        gitstat_free(r_name);
        gitstat_free(r_desc);
        gitstat_free(r_updated);
        gitstat_free(r_url);
    }

    gitstat_buffer_append_fmt(out_buf, "  ]\n");
    gitstat_buffer_append_fmt(out_buf, "}\n");

    return GITSTAT_OK;
}
