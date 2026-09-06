set(TUINATOR_TEST_SOURCES
    core/test_main.cpp
    core/test_text.cpp
    core/test_memory_backend.cpp
    core/test_inline_backend.cpp
    core/test_inline_view.cpp
    core/test_dirty_region.cpp
    render/test_glyphs.cpp
    render/test_border_style.cpp
    render/test_block_element.cpp
    render/test_box_drawing.cpp
    render/test_file_icon.cpp
    render/test_git_change_status.cpp
    render/test_ui_icon.cpp
    render/test_line_icon.cpp
    render/test_weather_icon.cpp
    render/test_nerd_icon_catalog.cpp
    render/test_nerd_glyph_matrix.cpp
    terminal/test_ansi_terminal.cpp
    widgets/test_label.cpp
    widgets/test_list_view.cpp
    widgets/test_controls.cpp
    widgets/test_checkbox.cpp
    widgets/test_menu_bar.cpp
    widgets/test_text_area.cpp
    widgets/test_scrollbar.cpp
    widgets/test_throbber.cpp
    widgets/test_big_text.cpp
    widgets/test_image.cpp
    widgets/charts/test_pie_chart.cpp
    widgets/charts/test_bar_chart.cpp
    widgets/charts/test_line_chart.cpp
    widgets/charts/test_advanced_charts.cpp
    widgets/chrome/test_source_control_panel.cpp
    widgets/chrome/test_status_line.cpp
    widgets/chrome/test_terminal_frame.cpp
)

add_executable(tuinator-tests ${TUINATOR_TEST_SOURCES})
set_target_properties(tuinator-tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(tuinator-tests PRIVATE tuinator::tuinator)
target_include_directories(tuinator-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/support
)
target_compile_definitions(tuinator-tests PRIVATE TUINATOR_DATA_DIR="${TUINATOR_SOURCE_DIR}/data")
add_test(NAME tuinator-tests COMMAND tuinator-tests)
set_tests_properties(tuinator-tests PROPERTIES WORKING_DIRECTORY ${TUINATOR_SOURCE_DIR})
