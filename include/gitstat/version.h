#ifndef GITSTAT_VERSION_H
#define GITSTAT_VERSION_H

#define GITSTAT_VERSION_MAJOR 1
#define GITSTAT_VERSION_MINOR 0
#define GITSTAT_VERSION_PATCH 0
#define GITSTAT_VERSION_STRING "1.0.0"

[[nodiscard]] const char *gitstat_get_version_string(void);

#endif
