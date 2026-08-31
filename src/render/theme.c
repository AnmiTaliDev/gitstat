#include "gitstat/render/theme.h"
#include "gitstat/render/color.h"

gitstat_theme_t gitstat_theme_get(bool use_color) {
    if (!use_color) {
        return (gitstat_theme_t){
            .reset = "",
            .bold = "",
            .dim = "",
            .error = "",
            .title = "",
            .subtitle = "",
            .header = "",
            .stat_contributions = "",
            .stat_commits = "",
            .stat_prs = "",
            .stat_restricted = "",
            .stat_repos = "",
            .repo_name = "",
            .repo_url = ""
        };
    }

    return (gitstat_theme_t){
        .reset = GITSTAT_ANSI_RESET,
        .bold = GITSTAT_ANSI_BOLD,
        .dim = GITSTAT_ANSI_DIM,
        .error = GITSTAT_ANSI_YELLOW,
        .title = GITSTAT_ANSI_BOLD GITSTAT_ANSI_CYAN,
        .subtitle = GITSTAT_ANSI_DIM,
        .header = GITSTAT_ANSI_BOLD GITSTAT_ANSI_YELLOW,
        .stat_contributions = GITSTAT_ANSI_GREEN,
        .stat_commits = GITSTAT_ANSI_BLUE,
        .stat_prs = GITSTAT_ANSI_MAGENTA,
        .stat_restricted = GITSTAT_ANSI_DIM,
        .stat_repos = GITSTAT_ANSI_CYAN,
        .repo_name = GITSTAT_ANSI_BOLD GITSTAT_ANSI_GREEN,
        .repo_url = GITSTAT_ANSI_BLUE
    };
}
