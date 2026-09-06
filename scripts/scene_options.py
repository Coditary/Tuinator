"""Shared option-field emission for scripts/generate_scene_cpp.py."""

from __future__ import annotations

import json
from typing import Any, Callable

# Each field: (json_keys, cpp_member, kind)
# kind: int, float, bool, string, style, enum:Name, rgb, optional_style

OPTION_FIELDS: dict[str, list[tuple[tuple[str, ...], str, str]]] = {
    "BoxOptions": [
        (("gap",), "gap", "int"),
        (("padding",), "padding", "int"),
    ],
    "GridOptions": [
        (("columns",), "columns", "int"),
        (("gap",), "gap", "int"),
        (("padding",), "padding", "int"),
    ],
    "WindowOptions": [
        (("modal",), "modal", "bool"),
        (("movable",), "movable", "bool"),
        (("resizable",), "resizable", "bool"),
        (("closable",), "closable", "bool"),
    ],
    "SplitPaneOptions": [
        (("orientation",), "orientation", "enum:SplitOrientation"),
        (("firstSize", "first_size"), "first_size", "int"),
        (("dividerStyle", "divider_style"), "divider_style", "style"),
    ],
    "TabsOptions": [
        (("tabStyle", "tab_style"), "tab_style", "style"),
        (("selectedTabStyle", "selected_tab_style"), "selected_tab_style", "style"),
    ],
    "ScrollViewOptions": [
        (("width",), "width", "int"),
        (("height",), "height", "int"),
        (("background", "backgroundStyle"), "background", "style"),
    ],
    "ScrollbarConfig": [
        (("enabled",), "enabled", "bool"),
        (("vertical",), "vertical", "bool"),
        (("horizontal",), "horizontal", "bool"),
    ],
    "BigTextOptions": [
        (("kind",), "kind", "enum:BigTextKind"),
        (("letterSpacing", "letter_spacing"), "letter_spacing", "int"),
        (("scale",), "scale", "int"),
        (("rainbow",), "rainbow", "bool"),
        (("underline",), "underline", "string"),
        (("gradientAxis", "gradient_axis"), "gradient_axis", "enum:BigTextGradientAxis"),
        (("shadowStyle", "shadow_style"), "shadow_style", "optional_style"),
        (("shadowDx", "shadow_dx"), "shadow_dx", "int"),
        (("shadowDy", "shadow_dy"), "shadow_dy", "int"),
        (("shadowLayers", "shadow_layers"), "shadow_layers", "int"),
    ],
    "BorderGlyphs": [
        (("topLeft", "top_left"), "top_left", "string"),
        (("topRight", "top_right"), "top_right", "string"),
        (("bottomLeft", "bottom_left"), "bottom_left", "string"),
        (("bottomRight", "bottom_right"), "bottom_right", "string"),
        (("horizontal",), "horizontal", "string"),
        (("vertical",), "vertical", "string"),
        (("resizeHandle", "resize_handle"), "resize_handle", "string"),
    ],
    "TextInputOptions": [
        (("minWidth", "min_width"), "min_width", "int"),
        (("placeholder",), "placeholder", "string"),
    ],
    "TextAreaOptions": [
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("lineNumbers", "line_numbers"), "line_numbers", "bool"),
        (("statusBar", "status_bar"), "status_bar", "bool"),
        (("gutterWidth", "gutter_width"), "gutter_width", "int"),
        (("title",), "title", "string"),
        (("placeholder",), "placeholder", "string"),
    ],
    "TerminalFrameOptions": [
        (("showStatusLine", "show_status_line"), "show_status_line", "bool"),
        (("liveContent", "live_content"), "live_content", "bool"),
    ],
    "TerminalFrameStyle": [
        (("borderStyle", "border_style"), "border_style", "enum:BorderStyle"),
        (("showControls", "show_controls"), "show_controls", "bool"),
        (("border",), "border", "style"),
        (("title",), "title", "style"),
        (("titleBackground", "title_background"), "title_background", "style"),
        (("contentBackground", "content_background"), "content_background", "style"),
    ],
    "ProgressBarOptions": [
        (("layout",), "layout", "enum:ProgressBarLayout"),
        (("label",), "label", "string"),
        (("insideLabel", "inside_label"), "inside_label", "string"),
        (("metricValue", "metric_value"), "metric_value", "string"),
        (("showPercent", "show_percent"), "show_percent", "bool"),
        (("percentPosition", "percent_position"), "percent_position", "enum:ProgressBarPercentPosition"),
        (("segmented",), "segmented", "bool"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("animationPhase", "animation_phase"), "animation_phase", "int"),
        (("pulseWidth", "pulse_width"), "pulse_width", "int"),
        (("indeterminate",), "indeterminate", "bool"),
        (("completed",), "completed", "bool"),
        (("etaText", "eta_text"), "eta_text", "string"),
        (("fillStyle", "fill_style"), "fill_style", "style"),
        (("trackStyle", "track_style"), "track_style", "style"),
        (("labelStyle", "label_style"), "label_style", "style"),
        (("percentStyle", "percent_style"), "percent_style", "style"),
        (("valueStyle", "value_style"), "value_style", "style"),
        (("headStyle", "head_style"), "head_style", "style"),
        (("insideLabelStyle", "inside_label_style"), "inside_label_style", "style"),
        (("etaStyle", "eta_style"), "eta_style", "style"),
    ],
    "BarChartOptions": [
        (("style", "glyphStyle"), "style", "enum:ChartGlyphStyle"),
        (("customGlyph", "custom_glyph"), "custom_glyph", "string"),
        (("title",), "title", "string"),
        (("valueLabel", "value_label"), "value_label", "string"),
        (("orientation",), "orientation", "enum:BarChartOrientation"),
        (("minValue", "min_value"), "min_value", "float"),
        (("maxValue", "max_value"), "max_value", "float"),
        (("showValues", "show_values"), "show_values", "bool"),
        (("showAxis", "show_axis"), "show_axis", "bool"),
        (("showGrid", "show_grid"), "show_grid", "bool"),
        (("showLabels", "show_labels"), "show_labels", "bool"),
        (("interactive",), "interactive", "bool"),
        (("barGap", "bar_gap"), "bar_gap", "int"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("axisStyle", "axis_style"), "axis_style", "style"),
        (("gridStyle", "grid_style"), "grid_style", "style"),
        (("valueStyle", "value_style"), "value_style", "style"),
    ],
    "LineChartOptions": [
        (("style", "glyphStyle"), "style", "enum:ChartGlyphStyle"),
        (("customGlyph", "custom_glyph"), "custom_glyph", "string"),
        (("mode",), "mode", "enum:LineChartMode"),
        (("title",), "title", "string"),
        (("minValue", "min_value"), "min_value", "float"),
        (("maxValue", "max_value"), "max_value", "float"),
        (("showAxes", "show_axes"), "show_axes", "bool"),
        (("showGrid", "show_grid"), "show_grid", "bool"),
        (("showLegend", "show_legend"), "show_legend", "bool"),
        (("mirror",), "mirror", "bool"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("axisStyle", "axis_style"), "axis_style", "style"),
        (("gridStyle", "grid_style"), "grid_style", "style"),
        (("legendStyle", "legend_style"), "legend_style", "style"),
    ],
    "PieChartOptions": [
        (("style",), "style", "enum:PieChartStyle"),
        (("customGlyph", "custom_glyph"), "custom_glyph", "string"),
        (("title",), "title", "string"),
        (("showLegend", "show_legend"), "show_legend", "bool"),
        (("showPercent", "show_percent"), "show_percent", "bool"),
        (("interactive",), "interactive", "bool"),
        (("rotation",), "rotation", "float"),
        (("diameter",), "diameter", "int"),
        (("minSliceRatio", "min_slice_ratio"), "min_slice_ratio", "float"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("legendStyle", "legend_style"), "legend_style", "style"),
    ],
    "GaugeChartOptions": [
        (("style",), "style", "enum:GaugeStyle"),
        (("glyph",), "glyph", "enum:ChartGlyphStyle"),
        (("title",), "title", "string"),
        (("unit",), "unit", "string"),
        (("minValue", "min_value"), "min_value", "float"),
        (("maxValue", "max_value"), "max_value", "float"),
        (("showValue", "show_value"), "show_value", "bool"),
        (("showTicks", "show_ticks"), "show_ticks", "bool"),
        (("diameter",), "diameter", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("trackStyle", "track_style"), "track_style", "style"),
        (("fillStyle", "fill_style"), "fill_style", "style"),
        (("valueStyle", "value_style"), "value_style", "style"),
        (("tickStyle", "tick_style"), "tick_style", "style"),
    ],
    "HistogramOptions": [
        (("style", "glyphStyle"), "style", "enum:ChartGlyphStyle"),
        (("customGlyph", "custom_glyph"), "custom_glyph", "string"),
        (("title",), "title", "string"),
        (("minValue", "min_value"), "min_value", "float"),
        (("maxValue", "max_value"), "max_value", "float"),
        (("showAxis", "show_axis"), "show_axis", "bool"),
        (("showGrid", "show_grid"), "show_grid", "bool"),
        (("showLabels", "show_labels"), "show_labels", "bool"),
        (("showCounts", "show_counts"), "show_counts", "bool"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("axisStyle", "axis_style"), "axis_style", "style"),
        (("gridStyle", "grid_style"), "grid_style", "style"),
        (("countStyle", "count_style"), "count_style", "style"),
    ],
    "HeatmapOptions": [
        (("title",), "title", "string"),
        (("minValue", "min_value"), "min_value", "float"),
        (("maxValue", "max_value"), "max_value", "float"),
        (("showRowLabels", "show_row_labels"), "show_row_labels", "bool"),
        (("showColLabels", "show_col_labels"), "show_col_labels", "bool"),
        (("showLegend", "show_legend"), "show_legend", "bool"),
        (("style", "glyphStyle"), "style", "enum:ChartGlyphStyle"),
        (("cellGap", "cell_gap"), "cell_gap", "int"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("labelStyle", "label_style"), "label_style", "style"),
        (("lowStyle", "low_style"), "low_style", "style"),
        (("highStyle", "high_style"), "high_style", "style"),
    ],
    "CandlestickChartOptions": [
        (("title",), "title", "string"),
        (("minValue", "min_value"), "min_value", "float"),
        (("maxValue", "max_value"), "max_value", "float"),
        (("showAxis", "show_axis"), "show_axis", "bool"),
        (("showGrid", "show_grid"), "show_grid", "bool"),
        (("showLabels", "show_labels"), "show_labels", "bool"),
        (("showVolume", "show_volume"), "show_volume", "bool"),
        (("barWidth", "bar_width"), "bar_width", "int"),
        (("barGap", "bar_gap"), "bar_gap", "int"),
        (("compactLayout", "compact_layout"), "compact_layout", "bool"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("axisStyle", "axis_style"), "axis_style", "style"),
        (("gridStyle", "grid_style"), "grid_style", "style"),
        (("upStyle", "up_style"), "up_style", "style"),
        (("downStyle", "down_style"), "down_style", "style"),
        (("wickStyle", "wick_style"), "wick_style", "style"),
    ],
    "StackedAreaChartOptions": [
        (("style", "glyphStyle"), "style", "enum:ChartGlyphStyle"),
        (("title",), "title", "string"),
        (("minValue", "min_value"), "min_value", "float"),
        (("maxValue", "max_value"), "max_value", "float"),
        (("showAxes", "show_axes"), "show_axes", "bool"),
        (("showGrid", "show_grid"), "show_grid", "bool"),
        (("showLegend", "show_legend"), "show_legend", "bool"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("axisStyle", "axis_style"), "axis_style", "style"),
        (("gridStyle", "grid_style"), "grid_style", "style"),
        (("legendStyle", "legend_style"), "legend_style", "style"),
    ],
    "WaterfallChartOptions": [
        (("style", "glyphStyle"), "style", "enum:ChartGlyphStyle"),
        (("title",), "title", "string"),
        (("baseline",), "baseline", "float"),
        (("showAxis", "show_axis"), "show_axis", "bool"),
        (("showGrid", "show_grid"), "show_grid", "bool"),
        (("showLabels", "show_labels"), "show_labels", "bool"),
        (("showConnectors", "show_connectors"), "show_connectors", "bool"),
        (("barWidth", "bar_width"), "bar_width", "int"),
        (("barGap", "bar_gap"), "bar_gap", "int"),
        (("compactLayout", "compact_layout"), "compact_layout", "bool"),
        (("minWidth", "min_width"), "min_width", "int"),
        (("minHeight", "min_height"), "min_height", "int"),
        (("titleStyle", "title_style"), "title_style", "style"),
        (("axisStyle", "axis_style"), "axis_style", "style"),
        (("gridStyle", "grid_style"), "grid_style", "style"),
        (("connectorStyle", "connector_style"), "connector_style", "style"),
        (("totalStyle", "total_style"), "total_style", "style"),
    ],
    "SourceControlPanelStyle": [
        (("background",), "background", "style"),
        (("header",), "header", "style"),
        (("sectionTitle", "section_title"), "section_title", "style"),
        (("entryText", "entry_text"), "entry_text", "style"),
        (("muted",), "muted", "style"),
        (("footer",), "footer", "style"),
        (("panelBg", "panel_bg"), "panel_bg", "rgb"),
        (("textFg", "text_fg"), "text_fg", "rgb"),
        (("mutedFg", "muted_fg"), "muted_fg", "rgb"),
        (("selectionBg", "selection_bg"), "selection_bg", "rgb"),
        (("statusModified", "status_modified"), "status_modified", "rgb"),
        (("statusAdded", "status_added"), "status_added", "rgb"),
        (("statusDeleted", "status_deleted"), "status_deleted", "rgb"),
        (("preferredWidth", "preferred_width"), "preferred_width", "int"),
        (("footerText", "footer_text"), "footer_text", "string"),
        (("showFooter", "show_footer"), "show_footer", "bool"),
        (("nameMaxColumns", "name_max_columns"), "name_max_columns", "int"),
        (("tagMaxColumns", "tag_max_columns"), "tag_max_columns", "int"),
    ],
    "NerdGlyphMatrixGalleryStyle": [
        (("background",), "background", "style"),
        (("title",), "title", "style"),
        (("domainHeader", "domain_header"), "domain_header", "style"),
        (("categoryHeader", "category_header"), "category_header", "style"),
        (("glyph",), "glyph", "style"),
    ],
    "NerdIconCatalogGalleryStyle": [
        (("background",), "background", "style"),
        (("title",), "title", "style"),
        (("domainHeader", "domain_header"), "domain_header", "style"),
        (("categoryHeader", "category_header"), "category_header", "style"),
        (("rowText", "row_text"), "row_text", "style"),
        (("icon",), "icon", "style"),
    ],
    "ShellTerminalStyle": [
        (("defaultCell", "default_cell"), "default_cell", "style"),
        (("cursor",), "cursor", "style"),
    ],
    "CheckboxOptions": [
        (("highlightRowOnFocus", "highlight_row_on_focus"), "highlight_row_on_focus", "bool"),
        (("markerStyle", "marker_style"), "marker_style", "style"),
        (("markerCheckedStyle", "marker_checked_style"), "marker_checked_style", "style"),
        (("labelStyle", "label_style"), "label_style", "style"),
        (("labelCheckedStyle", "label_checked_style"), "label_checked_style", "style"),
        (("focusedStyle", "focused_style"), "focused_style", "style"),
    ],
    "CheckboxGlyphs": [
        (("unchecked",), "unchecked", "string"),
        (("checked",), "checked", "string"),
        (("gap",), "gap", "string"),
    ],
    "ProgressBarGlyphs": [
        (("fill",), "fill", "string"),
        (("head",), "head", "string"),
        (("empty",), "empty", "string"),
        (("leftCap", "left_cap"), "left_cap", "string"),
        (("rightCap", "right_cap"), "right_cap", "string"),
    ],
}


def merge_options_source(node: dict[str, Any]) -> dict[str, Any]:
    merged: dict[str, Any] = {}
    options = node.get("options")
    if isinstance(options, dict):
        merged.update(options)
    for key, value in node.items():
        if key not in {"type", "id", "children", "content", "first", "second", "tabs", "windows", "properties", "background", "contextMenu", "context_menu", "commandPalette", "command_palette"}:
            merged.setdefault(key, value)
    props = node.get("properties")
    if isinstance(props, dict):
        for key, value in props.items():
            merged.setdefault(key, value)
    return merged


def emit_rect(value: Any) -> str:
    if isinstance(value, list) and len(value) == 4:
        return f"tuinator::Rect{{{value[0]}, {value[1]}, {value[2]}, {value[3]}}}"
    if isinstance(value, dict):
        return (
            f"tuinator::Rect{{{int(value.get('x', 0))}, {int(value.get('y', 0))}, "
            f"{int(value.get('width', value.get('w', 0)))}, "
            f"{int(value.get('height', value.get('h', 0)))}}}"
        )
    raise ValueError(f"Invalid rect value: {value}")


def emit_hex_uint(value: Any) -> str:
    if isinstance(value, str):
        hex_val = value.strip().removeprefix("#")
        if len(hex_val) == 6 and all(c in "0123456789abcdefABCDEF" for c in hex_val):
            return f"0x{hex_val.lower()}"
        raise ValueError(f"Invalid hex color: {value}")
    if isinstance(value, dict):
        return (
            f"0x{int(value['r']):02x}{int(value['g']):02x}{int(value['b']):02x}"
        )
    if isinstance(value, int):
        return f"0x{value:08x}"[-8:]  # allow raw int
    raise ValueError(f"Invalid color value: {value}")


def emit_rgb(value: Any) -> str:
    if isinstance(value, dict) and "r" in value:
        return f"tuinator::Rgb{{{int(value['r'])}, {int(value['g'])}, {int(value['b'])}}}"
    if isinstance(value, str):
        hex_val = value.strip().removeprefix("#")
        if len(hex_val) == 6 and all(c in "0123456789abcdefABCDEF" for c in hex_val):
            r = int(hex_val[0:2], 16)
            g = int(hex_val[2:4], 16)
            b = int(hex_val[4:6], 16)
            return f"tuinator::Rgb{{{r}, {g}, {b}}}"
    raise ValueError(f"Invalid RGB value: {value}")


def emit_gradient_stops(
    stops: list[Any],
    helper: str,
    theme_var: str,
    emit_style: Callable[[Any, str], str],
) -> str:
    pairs: list[str] = []
    for stop in stops:
        if not isinstance(stop, dict):
            raise ValueError("Each gradient stop must be an object")
        position = float(stop.get("position", 0.0))
        color = stop.get("color")
        if color is None:
            raise ValueError("Gradient stop requires 'color'")
        pairs.append(f"{{{position}f, {emit_hex_uint(color)}}}")
    return f"tuinator::{helper}({{{', '.join(pairs)}}})"


def emit_border_glyphs(
    glyphs: dict[str, Any],
    theme_var: str,
    emit_style: Callable[[Any, str], str],
    emit_enum: Callable[[str, str], str],
) -> str:
    fields = OPTION_FIELDS["BorderGlyphs"]
    parts: list[str] = []
    for keys, member, kind in fields:
        value = lookup_field_value(glyphs, keys)
        if value is None:
            continue
        parts.append(f".{member} = {emit_value(kind, value, theme_var, emit_style, emit_enum)}")
    return "tuinator::BorderGlyphs{" + ", ".join(parts) + "}" if parts else "tuinator::BorderGlyphs{}"


def assign_scrollbar_options(
    state_nodes: list[str],
    target_var: str,
    source: dict[str, Any],
    theme_var: str,
    emit_style: Callable[[Any, str], str],
    emit_enum: Callable[[str, str], str],
) -> None:
    preset = lookup_field_value(source, ("preset", "scrollbarPreset", "scrollbar_preset"))
    if preset is not None:
        state_nodes.append(
            f"    {target_var} = tuinator::scrollbar_options("
            f"{theme_var}, {emit_enum('ScrollbarPreset', str(preset))});"
        )
    config = source.get("config")
    if isinstance(config, dict):
        assign_options_fields(
            state_nodes, f"{target_var}.config", config, "ScrollbarConfig",
            theme_var, emit_style, emit_enum,
        )
    else:
        assign_options_fields(
            state_nodes, f"{target_var}.config", source, "ScrollbarConfig",
            theme_var, emit_style, emit_enum,
        )


def emit_value(
    kind: str,
    value: Any,
    theme_var: str,
    emit_style: Callable[[Any, str], str],
    emit_enum: Callable[[str, str], str],
) -> str:
    if kind == "int":
        return str(int(value))
    if kind == "float":
        return str(float(value))
    if kind == "bool":
        return "true" if value else "false"
    if kind == "string":
        return json.dumps(str(value), ensure_ascii=True)
    if kind == "style":
        return emit_style(value, theme_var)
    if kind == "optional_style":
        return emit_style(value, theme_var)
    if kind.startswith("enum:"):
        enum_name = kind.split(":", 1)[1]
        return emit_enum(enum_name, str(value))
    if kind == "rgb":
        return emit_rgb(value)
    raise ValueError(f"Unknown field kind: {kind}")


def lookup_field_value(source: dict[str, Any], keys: tuple[str, ...]) -> Any:
    for key in keys:
        if key in source:
            return source[key]
    return None


def assign_options_fields(
    state_nodes: list[str],
    options_var: str,
    source: dict[str, Any],
    struct_name: str,
    theme_var: str,
    emit_style: Callable[[Any, str], str],
    emit_enum: Callable[[str, str], str],
) -> None:
    fields = OPTION_FIELDS.get(struct_name, [])
    for keys, member, kind in fields:
        value = lookup_field_value(source, keys)
        if value is None:
            continue
        rhs = emit_value(kind, value, theme_var, emit_style, emit_enum)
        state_nodes.append(f"    {options_var}.{member} = {rhs};")


def build_options_var(
    state_nodes: list[str],
    prefix: str,
    struct_name: str,
    node: dict[str, Any],
    theme_var: str,
    emit_style: Callable[[Any, str], str],
    emit_enum: Callable[[str, str], str],
    init_expr: str | None = None,
) -> str:
    var = f"{prefix}_options"
    if init_expr:
        state_nodes.append(f"    tuinator::{struct_name} {var} = {init_expr};")
    else:
        state_nodes.append(f"    tuinator::{struct_name} {var};")
    assign_options_fields(
        state_nodes,
        var,
        merge_options_source(node),
        struct_name,
        theme_var,
        emit_style,
        emit_enum,
    )
    return var


def assign_nested_style_fields(
    state_nodes: list[str],
    target_var: str,
    source: dict[str, Any],
    struct_name: str,
    theme_var: str,
    emit_style: Callable[[Any, str], str],
    emit_enum: Callable[[str, str], str],
) -> None:
    assign_options_fields(
        state_nodes,
        target_var,
        source,
        struct_name,
        theme_var,
        emit_style,
        emit_enum,
    )
