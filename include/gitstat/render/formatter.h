#ifndef GITSTAT_RENDER_FORMATTER_H
#define GITSTAT_RENDER_FORMATTER_H

#include "gitstat/config/config.h"
#include "gitstat/model/user_stats.h"
#include "gitstat/model/repository.h"
#include "gitstat/render/theme.h"
#include "gitstat/util/buffer.h"

[[nodiscard]] gitstat_error_t gitstat_format_stats(const gitstat_user_stats_t *stats, const gitstat_config_t *cfg, const gitstat_theme_t *theme, gitstat_buffer_t *out_buf);
[[nodiscard]] gitstat_error_t gitstat_format_stats_json(const gitstat_user_stats_t *stats, gitstat_buffer_t *out_buf);
[[nodiscard]] gitstat_error_t gitstat_format_repo_stats(const gitstat_repository_t *repo, const gitstat_theme_t *theme, gitstat_buffer_t *out_buf);
[[nodiscard]] gitstat_error_t gitstat_format_repo_stats_json(const gitstat_repository_t *repo, gitstat_buffer_t *out_buf);

#endif
