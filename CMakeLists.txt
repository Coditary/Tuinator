cmake_minimum_required(VERSION 3.16)
project(Tuinator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# --- Terminal backend ---------------------------------------------------

set(TUINATOR_BACKEND_SOURCES
    src/backend/curses_backend.cpp
    src/backend/memory_backend.cpp
    src/backend/terminal_backend.cpp
)

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
        add_library(PkgConfig::NCURSES UNKNOWN IMPORTED)
        set_target_properties(PkgConfig::NCURSES PROPERTIES
            IMPORTED_LOCATION "${CURSES_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
        )
    endif()

    set(TUINATOR_CURSES_TARGET PkgConfig::NCURSES)
    set(TUINATOR_CURSES_INCLUDE "")
    set(TUINATOR_CURSES_LIBS "")
endif()

find_package(ZLIB REQUIRED)

# --- Library ------------------------------------------------------------

add_library(tuinator
    ${TUINATOR_BACKEND_SOURCES}
    src/core/action_registry.cpp
    src/core/application.cpp
    src/debug/startup_profiler.cpp
    src/layout/box.cpp
    src/layout/grid.cpp
    src/render/canvas.cpp
    src/render/color.cpp
    src/render/graphics_encode.cpp
    src/render/graphics_protocol.cpp
    src/render/glyphs.cpp
    src/render/scrollbar/core.cpp
    src/render/scrollbar/behavior.cpp
    src/render/scrollbar/style.cpp
    src/render/scrollbar/scrollbar.cpp
    src/render/terminal_image.cpp
    src/render/text.cpp
    src/render/theme.cpp
    src/widgets/widget.cpp
    src/widgets/controls/button.cpp
    src/widgets/controls/checkbox.cpp
    src/widgets/controls/combo_box.cpp
    src/widgets/controls/slider.cpp
    src/widgets/controls/text_area.cpp
    src/widgets/controls/text_input.cpp
    src/widgets/controls/toggle.cpp
    src/widgets/display/big_text.cpp
    src/widgets/display/image_view.cpp
    src/widgets/display/label.cpp
    src/widgets/display/progress_bar.cpp
    src/widgets/display/separator.cpp
    src/widgets/display/spinner.cpp
    src/widgets/display/throbber.cpp
    src/widgets/containers/panel.cpp
    src/widgets/containers/scroll_view.cpp
    src/widgets/containers/split_pane.cpp
    src/widgets/containers/tabs.cpp
    src/widgets/views/list_view.cpp
    src/widgets/views/table.cpp
    src/widgets/views/tree_view.cpp
    src/widgets/charts/bar_chart.cpp
    src/widgets/charts/candlestick_chart.cpp
    src/widgets/charts/chart_common.cpp
    src/widgets/charts/gauge_chart.cpp
    src/widgets/charts/heatmap.cpp
    src/widgets/charts/histogram.cpp
    src/widgets/charts/line_chart.cpp
    src/widgets/charts/pie_chart.cpp
    src/widgets/charts/stacked_area_chart.cpp
    src/widgets/charts/waterfall_chart.cpp
    src/widgets/menu/command_palette.cpp
    src/widgets/menu/context_menu.cpp
    src/widgets/menu/menu_bar.cpp
    src/widgets/menu/menu_common.cpp
    src/widgets/chrome/dialog.cpp
    src/widgets/chrome/status_bar.cpp
    src/window/desktop.cpp
    src/window/window.cpp
    src/window/window_host.cpp
)

add_library(tuinator::tuinator ALIAS tuinator)

target_include_directories(tuinator
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${TUINATOR_CURSES_INCLUDE}
)

if(TUINATOR_CURSES_TARGET)
    target_link_libraries(tuinator PUBLIC ${TUINATOR_CURSES_TARGET})
else()
    target_link_libraries(tuinator PUBLIC ${TUINATOR_CURSES_LIBS})
endif()
target_link_libraries(tuinator PRIVATE ZLIB::ZLIB)

target_compile_definitions(tuinator
    PUBLIC  $<$<BOOL:${WIN32}>:TUINATOR_PLATFORM_WINDOWS=1>
    PRIVATE ${TUINATOR_BACKEND_DEF}
)

target_compile_options(tuinator PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# --- Unit tests ---------------------------------------------------------

option(TUINATOR_BUILD_TESTS "Build unit tests" ON)

if(TUINATOR_BUILD_TESTS)
    add_executable(tuinator-tests
        tests/test_main.cpp
        tests/test_text.cpp
        tests/test_label.cpp
        tests/test_list_view.cpp
        tests/test_memory_backend.cpp
        tests/test_glyphs.cpp
        tests/test_image.cpp
        tests/test_controls.cpp
        tests/test_checkbox.cpp
        tests/test_menu_bar.cpp
        tests/test_text_area.cpp
        tests/test_scrollbar.cpp
        tests/test_throbber.cpp
        tests/test_big_text.cpp
        tests/test_pie_chart.cpp
        tests/test_bar_chart.cpp
        tests/test_line_chart.cpp
        tests/test_advanced_charts.cpp
    )
    target_link_libraries(tuinator-tests PRIVATE tuinator::tuinator)
    target_include_directories(tuinator-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_test(NAME tuinator-tests COMMAND tuinator-tests)
endif()

# --- Examples -----------------------------------------------------------

option(TUINATOR_BUILD_EXAMPLES "Build example programs" ON)

if(TUINATOR_BUILD_EXAMPLES)
    set(TUINATOR_DEMOS hello form colors layout counter buttons windows mouse-test scroll theme startup-profile dashboard data controls menu image textarea throbber bigtext checkbox piechart charts)
    foreach(demo IN LISTS TUINATOR_DEMOS)
        add_executable(tuinator-${demo} examples/${demo}.cpp)
        target_link_libraries(tuinator-${demo} PRIVATE tuinator::tuinator)
    endforeach()
    target_compile_definitions(tuinator-image PRIVATE
        TUINATOR_LENNA_PATH="${CMAKE_SOURCE_DIR}/examples/assets/lenna.png")
endif()
