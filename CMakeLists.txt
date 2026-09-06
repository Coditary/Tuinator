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
    src/backend/inline_backend.cpp
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

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# --- Library ------------------------------------------------------------

add_subdirectory(third_party/libvterm EXCLUDE_FROM_ALL)

add_library(tuinator
    ${TUINATOR_BACKEND_SOURCES}
    src/core/action_registry.cpp
    src/core/application.cpp
    src/core/inline_view.cpp
    src/debug/startup_profiler.cpp
    src/layout/box.cpp
    src/layout/grid.cpp
    src/render/canvas.cpp
    src/render/color.cpp
    src/render/graphics_encode.cpp
    src/render/graphics_protocol.cpp
    src/render/file_icon.cpp
    src/render/git_change_status.cpp
    src/render/ui_icon.cpp
    src/render/line_icon.cpp
    src/render/nerd_icon_set.cpp
    src/render/weather_icon.cpp
    src/render/moon_icon.cpp
    src/render/clock_icon.cpp
    src/render/compass_icon.cpp
    src/render/measurement_icon.cpp
    src/render/astronomy_icon.cpp
    src/render/air_quality_icon.cpp
    src/render/hazard_icon.cpp
    src/render/marine_alert_icon.cpp
    src/render/action_icon.cpp
    src/render/nerd_glyph_catalog.cpp
    src/render/block_element.cpp
    src/render/box_drawing.cpp
    src/render/glyphs.cpp
    src/render/scrollbar/core.cpp
    src/render/scrollbar/behavior.cpp
    src/render/scrollbar/style.cpp
    src/render/scrollbar/scrollbar.cpp
    src/render/terminal_image.cpp
    src/render/text.cpp
    src/render/theme.cpp
    src/widgets/widget.cpp
    src/widgets/capabilities.cpp
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
    src/widgets/display/nerd_icon_catalog_gallery.cpp
    src/widgets/display/nerd_glyph_matrix_gallery.cpp
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
    src/widgets/chrome/source_control_panel.cpp
    src/widgets/chrome/status_bar.cpp
    src/widgets/chrome/status_line.cpp
    src/widgets/chrome/terminal_frame.cpp
    src/widgets/chrome/shell_terminal.cpp
    src/terminal/ansi_terminal_buffer.cpp
    src/terminal/pty_session.cpp
    src/window/desktop.cpp
    src/window/window.cpp
    src/window/window_manager.cpp
    src/window/window_host.cpp
    src/scene/handler_registry.cpp
    src/scene/scene_context.cpp
    src/scene/scene_builder.cpp
    src/scene/detail/json_node.cpp
    src/scene/detail/style_resolve.cpp
    src/scene/detail/binding_engine.cpp
    src/scene/runtime/widget_builder.generated.cpp
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
target_link_libraries(tuinator PRIVATE ZLIB::ZLIB tuinator_libvterm nlohmann_json::nlohmann_json)

if(NOT WIN32)
    find_library(TUINATOR_UTIL_LIBRARY util)
    if(TUINATOR_UTIL_LIBRARY)
        target_link_libraries(tuinator PRIVATE ${TUINATOR_UTIL_LIBRARY})
    endif()
endif()

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
    set(TUINATOR_TEST_SOURCES
        # Core
        tests/core/test_main.cpp
        tests/core/test_text.cpp
        tests/core/test_memory_backend.cpp
        tests/core/test_inline_backend.cpp
        tests/core/test_dirty_region.cpp
        # Render
        tests/render/test_glyphs.cpp
        tests/render/test_border_style.cpp
        tests/render/test_block_element.cpp
        tests/render/test_box_drawing.cpp
        tests/render/test_file_icon.cpp
        tests/render/test_git_change_status.cpp
        tests/render/test_ui_icon.cpp
        tests/render/test_line_icon.cpp
        tests/render/test_weather_icon.cpp
        tests/render/test_nerd_icon_catalog.cpp
        tests/render/test_nerd_glyph_matrix.cpp
        tests/terminal/test_ansi_terminal.cpp
        # Widgets
        tests/widgets/test_label.cpp
        tests/widgets/test_list_view.cpp
        tests/widgets/test_controls.cpp
        tests/widgets/test_checkbox.cpp
        tests/widgets/test_menu_bar.cpp
        tests/widgets/test_text_area.cpp
        tests/widgets/test_scrollbar.cpp
        tests/widgets/test_throbber.cpp
        tests/widgets/test_big_text.cpp
        tests/widgets/test_image.cpp
        tests/widgets/charts/test_pie_chart.cpp
        tests/widgets/charts/test_bar_chart.cpp
        tests/widgets/charts/test_line_chart.cpp
        tests/widgets/charts/test_advanced_charts.cpp
        tests/widgets/chrome/test_source_control_panel.cpp
        tests/widgets/chrome/test_status_line.cpp
        tests/widgets/chrome/test_terminal_frame.cpp
    )

    add_executable(tuinator-tests ${TUINATOR_TEST_SOURCES})
    target_link_libraries(tuinator-tests PRIVATE tuinator::tuinator)
    target_include_directories(tuinator-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/support
    )
    target_compile_definitions(tuinator-tests PRIVATE TUINATOR_DATA_DIR="${CMAKE_SOURCE_DIR}/data")
    add_test(NAME tuinator-tests COMMAND tuinator-tests)
endif()

# --- Examples -----------------------------------------------------------

option(TUINATOR_BUILD_EXAMPLES "Build example programs" ON)

if(TUINATOR_BUILD_EXAMPLES)
    set(TUINATOR_DEMOS hello form colors layout counter buttons windows mouse-test scroll theme startup-profile dashboard data controls menu image textarea throbber bigtext checkbox piechart charts diffview weather glyphs terminal-frame scene scene-runtime inline-tasks)
    foreach(demo IN LISTS TUINATOR_DEMOS)
        if(demo STREQUAL "scene")
            add_executable(tuinator-scene
                examples/scene.cpp
                examples/scenes/form_scene.generated.cpp
            )
            target_include_directories(tuinator-scene PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples)
        elseif(demo STREQUAL "scene-runtime")
            add_executable(tuinator-scene-runtime examples/scene_runtime.cpp)
            target_include_directories(tuinator-scene-runtime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples)
        else()
            add_executable(tuinator-${demo} examples/${demo}.cpp)
        endif()
        target_link_libraries(tuinator-${demo} PRIVATE tuinator::tuinator)
    endforeach()
    target_compile_definitions(tuinator-image PRIVATE
        TUINATOR_LENNA_PATH="${CMAKE_SOURCE_DIR}/examples/assets/lenna.png")
endif()
