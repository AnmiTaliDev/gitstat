#include "gitstat/os/paths.h"
#include "gitstat/os/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(GITSTAT_OS_WINDOWS)
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

bool gitstat_paths_file_exists(const char *path) {
    if (path == nullptr || path[0] == '\0') {
        return false;
    }
    return access(path, F_OK) == 0;
}

gitstat_error_t gitstat_paths_get_config_path(char *buffer, size_t buffer_size) {
    if (buffer == nullptr || buffer_size == 0) {
        return GITSTAT_ERR_INVALID_ARG;
    }

#if defined(GITSTAT_OS_WINDOWS)
    const char *appdata = getenv("APPDATA");
    if (appdata != nullptr && appdata[0] != '\0') {
        int written = snprintf(buffer, buffer_size, "%s\\gitstat\\config.ini", appdata);
        if (written > 0 && (size_t)written < buffer_size && gitstat_paths_file_exists(buffer)) {
            return GITSTAT_OK;
        }
    }

    const char *userprofile = getenv("USERPROFILE");
    if (userprofile != nullptr && userprofile[0] != '\0') {
        int written = snprintf(buffer, buffer_size, "%s\\.gitstatrc", userprofile);
        if (written > 0 && (size_t)written < buffer_size) {
            return GITSTAT_OK;
        }
    }
#else
    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg != nullptr && xdg[0] != '\0') {
        int written = snprintf(buffer, buffer_size, "%s/gitstat/config.ini", xdg);
        if (written > 0 && (size_t)written < buffer_size && gitstat_paths_file_exists(buffer)) {
            return GITSTAT_OK;
        }
    }

    const char *home = getenv("HOME");
    if (home != nullptr && home[0] != '\0') {
        int written = snprintf(buffer, buffer_size, "%s/.config/gitstat/config.ini", home);
        if (written > 0 && (size_t)written < buffer_size && gitstat_paths_file_exists(buffer)) {
            return GITSTAT_OK;
        }

        written = snprintf(buffer, buffer_size, "%s/.gitstatrc", home);
        if (written > 0 && (size_t)written < buffer_size && gitstat_paths_file_exists(buffer)) {
            return GITSTAT_OK;
        }

        written = snprintf(buffer, buffer_size, "%s/.config/gitstat/config.ini", home);
        if (written > 0 && (size_t)written < buffer_size) {
            return GITSTAT_OK;
        }
    }
#endif

    buffer[0] = '\0';
    return GITSTAT_ERR_NOT_FOUND;
}
