#pragma once

#include <tuinator/core/action_registry.hpp>
#include <tuinator/version.hpp>
#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/backend/inline_backend.hpp>
#include <tuinator/core/application.hpp>
#include <tuinator/core/inline_view.hpp>
#include <tuinator/core/event.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/render/canvas.hpp>
#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/color.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/file_icon.hpp>
#include <tuinator/render/git_change_status.hpp>
#include <tuinator/render/ui_icon.hpp>
#include <tuinator/render/line_icon.hpp>
#include <tuinator/render/nerd_icon_set.hpp>
#include <tuinator/render/weather_icon.hpp>
#include <tuinator/render/moon_icon.hpp>
#include <tuinator/render/clock_icon.hpp>
#include <tuinator/render/compass_icon.hpp>
#include <tuinator/render/measurement_icon.hpp>
#include <tuinator/render/astronomy_icon.hpp>
#include <tuinator/render/air_quality_icon.hpp>
#include <tuinator/render/hazard_icon.hpp>
#include <tuinator/render/marine_alert_icon.hpp>
#include <tuinator/render/action_icon.hpp>
#include <tuinator/render/block_element.hpp>
#include <tuinator/render/box_drawing.hpp>
#include <tuinator/render/border_style.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/render/scrollbar.hpp>
#include <tuinator/render/graphics_protocol.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/render/theme.hpp>

// Controls
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/controls/text_area.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/controls/toggle.hpp>

// Display
#include <tuinator/widgets/display/big_text.hpp>
#include <tuinator/widgets/display/image_view.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/nerd_icon_catalog_gallery.hpp>
#include <tuinator/widgets/display/nerd_glyph_matrix_gallery.hpp>
#include <tuinator/render/nerd_glyph_catalog.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>
#include <tuinator/widgets/display/separator.hpp>
#include <tuinator/widgets/display/spinner.hpp>
#include <tuinator/widgets/display/throbber.hpp>

// Containers
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/containers/tabs.hpp>

// Views
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>
#include <tuinator/widgets/views/tree_view.hpp>

// Charts
#include <tuinator/widgets/charts/bar_chart.hpp>
#include <tuinator/widgets/charts/candlestick_chart.hpp>
#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/charts/gauge_chart.hpp>
#include <tuinator/widgets/charts/heatmap.hpp>
#include <tuinator/widgets/charts/histogram.hpp>
#include <tuinator/widgets/charts/line_chart.hpp>
#include <tuinator/widgets/charts/pie_chart.hpp>
#include <tuinator/widgets/charts/stacked_area_chart.hpp>
#include <tuinator/widgets/charts/waterfall_chart.hpp>

// Menu
#include <tuinator/widgets/menu/command_palette.hpp>
#include <tuinator/widgets/menu/context_menu.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>
#include <tuinator/widgets/menu/menu_common.hpp>

// Chrome
#include <tuinator/widgets/chrome/dialog.hpp>
#include <tuinator/widgets/chrome/source_control_panel.hpp>
#include <tuinator/widgets/chrome/status_bar.hpp>
#include <tuinator/widgets/chrome/status_line.hpp>
#include <tuinator/widgets/chrome/terminal_frame.hpp>
#include <tuinator/widgets/chrome/shell_terminal.hpp>

#include <tuinator/widgets/widget.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/window/desktop.hpp>
#include <tuinator/window/window.hpp>
#include <tuinator/window/window_host.hpp>
#include <tuinator/window/window_manager.hpp>

#include <tuinator/scene/scene.hpp>
