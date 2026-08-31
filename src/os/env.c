#include "gitstat/os/env.h"
#include <stdlib.h>
#include <string.h>

const char *gitstat_env_get(const char *name) {
    if (name == nullptr || name[0] == '\0') {
        return nullptr;
    }
    const char *val = getenv(name);
    if (val != nullptr && val[0] != '\0') {
        return val;
    }
    return nullptr;
}

bool gitstat_env_get_string(const char *name, char *buffer, size_t buffer_size) {
    if (name == nullptr || buffer == nullptr || buffer_size == 0) {
        return false;
    }
    const char *val = gitstat_env_get(name);
    if (val == nullptr) {
        return false;
    }
    strncpy(buffer, val, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return true;
}

const char *gitstat_env_detect_username(void) {
    const char *vars[] = {
        "GITSTAT_USER",
        "GITHUB_USER",
        "GITLAB_USER",
        "USER",
        "LOGNAME",
        "USERNAME"
    };

    for (size_t i = 0; i < sizeof(vars) / sizeof(vars[0]); ++i) {
        const char *user = gitstat_env_get(vars[i]);
        if (user != nullptr && user[0] != '\0') {
            return user;
        }
    }

    return nullptr;
}
