set(TUINATOR_TEST_SOURCES
    core/test_main.cpp
    core/test_harness_meta.cpp
    core/test_text.cpp
    core/test_memory_backend.cpp
    core/test_inline_backend.cpp
    backend/test_terminal_input.cpp
    backend/test_ansi_backend_images.cpp
    core/test_inline_view.cpp
    core/test_dirty_region.cpp
    core/test_screen.cpp
    core/test_debug_paint.cpp
    core/test_testing_driver.cpp
    core/test_json_protocol_extended.cpp
    scene/test_scene_views.cpp
    render/test_glyphs.cpp
    render/test_border_style.cpp
    render/test_block_element.cpp
    render/test_box_drawing.cpp
    render/test_stylesheet.cpp
    render/test_stylesheet_pseudo_tokens.cpp
    render/test_hover_and_warnings.cpp
    render/test_tty_overlay.cpp
    render/test_widget_options.cpp
    render/test_style_attributes.cpp
    render/test_markdown.cpp
    render/test_asciidoc.cpp
    render/test_animated_image.cpp
    widgets/test_stylesheet_widgets.cpp
    widgets/test_widget_roles.cpp
    render/test_file_icon.cpp
    render/test_git_change_status.cpp
    render/test_ui_icon.cpp
    render/test_line_icon.cpp
    render/test_weather_icon.cpp
    render/test_nerd_icon_catalog.cpp
    render/test_nerd_glyph_matrix.cpp
    terminal/test_ansi_terminal.cpp
    widgets/test_label.cpp
    widgets/test_markdown_view.cpp
    widgets/test_asciidoc_view.cpp
    widgets/test_video_view.cpp
    widgets/test_list_view.cpp
    widgets/test_list_view_app_driver.cpp
    widgets/test_button.cpp
    widgets/test_toggle.cpp
    widgets/test_progress_bar.cpp
    widgets/test_spinner.cpp
    widgets/test_text_input.cpp
    widgets/test_table.cpp
    widgets/test_tree_view.cpp
    widgets/test_tabs.cpp
    widgets/test_combo_box.cpp
    widgets/test_slider.cpp
    widgets/test_separator.cpp
    widgets/test_status_bar.cpp
    widgets/test_layout_box.cpp
    widgets/test_dialog.cpp
    widgets/test_window.cpp
    window/test_desktop.cpp
    widgets/test_panel.cpp
    widgets/test_scroll_view.cpp
    widgets/test_widgets_app_driver.cpp
    widgets/test_action_registry.cpp
    widgets/test_command_palette.cpp
    widgets/test_context_menu.cpp
    widgets/test_checkbox.cpp
    widgets/test_component_app_driver.cpp
    widgets/test_panes_app_driver.cpp
    integration/test_pane_nesting.cpp
    integration/test_widget_nesting.cpp
    widgets/edge/test_edge_views.cpp
    widgets/edge/test_edge_controls.cpp
    widgets/edge/test_edge_containers.cpp
    widgets/edge/test_edge_display_menu.cpp
    widgets/edge/test_edge_charts.cpp
    widgets/edge/test_edge_misc.cpp
    widgets/edge/test_edge_param.cpp
    widgets/edge/test_edge_negative.cpp
    widgets/test_menu_bar.cpp
    widgets/test_text_area.cpp
    widgets/test_scrollbar.cpp
    widgets/test_split_pane.cpp
    widgets/test_custom_paint.cpp
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
    widgets/chrome/test_shell_terminal.cpp
)

add_executable(tuinator-tests ${TUINATOR_TEST_SOURCES})
set_target_properties(tuinator-tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(tuinator-tests PRIVATE tuinator::tuinator)
target_include_directories(tuinator-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${TUINATOR_SOURCE_DIR}/src
)
target_compile_definitions(tuinator-tests PRIVATE TUINATOR_DATA_DIR="${TUINATOR_SOURCE_DIR}/data")
add_test(NAME tuinator-tests COMMAND tuinator-tests)
set_tests_properties(tuinator-tests PROPERTIES WORKING_DIRECTORY ${TUINATOR_SOURCE_DIR})

set(TUINATOR_PERF_SOURCES
    perf/perf_main.cpp
    perf/perf_text.cpp
    perf/perf_render.cpp
    perf/perf_ansi_terminal.cpp
    perf/perf_charts.cpp
    perf/perf_nerd_glyph.cpp
    perf/perf_graphics.cpp
    perf/perf_widgets.cpp
    perf/perf_app_driver.cpp
)

add_executable(tuinator-perf-tests ${TUINATOR_PERF_SOURCES})
set_target_properties(tuinator-perf-tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(tuinator-perf-tests PRIVATE tuinator::tuinator)
target_include_directories(tuinator-perf-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/support
)
target_compile_definitions(tuinator-perf-tests PRIVATE TUINATOR_DATA_DIR="${TUINATOR_SOURCE_DIR}/data")
add_test(NAME tuinator-perf-tests COMMAND tuinator-perf-tests)
set_tests_properties(tuinator-perf-tests PROPERTIES
    WORKING_DIRECTORY ${TUINATOR_SOURCE_DIR}
    LABELS "perf"
)
