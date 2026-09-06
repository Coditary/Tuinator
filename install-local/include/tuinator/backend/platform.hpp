#pragma once

#if defined(_WIN32)
#define TUINATOR_PLATFORM_WINDOWS 1
#define TUINATOR_PLATFORM_POSIX 0
#else
#define TUINATOR_PLATFORM_WINDOWS 0
#define TUINATOR_PLATFORM_POSIX 1
#endif

#if TUINATOR_PLATFORM_WINDOWS
#define TUINATOR_BACKEND_ID pdcurses
#define TUINATOR_BACKEND_NAME "pdcurses"
#else
#define TUINATOR_BACKEND_ID ncurses
#define TUINATOR_BACKEND_NAME "ncursesw"
#endif
