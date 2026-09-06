if(WIN32)
    set(TUINATOR_BACKEND_DEF TUINATOR_BACKEND_PDCURSES)

    set(TUINATOR_PDCURSES_ROOT "" CACHE PATH "Root directory of a PDCurses / PDCursesMod install")

    if(TUINATOR_PDCURSES_ROOT)
        set(PDCURSES_INCLUDE_DIR "${TUINATOR_PDCURSES_ROOT}/include")
        find_library(PDCURSES_LIBRARY
            NAMES pdcurses pdcursesmod pdcurses.lib
            PATHS "${TUINATOR_PDCURSES_ROOT}/lib" "${TUINATOR_PDCURSES_ROOT}"
            NO_DEFAULT_PATH
        )
    else()
        find_path(PDCURSES_INCLUDE_DIR NAMES curses.h)
        find_library(PDCURSES_LIBRARY NAMES pdcurses pdcursesmod)
    endif()

    if(NOT PDCURSES_INCLUDE_DIR OR NOT PDCURSES_LIBRARY)
        message(FATAL_ERROR
            "PDCurses not found. Install it (e.g. vcpkg install pdcurses) "
            "or pass -DTUINATOR_PDCURSES_ROOT=/path/to/pdcurses")
    endif()

    set(TUINATOR_CURSES_TARGET "")
    set(TUINATOR_CURSES_INCLUDE "${PDCURSES_INCLUDE_DIR}")
    set(TUINATOR_CURSES_LIBS "${PDCURSES_LIBRARY}")
else()
    set(TUINATOR_BACKEND_DEF TUINATOR_BACKEND_NCURSES)

    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(NCURSES QUIET IMPORTED_TARGET ncursesw)
    endif()

    if(NOT NCURSES_FOUND)
        set(CURSES_NEED_NCURSES TRUE)
        set(CURSES_NEED_WIDE TRUE)
        find_package(Curses REQUIRED)
        add_library(Tuinator::NCURSES UNKNOWN IMPORTED)
        set_target_properties(Tuinator::NCURSES PROPERTIES
            IMPORTED_LOCATION "${CURSES_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
        )
        set(TUINATOR_CURSES_TARGET Tuinator::NCURSES)
    else()
        set(TUINATOR_CURSES_TARGET PkgConfig::NCURSES)
    endif()

    set(TUINATOR_CURSES_INCLUDE "")
    set(TUINATOR_CURSES_LIBS "")
endif()

find_package(ZLIB REQUIRED)

if(NOT TARGET nlohmann_json::nlohmann_json)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
    set(TUINATOR_JSON_INCLUDE_DIR ${nlohmann_json_SOURCE_DIR}/include)
else()
    get_target_property(TUINATOR_JSON_INCLUDE_DIR nlohmann_json::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/libvterm.cmake)
