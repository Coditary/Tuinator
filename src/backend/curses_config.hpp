#pragma once

#if defined(TUINATOR_BACKEND_PDCURSES)
#include <curses.h>
#elif defined(TUINATOR_BACKEND_NCURSES)
#if __has_include(<ncursesw/ncurses.h>)
#include <ncursesw/ncurses.h>
#elif __has_include(<ncurses.h>)
#include <ncurses.h>
#else
#error "ncurses development headers not found"
#endif
#else
#error "No curses backend configured. Define TUINATOR_BACKEND_NCURSES or TUINATOR_BACKEND_PDCURSES."
#endif
