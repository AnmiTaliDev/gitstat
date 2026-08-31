#ifndef GITSTAT_RENDER_THEME_H
#define GITSTAT_RENDER_THEME_H

#include <stdbool.h>

typedef struct gitstat_theme {
    const char *reset;
    const char *bold;
    const char *dim;
    const char *error;
    const char *title;
    const char *subtitle;
    const char *header;
    const char *stat_contributions;
    const char *stat_commits;
    const char *stat_prs;
    const char *stat_restricted;
    const char *stat_repos;
    const char *repo_name;
    const char *repo_url;
} gitstat_theme_t;

[[nodiscard]] gitstat_theme_t gitstat_theme_get(bool use_color);

#endif
