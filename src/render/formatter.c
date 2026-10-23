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

    if (stats->organizations_count > 0) {
        gitstat_buffer_append_fmt(out_buf, "  %s Organizations:%s       ",
            theme->stat_repos,
            theme->reset
        );
        for (size_t i = 0; i < stats->organizations_count; ++i) {
            gitstat_buffer_append_fmt(out_buf, "@%s%s",
                stats->organizations[i],
                (i + 1 < stats->organizations_count) ? ", " : "\n"
            );
        }
    }

    if (stats->contributed_repos_count > 0) {
        gitstat_buffer_append_fmt(out_buf, "\n%s-- Recently Contributed Repos --%s\n",
            theme->header,
            theme->reset
        );

        for (size_t i = 0; i < stats->contributed_repos_count; ++i) {
            const gitstat_repository_t *repo = &stats->contributed_repos[i];
            const char *display_name = (repo->full_name && repo->full_name[0] != '\0') ? repo->full_name : repo->name;
            gitstat_buffer_append_fmt(out_buf, "  %s %s%s %s[%s]%s\n",
                theme->repo_name,
                display_name ? display_name : "",
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
            if (i + 1 < stats->contributed_repos_count) {
                gitstat_buffer_append_char(out_buf, '\n');
            }
        }
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
    gitstat_buffer_append_fmt(out_buf, "  \"organizations\": [\n");
    for (size_t i = 0; i < stats->organizations_count; ++i) {
        char *esc_org = gitstat_str_escape_json(stats->organizations[i] ? stats->organizations[i] : "");
        gitstat_buffer_append_fmt(out_buf, "    \"%s\"%s\n", esc_org, (i + 1 < stats->organizations_count) ? "," : "");
        gitstat_free(esc_org);
    }
    gitstat_buffer_append_fmt(out_buf, "  ],\n");
    gitstat_buffer_append_fmt(out_buf, "  \"contributedRepositories\": [\n");
    for (size_t i = 0; i < stats->contributed_repos_count; ++i) {
        const gitstat_repository_t *repo = &stats->contributed_repos[i];
        char *r_name = gitstat_str_escape_json(repo->name ? repo->name : "");
        char *r_full = gitstat_str_escape_json(repo->full_name ? repo->full_name : "");
        char *r_desc = gitstat_str_escape_json(repo->description ? repo->description : "");
        char *r_updated = gitstat_str_escape_json(repo->updated_at ? repo->updated_at : "");
        char *r_url = gitstat_str_escape_json(repo->url ? repo->url : "");

        gitstat_buffer_append_fmt(out_buf, "    {\n");
        gitstat_buffer_append_fmt(out_buf, "      \"name\": \"%s\",\n", r_name);
        gitstat_buffer_append_fmt(out_buf, "      \"fullName\": \"%s\",\n", r_full);
        gitstat_buffer_append_fmt(out_buf, "      \"description\": \"%s\",\n", r_desc);
        gitstat_buffer_append_fmt(out_buf, "      \"updatedAt\": \"%s\",\n", r_updated);
        gitstat_buffer_append_fmt(out_buf, "      \"url\": \"%s\"\n", r_url);
        gitstat_buffer_append_fmt(out_buf, "    }%s\n", (i + 1 < stats->contributed_repos_count) ? "," : "");

        gitstat_free(r_name);
        gitstat_free(r_full);
        gitstat_free(r_desc);
        gitstat_free(r_updated);
        gitstat_free(r_url);
    }
    gitstat_buffer_append_fmt(out_buf, "  ],\n");
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

gitstat_error_t gitstat_format_repo_stats(const gitstat_repository_t *repo, const gitstat_theme_t *theme, gitstat_buffer_t *out_buf) {
    if (repo == nullptr || theme == nullptr || out_buf == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    gitstat_buffer_append_fmt(out_buf, " %s%s%s\n",
        theme->title,
        (repo->full_name && repo->full_name[0] != '\0') ? repo->full_name : (repo->name ? repo->name : ""),
        theme->reset
    );

    if (repo->description && repo->description[0] != '\0') {
        gitstat_buffer_append_fmt(out_buf, "  %s%s%s\n",
            theme->dim,
            repo->description,
            theme->reset
        );
    }

    if (repo->url && repo->url[0] != '\0') {
        gitstat_buffer_append_fmt(out_buf, "  %s%s%s\n",
            theme->repo_url,
            repo->url,
            theme->reset
        );
    }

    gitstat_buffer_append_fmt(out_buf, "%s-- Repository Stats --%s\n",
        theme->header,
        theme->reset
    );

    gitstat_buffer_append_fmt(out_buf, "  %s Stars:%s        %lld\n",
        theme->stat_contributions,
        theme->reset,
        (long long)repo->stars_count
    );

    gitstat_buffer_append_fmt(out_buf, "  %s Forks:%s        %lld\n",
        theme->stat_prs,
        theme->reset,
        (long long)repo->forks_count
    );

    gitstat_buffer_append_fmt(out_buf, "  %s Open Issues:%s  %lld\n",
        theme->stat_commits,
        theme->reset,
        (long long)repo->open_issues_count
    );

    if (repo->watchers_count > 0) {
        gitstat_buffer_append_fmt(out_buf, "  %s Watchers:%s     %lld\n",
            theme->stat_repos,
            theme->reset,
            (long long)repo->watchers_count
        );
    }

    if (repo->language && repo->language[0] != '\0') {
        gitstat_buffer_append_fmt(out_buf, "  %s Language:%s     %s\n",
            theme->stat_repos,
            theme->reset,
            repo->language
        );
    }

    if (repo->license && repo->license[0] != '\0') {
        gitstat_buffer_append_fmt(out_buf, "  %s License:%s      %s\n",
            theme->stat_restricted,
            theme->reset,
            repo->license
        );
    }

    if (repo->default_branch && repo->default_branch[0] != '\0') {
        gitstat_buffer_append_fmt(out_buf, "  %s Branch:%s       %s\n",
            theme->subtitle,
            theme->reset,
            repo->default_branch
        );
    }

    if (repo->updated_at && repo->updated_at[0] != '\0') {
        gitstat_buffer_append_fmt(out_buf, "  %s Updated:%s      %s\n",
            theme->dim,
            theme->reset,
            repo->updated_at
        );
    }

    return GITSTAT_OK;
}

gitstat_error_t gitstat_format_repo_stats_json(const gitstat_repository_t *repo, gitstat_buffer_t *out_buf) {
    if (repo == nullptr || out_buf == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    char *esc_name = gitstat_str_escape_json(repo->name ? repo->name : "");
    char *esc_full_name = gitstat_str_escape_json(repo->full_name ? repo->full_name : "");
    char *esc_desc = gitstat_str_escape_json(repo->description ? repo->description : "");
    char *esc_url = gitstat_str_escape_json(repo->url ? repo->url : "");
    char *esc_lang = gitstat_str_escape_json(repo->language ? repo->language : "");
    char *esc_lic = gitstat_str_escape_json(repo->license ? repo->license : "");
    char *esc_branch = gitstat_str_escape_json(repo->default_branch ? repo->default_branch : "");
    char *esc_updated = gitstat_str_escape_json(repo->updated_at ? repo->updated_at : "");

    gitstat_buffer_append_fmt(out_buf, "{\n");
    gitstat_buffer_append_fmt(out_buf, "  \"name\": \"%s\",\n", esc_name);
    gitstat_buffer_append_fmt(out_buf, "  \"fullName\": \"%s\",\n", esc_full_name);
    gitstat_buffer_append_fmt(out_buf, "  \"description\": \"%s\",\n", esc_desc);
    gitstat_buffer_append_fmt(out_buf, "  \"url\": \"%s\",\n", esc_url);
    gitstat_buffer_append_fmt(out_buf, "  \"stars\": %lld,\n", (long long)repo->stars_count);
    gitstat_buffer_append_fmt(out_buf, "  \"forks\": %lld,\n", (long long)repo->forks_count);
    gitstat_buffer_append_fmt(out_buf, "  \"openIssues\": %lld,\n", (long long)repo->open_issues_count);
    gitstat_buffer_append_fmt(out_buf, "  \"watchers\": %lld,\n", (long long)repo->watchers_count);
    gitstat_buffer_append_fmt(out_buf, "  \"language\": \"%s\",\n", esc_lang);
    gitstat_buffer_append_fmt(out_buf, "  \"license\": \"%s\",\n", esc_lic);
    gitstat_buffer_append_fmt(out_buf, "  \"defaultBranch\": \"%s\",\n", esc_branch);
    gitstat_buffer_append_fmt(out_buf, "  \"updatedAt\": \"%s\"\n", esc_updated);
    gitstat_buffer_append_fmt(out_buf, "}\n");

    gitstat_free(esc_name);
    gitstat_free(esc_full_name);
    gitstat_free(esc_desc);
    gitstat_free(esc_url);
    gitstat_free(esc_lang);
    gitstat_free(esc_lic);
    gitstat_free(esc_branch);
    gitstat_free(esc_updated);

    return GITSTAT_OK;
}
