#ifndef GITSTAT_OS_PLATFORM_H
#define GITSTAT_OS_PLATFORM_H

#include <stdbool.h>

#if defined(__APPLE__) && defined(__MACH__)
    #define GITSTAT_OS_MACOS 1
#elif defined(__FreeBSD__)
    #define GITSTAT_OS_FREEBSD 1
#elif defined(__OpenBSD__)
    #define GITSTAT_OS_OPENBSD 1
#elif defined(__NetBSD__)
    #define GITSTAT_OS_NETBSD 1
#elif defined(__linux__)
    #define GITSTAT_OS_LINUX 1
#elif defined(__GNU__)
    #define GITSTAT_OS_HURD 1
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define GITSTAT_OS_WINDOWS 1
#else
    #define GITSTAT_OS_UNKNOWN 1
#endif

typedef enum gitstat_os_type {
    GITSTAT_OS_TYPE_LINUX = 0,
    GITSTAT_OS_TYPE_MACOS,
    GITSTAT_OS_TYPE_FREEBSD,
    GITSTAT_OS_TYPE_OPENBSD,
    GITSTAT_OS_TYPE_NETBSD,
    GITSTAT_OS_TYPE_HURD,
    GITSTAT_OS_TYPE_WINDOWS,
    GITSTAT_OS_TYPE_UNKNOWN
} gitstat_os_type_t;

[[nodiscard]] gitstat_os_type_t gitstat_os_current(void);
[[nodiscard]] const char *gitstat_os_name(void);

#endif
