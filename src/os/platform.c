#include "gitstat/os/platform.h"

gitstat_os_type_t gitstat_os_current(void) {
#if defined(GITSTAT_OS_MACOS)
    return GITSTAT_OS_TYPE_MACOS;
#elif defined(GITSTAT_OS_FREEBSD)
    return GITSTAT_OS_TYPE_FREEBSD;
#elif defined(GITSTAT_OS_OPENBSD)
    return GITSTAT_OS_TYPE_OPENBSD;
#elif defined(GITSTAT_OS_NETBSD)
    return GITSTAT_OS_TYPE_NETBSD;
#elif defined(GITSTAT_OS_LINUX)
    return GITSTAT_OS_TYPE_LINUX;
#elif defined(GITSTAT_OS_HURD)
    return GITSTAT_OS_TYPE_HURD;
#elif defined(GITSTAT_OS_WINDOWS)
    return GITSTAT_OS_TYPE_WINDOWS;
#else
    return GITSTAT_OS_TYPE_UNKNOWN;
#endif
}

const char *gitstat_os_name(void) {
#if defined(GITSTAT_OS_MACOS)
    return "macOS";
#elif defined(GITSTAT_OS_FREEBSD)
    return "FreeBSD";
#elif defined(GITSTAT_OS_OPENBSD)
    return "OpenBSD";
#elif defined(GITSTAT_OS_NETBSD)
    return "NetBSD";
#elif defined(GITSTAT_OS_LINUX)
    return "Linux";
#elif defined(GITSTAT_OS_HURD)
    return "GNU/Hurd";
#elif defined(GITSTAT_OS_WINDOWS)
    return "Windows";
#else
    return "Unknown OS";
#endif
}
