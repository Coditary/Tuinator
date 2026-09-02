#pragma once

#if defined(TUINATOR_BACKEND_PDCURSES)
#include <curses.h>
#elif defined(TUINATOR_BACKEND_NCURSES)
#include <ncursesw/ncurses.h>
#else
#error "No curses backend configured. Define TUINATOR_BACKEND_NCURSES or TUINATOR_BACKEND_PDCURSES."
#endif
