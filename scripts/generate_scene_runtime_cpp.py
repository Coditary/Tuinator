#!/usr/bin/env python3
"""Generate runtime scene widget builder C++ from widget metadata."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent))

from generate_scene_cpp import CPP_ENUMS, SUPPORTED_WIDGETS  # noqa: E402
from scene_options import OPTION_FIELDS  # noqa: E402

HEADER_PATH = (
    ROOT / "include/tuinator/scene/detail/widget_builder.generated.hpp"
)
SOURCE_PATH = ROOT / "src/scene/runtime/widget_builder.generated.cpp"

THEME_VAR = "ctx.theme"


def cpp_string(value: str) -> str:
    return json.dumps(value, ensure_ascii=True)


def rt_keys(*keys: str) -> str:
    return ", ".join(cpp_string(k) for k in keys)


def rt_string(*keys: str, default: str = '""') -> str:
    if default == '""':
        return f"json::node_string(node, {rt_keys(*keys)})"
    return f"json::node_string_default(node, {default}, {rt_keys(*keys)})"


def rt_int(*keys: str, default: int = 0) -> str:
    return f"json::node_int(node, {default}, {rt_keys(*keys)})"


def rt_bool(*keys: str, default: bool = False) -> str:
    return f"json::node_bool(node, {str(default).lower()}, {rt_keys(*keys)})"


def rt_double(*keys: str, default: float = 0.0) -> str:
    return f"json::node_double(node, {default}, {rt_keys(*keys)})"


def rt_style(*keys: str) -> str:
    return f"style::resolve_style(json::sub(node, {rt_keys(*keys)}), {THEME_VAR})"


def rt_invoke(handler: str) -> str:
    return f"ctx.invoke_handler({cpp_string(handler)})"


def rt_handler_lambda(handler: str | None) -> str:
    if not handler:
        return "{}"
    return f"[&ctx]() {{ {rt_invoke(handler)}; }}"


def rt_handler_lambda_bool(handler: str | None) -> str:
    if not handler:
        return "{}"
    return f"[&ctx](bool /*value*/) {{ {rt_invoke(handler)}; }}"


def rt_schedule(widget_expr: str, body: str) -> str:
    return (
        f"ctx.schedule_post_init([{widget_expr}]() {{\n"
        f"        {body}\n"
        f"    }});"
    )


def rt_register(widget_type: str, ptr_expr: str = "raw") -> str:
    return (
        'if (const std::string widget_id = json::node_string(node, "id"); '
        "!widget_id.empty()) {\n"
        f"            ctx.register_widget(widget_id, {cpp_string(widget_type)}, {ptr_expr});\n"
        "        }"
    )


def rt_enum_switch(
    enum_type: str,
    value_expr: str,
    target_expr: str,
    indent: str = "",
) -> list[str]:
    mapping = CPP_ENUMS[enum_type]
    lines = [f"{indent}{{ const std::string _enum_val = {value_expr};"]
    first = True
    for key, member in mapping.items():
        prefix = "if" if first else "else if"
        first = False
        lines.append(
            f"{indent}    {prefix} (_enum_val == {cpp_string(key)}) "
            f"{{ {target_expr} = tuinator::{enum_type}::{member}; }}"
        )
    default_member = next(iter(mapping.values()))
    lines.append(
        f"{indent}    else {{ {target_expr} = tuinator::{enum_type}::{default_member}; }}"
    )
    lines.append(f"{indent}}}")
    return lines


def rt_visit_field(key: str, body: str, indent: str = "    ", json_var: str = "node") -> str:
    return (
        f"{indent}if (const nlohmann::json* value = json::find({json_var}, {cpp_string(key)})) {{\n"
        f"{indent}    {body}\n"
        f"{indent}}}"
    )


def rt_assign_options_fields(
    options_var: str,
    struct_name: str,
    indent: str = "    ",
    json_var: str = "node",
) -> list[str]:
    lines: list[str] = []
    fields = OPTION_FIELDS.get(struct_name, [])
    for keys, member, kind in fields:
        key = keys[0]
        if kind == "int":
            rhs = f"json::as_int(*value, {options_var}.{member})"
        elif kind == "float":
            rhs = f"json::as_double(*value, {options_var}.{member})"
        elif kind == "bool":
            rhs = f"json::as_bool(*value, {options_var}.{member})"
        elif kind == "string":
            rhs = "json::as_string(*value)"
        elif kind == "style":
            rhs = f"style::resolve_style(*value, {THEME_VAR})"
        elif kind == "optional_style":
            rhs = f"style::resolve_style(*value, {THEME_VAR})"
        elif kind.startswith("enum:"):
            enum_name = kind.split(":", 1)[1]
            tmp = f"{options_var}.{member}"
            inner = rt_enum_switch(enum_name, "json::as_string(*value)", tmp)
            body = "\n".join(f"        {line}" for line in inner)
            lines.append(rt_visit_field(key, body, indent, json_var))
            for alt in keys[1:]:
                lines.append(rt_visit_field(alt, body, indent, json_var))
            continue
        elif kind == "rgb":
            rhs = "json::as_rgb(*value)"
        else:
            raise ValueError(f"Unknown field kind: {kind}")
        assign = f"{options_var}.{member} = {rhs};"
        lines.append(rt_visit_field(key, assign, indent, json_var))
        for alt in keys[1:]:
            lines.append(rt_visit_field(alt, assign, indent, json_var))
    return lines


def gen_fill_options_function(struct_name: str) -> list[str]:
    lines = [
        f"void fill_{struct_name}(tuinator::{struct_name}& opts, "
        f"const nlohmann::json& node, SceneContext& ctx) {{",
    ]
    lines.extend(rt_assign_options_fields("opts", struct_name))
    lines.append("}")
    return lines


def gen_runtime_helpers() -> list[str]:
    lines = [
        "const nlohmann::json& json_array_or_empty(const nlohmann::json& node, const char* key) {",
        "    static const nlohmann::json empty = nlohmann::json::array();",
        "    if (node.contains(key) && node[key].is_array()) {",
        "        return node[key];",
        "    }",
        "    return empty;",
        "}",
        "",
        "tuinator::TreeNode build_tree_node(const nlohmann::json& node) {",
        f"    tuinator::TreeNode tree_node{{{rt_string('label', 'text')}, {{}}, "
        f"{rt_bool('expanded', default=True)}}};",
        '    for (const auto& child : json_array_or_empty(node, "children")) {',
        "        tree_node.children.push_back(build_tree_node(child));",
        "    }",
        "    return tree_node;",
        "}",
        "",
        "void build_menu_items(",
        "    const nlohmann::json& items,",
        "    std::vector<tuinator::MenuItem>& out,",
        "    SceneContext& ctx) {",
        "    for (const auto& item : items) {",
        '        const std::string kind = json::node_string(item, "Action", "kind");',
        '        if (kind == "Separator") {',
        "            out.push_back(tuinator::MenuItem::separator());",
        "            continue;",
        "        }",
        '        const std::string label = json::node_string(item, "label");',
        '        const std::string shortcut = json::node_string(item, "shortcut");',
        '        const bool enabled = json::node_bool(item, true, "enabled");',
        '        const bool checked = json::node_bool(item, false, "checked");',
        '        const std::string on_click = json::node_string(item, "onClick", "on_click");',
        '        if (kind == "Checkbox") {',
        "            out.push_back(tuinator::MenuItem::checkbox(",
        "                label, checked, [&ctx, on_click]() {",
        "                    if (!on_click.empty()) { ctx.invoke_handler(on_click); }",
        "                }));",
        "            continue;",
        "        }",
        '        if (kind == "Submenu") {',
        "            std::vector<tuinator::MenuItem> children;",
        '            const nlohmann::json& child_items = item.contains("children")',
        '                ? item["children"]',
        '                : json_array_or_empty(item, "items");',
        "            build_menu_items(child_items, children, ctx);",
        "            out.push_back(tuinator::MenuItem::submenu(",
        '                label, std::move(children), shortcut, json::node_string(item, "icon")));',
        "            continue;",
        "        }",
        "        out.push_back(tuinator::MenuItem(",
        "            label,",
        "            [&ctx, on_click]() {",
        "                if (!on_click.empty()) { ctx.invoke_handler(on_click); }",
        "            },",
        "            shortcut,",
        "            enabled));",
        "    }",
        "}",
        "",
        "std::vector<tuinator::StatusSegment> build_status_segments(",
        "    const nlohmann::json& segments, SceneContext& ctx) {",
        "    std::vector<tuinator::StatusSegment> out;",
        "    for (const auto& segment : segments) {",
        "        tuinator::StatusSegment seg;",
        '        const std::string kind = json::node_string(segment, "Text", "kind");',
    ]
    lines.extend(
        f"        {line}"
        for line in rt_enum_switch(
            "StatusSegmentKind", "kind", "seg.kind"
        )
    )
    lines.extend([
        '        seg.text = json::node_string(segment, "text");',
        '        seg.bold = json::node_bool(segment, false, "bold");',
        '        seg.dim = json::node_bool(segment, false, "dim");',
        '        if (segment.contains("foreground") && segment["foreground"].is_object()) {',
        "            seg.foreground_rgb = json::as_rgb(segment[\"foreground\"]);",
        "        }",
        "        out.push_back(seg);",
        "    }",
        "    return out;",
        "}",
        "",
        "void apply_menu_bar_look(tuinator::MenuBar& bar, const std::string& look, SceneContext& ctx) {",
        "    const std::string preset = look;",
        '    if (preset == "classic") {',
        "        bar.apply_look(tuinator::menu_bar_look_classic(ctx.theme));",
        "        return;",
        "    }",
        '    if (preset == "mac") {',
        "        bar.apply_look(tuinator::menu_bar_look_mac(ctx.theme));",
        "        return;",
        "    }",
        '    if (preset == "minimal") {',
        "        bar.apply_look(tuinator::menu_bar_look_minimal(ctx.theme));",
        "        return;",
        "    }",
        "    if (const tuinator::MenuBarLook* named = tuinator::menu_bar_look_named(look)) {",
        "        bar.apply_look(*named);",
        "    }",
        "}",
        "",
        "void apply_context_menu_look(tuinator::ContextMenu& menu, const std::string& look, SceneContext& ctx) {",
        '    if (look == "classic") {',
        "        menu.set_look(tuinator::menu_bar_look_classic(ctx.theme));",
        "        return;",
        "    }",
        '    if (look == "mac") {',
        "        menu.set_look(tuinator::menu_bar_look_mac(ctx.theme));",
        "        return;",
        "    }",
        '    if (look == "minimal") {',
        "        menu.set_look(tuinator::menu_bar_look_minimal(ctx.theme));",
        "        return;",
        "    }",
        "    if (const tuinator::MenuBarLook* named = tuinator::menu_bar_look_named(look)) {",
        "        menu.set_look(*named);",
        "    }",
        "}",
        "",
        "std::vector<tuinator::BarChartBar> build_bar_chart_bars(",
        "    const nlohmann::json& bars, SceneContext& ctx) {",
        "    std::vector<tuinator::BarChartBar> out;",
        "    for (const auto& bar : bars) {",
        '        const std::string label = json::node_string(bar, "label");',
        '        const double value = json::node_double(bar, 0.0, "value");',
        '        if (bar.contains("style")) {',
        "            out.push_back(tuinator::BarChartBar{",
        "                label, value, style::resolve_style(bar[\"style\"], ctx.theme)});",
        "        } else {",
        "            out.push_back(tuinator::BarChartBar{label, value});",
        "        }",
        "    }",
        "    return out;",
        "}",
        "",
        "std::vector<tuinator::LineChartSeries> build_line_chart_series(",
        "    const nlohmann::json& series, SceneContext& ctx) {",
        "    std::vector<tuinator::LineChartSeries> out;",
        "    for (const auto& entry : series) {",
        '        const std::string label = json::node_string(entry, "label");',
        "        std::vector<double> values;",
        '        for (const auto& value : json_array_or_empty(entry, "values")) {',
        "            values.push_back(json::as_double(value, 0.0));",
        "        }",
        "        out.push_back(tuinator::LineChartSeries{",
        "            label,",
        "            std::move(values),",
        "            style::resolve_style(json::sub(entry, \"style\"), ctx.theme)});",
        "    }",
        "    return out;",
        "}",
        "",
        "std::vector<tuinator::StackedAreaSeries> build_stacked_area_series(",
        "    const nlohmann::json& series, SceneContext& ctx) {",
        "    std::vector<tuinator::StackedAreaSeries> out;",
        "    for (const auto& entry : series) {",
        '        const std::string label = json::node_string(entry, "label");',
        "        std::vector<double> values;",
        '        for (const auto& value : json_array_or_empty(entry, "values")) {',
        "            values.push_back(json::as_double(value, 0.0));",
        "        }",
        "        out.push_back(tuinator::StackedAreaSeries{",
        "            label,",
        "            std::move(values),",
        "            style::resolve_style(json::sub(entry, \"style\"), ctx.theme)});",
        "    }",
        "    return out;",
        "}",
        "",
        "tuinator::Rect parse_rect(const nlohmann::json& value) {",
        "    if (value.is_array() && value.size() == 4) {",
        "        return tuinator::Rect{",
        "            value[0].get<int>(), value[1].get<int>(),",
        "            value[2].get<int>(), value[3].get<int>()};",
        "    }",
        "    if (value.is_object()) {",
        '        return tuinator::Rect{',
        '            json::node_int(value, 0, "x"),',
        '            json::node_int(value, 0, "y"),',
        '            json::node_int(value, 0, "width", "w"),',
        '            json::node_int(value, 0, "height", "h")};',
        "    }",
        "    return tuinator::Rect{0, 0, 40, 12};",
        "}",
        "",
        "void fill_scrollbar_options(",
        "    tuinator::ScrollbarOptions& target,",
        "    const nlohmann::json& source,",
        "    SceneContext& ctx) {",
        '    if (const nlohmann::json* preset = json::find(source, "preset", "scrollbarPreset", "scrollbar_preset")) {',
        "        const std::string preset_name = json::as_string(*preset);",
        "        tuinator::ScrollbarPreset preset_enum = tuinator::ScrollbarPreset::Classic;",
    ])
    lines.extend(
        rt_enum_switch("ScrollbarPreset", "preset_name", "preset_enum", indent="        ")
    )
    lines.extend([
        "        target = tuinator::scrollbar_options(ctx.theme, preset_enum);",
        "    }",
        '    if (const nlohmann::json* config = json::find(source, "config")) {',
        "        fill_ScrollbarConfig(target.config, *config, ctx);",
        "    } else {",
        "        fill_ScrollbarConfig(target.config, source, ctx);",
        "    }",
        "}",
        "",
        "void create_windows(",
        "    tuinator::Desktop& host,",
        '    const char* method,',
        "    const nlohmann::json& windows,",
        "    SceneContext& ctx,",
        "    bool allow_modal) {",
        "    for (const auto& win : windows) {",
        '        if (!win.contains("content")) {',
        "            continue;",
        "        }",
        "        auto content = build_widget(win[\"content\"], ctx);",
        '        const tuinator::Rect bounds = parse_rect(json::sub(win, "bounds"));',
        '        const std::string title = json::node_string_default(win, "Window", "title");',
        "        tuinator::WindowOptions win_opts{};",
        "        fill_WindowOptions(win_opts, win, ctx);",
        '        const bool modal = json::node_bool(win, false, "modal");',
        "        if (modal && allow_modal) {",
        "            host.show_modal(title, bounds, std::move(content));",
        "            continue;",
        "        }",
        "        tuinator::Window* window_ptr = nullptr;",
        '        if (std::string(method) == "create_window") {',
        "            window_ptr = host.create_window(title, bounds, std::move(content), win_opts);",
        "        }",
        '        const std::string on_close = json::node_string(win, "onClose", "on_close");',
        "        if (window_ptr && !on_close.empty()) {",
        "            ctx.schedule_post_init([window_ptr, &ctx, on_close]() {",
        "                window_ptr->set_on_close([&ctx, on_close]() { ctx.invoke_handler(on_close); });",
        "            });",
        "        }",
        "    }",
        "}",
        "",
        "void create_windows(",
        "    tuinator::WindowHost& host,",
        '    const char* method,',
        "    const nlohmann::json& windows,",
        "    SceneContext& ctx,",
        "    bool allow_modal) {",
        "    for (const auto& win : windows) {",
        '        if (!win.contains("content")) {',
        "            continue;",
        "        }",
        "        auto content = build_widget(win[\"content\"], ctx);",
        '        const tuinator::Rect bounds = parse_rect(json::sub(win, "bounds"));',
        '        const std::string title = json::node_string_default(win, "Window", "title");',
        "        tuinator::WindowOptions win_opts{};",
        "        fill_WindowOptions(win_opts, win, ctx);",
        '        const bool modal = json::node_bool(win, false, "modal");',
        "        if (modal && allow_modal) {",
        '            throw std::runtime_error("Modal windows require a Desktop host");',
        "        }",
        "        tuinator::Window* window_ptr = nullptr;",
        '        if (std::string(method) == "create_window") {',
        "            window_ptr = host.create_window(title, bounds, std::move(content), win_opts);",
        "        }",
        '        const std::string on_close = json::node_string(win, "onClose", "on_close");',
        "        if (window_ptr && !on_close.empty()) {",
        "            ctx.schedule_post_init([window_ptr, &ctx, on_close]() {",
        "                window_ptr->set_on_close([&ctx, on_close]() { ctx.invoke_handler(on_close); });",
        "            });",
        "        }",
        "    }",
        "}",
    ])
    return lines


def gen_options_fill_functions() -> list[str]:
    used_structs = sorted(
        {
            "BoxOptions",
            "GridOptions",
            "SplitPaneOptions",
            "TabsOptions",
            "ScrollViewOptions",
            "ScrollbarConfig",
            "TextInputOptions",
            "TextAreaOptions",
            "TerminalFrameOptions",
            "TerminalFrameStyle",
            "CheckboxOptions",
            "CheckboxGlyphs",
            "BigTextOptions",
            "ProgressBarOptions",
            "ProgressBarGlyphs",
            "BarChartOptions",
            "LineChartOptions",
            "PieChartOptions",
            "GaugeChartOptions",
            "HistogramOptions",
            "HeatmapOptions",
            "CandlestickChartOptions",
            "StackedAreaChartOptions",
            "WaterfallChartOptions",
            "SourceControlPanelStyle",
            "NerdGlyphMatrixGalleryStyle",
            "NerdIconCatalogGalleryStyle",
            "ShellTerminalStyle",
            "WindowOptions",
            "BorderGlyphs",
        }
    )
    lines: list[str] = []
    for struct_name in used_structs:
        if struct_name == "BorderGlyphs":
            continue
        lines.extend(gen_fill_options_function(struct_name))
        lines.append("")
    lines.extend([
        "tuinator::BorderGlyphs build_border_glyphs(const nlohmann::json& glyphs, SceneContext& ctx) {",
        "    tuinator::BorderGlyphs result{};",
    ])
    lines.extend(rt_assign_options_fields("result", "BorderGlyphs", json_var="glyphs"))
    lines.extend([
        "    return result;",
        "}",
    ])
    return lines


def gen_widget_cases() -> list[str]:
    cases: list[str] = []

    def case(widget_type: str, body: list[str]) -> None:
        cases.append(f'    if (type == {cpp_string(widget_type)}) {{')
        cases.extend(f"        {line}" for line in body)
        cases.append("        return widget;")
        cases.append("    }")

    # VBox / HBox
    for box_type in ("VBox", "HBox"):
        case(box_type, [
            f"auto widget = std::make_unique<tuinator::{box_type}>(tuinator::BoxOptions{{{rt_int('gap', default=1)}, {rt_int('padding', default=0)}}});",
            f"if (const nlohmann::json* flex = json::find(node, \"flex\")) {{",
            "    widget->set_flex(json::as_int(*flex, 0));",
            "}",
            f"tuinator::{box_type}* raw = widget.get();",
            'for (const auto& child_node : json_array_or_empty(node, "children")) {',
            "    auto child = build_widget(child_node, ctx);",
            "    widget->add_child(std::move(child));",
            "}",
            rt_register(box_type),
        ])

    case("Grid", [
        f"auto widget = std::make_unique<tuinator::Grid>(tuinator::GridOptions{{{rt_int('columns', default=2)}, {rt_int('gap', default=1)}, {rt_int('padding', default=0)}}});",
        "tuinator::Grid* raw = widget.get();",
        'for (const auto& child_node : json_array_or_empty(node, "children")) {',
        "    auto child = build_widget(child_node, ctx);",
        "    widget->add_child(std::move(child));",
        "}",
        rt_register("Grid"),
    ])

    case("Panel", [
        f"const std::string title = {rt_string('title')};",
        f"const tuinator::Style border_style = {rt_style('borderStyle', 'border')};",
        f"const tuinator::Style title_style = {rt_style('titleStyle', 'heading')};",
        'if (node.contains("children")) {',
        '    throw std::runtime_error("Panel supports a single \'content\' node, not \'children\'");',
        "}",
        "std::unique_ptr<tuinator::Panel> widget;",
        'if (const nlohmann::json* glyphs = json::find(node, "borderGlyphs", "border_glyphs", "glyphs")) {',
        "    widget = std::make_unique<tuinator::Panel>(",
        "        title, border_style, title_style, build_border_glyphs(*glyphs, ctx));",
        "} else {",
        "    widget = std::make_unique<tuinator::Panel>(title, border_style, title_style);",
        "}",
        'if (node.contains("content")) {',
        "    auto content = build_widget(node[\"content\"], ctx);",
        "    widget->set_content(std::move(content));",
        "}",
        "tuinator::Panel* raw = widget.get();",
        rt_register("Panel"),
    ])

    case("ScrollView", [
        'if (!node.contains("content")) {',
        '    throw std::runtime_error("ScrollView requires a \'content\' node");',
        "}",
        "auto content = build_widget(node[\"content\"], ctx);",
        "tuinator::ScrollViewOptions opts{};",
        "fill_ScrollViewOptions(opts, node, ctx);",
        'if (const nlohmann::json* scrollbars = json::find(node, "scrollbars", "scrollbar")) {',
        "    fill_scrollbar_options(opts.scrollbars, *scrollbars, ctx);",
        "}",
        "auto widget = std::make_unique<tuinator::ScrollView>(std::move(content), opts);",
        "tuinator::ScrollView* raw = widget.get();",
        rt_register("ScrollView"),
    ])

    case("SplitPane", [
        'if (!node.contains("first") || !node.contains("second")) {',
        '    throw std::runtime_error("SplitPane requires \'first\' and \'second\' nodes");',
        "}",
        "auto first = build_widget(node[\"first\"], ctx);",
        "auto second = build_widget(node[\"second\"], ctx);",
        "tuinator::SplitPaneOptions opts{};",
        "fill_SplitPaneOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::SplitPane>(",
        "    std::move(first), std::move(second), opts);",
        "tuinator::SplitPane* raw = widget.get();",
        rt_register("SplitPane"),
    ])

    case("Tabs", [
        "tuinator::TabsOptions opts{};",
        "fill_TabsOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::Tabs>(opts);",
        "tuinator::Tabs* raw = widget.get();",
        'for (const auto& tab : json_array_or_empty(node, "tabs")) {',
        '    const std::string title = json::node_string(tab, "Tab", "title");',
        '    if (!tab.contains("content")) {',
        '        throw std::runtime_error("Each tab requires a \'content\' node");',
        "    }",
        "    auto content = build_widget(tab[\"content\"], ctx);",
        "    widget->add_tab(title, std::move(content));",
        "}",
        f'if (const nlohmann::json* selected = json::find(node, "selectedIndex", "selected_index")) {{',
        "    ctx.schedule_post_init([raw, selected_index = json::as_int(*selected, 0)]() {",
        "        raw->set_selected_index(selected_index);",
        "    });",
        "}",
        rt_register("Tabs"),
    ])

    case("TerminalFrame", [
        f"const std::string title = {rt_string('title', default='"Terminal"')};",
        'if (!node.contains("content")) {',
        '    throw std::runtime_error("TerminalFrame requires a \'content\' node");',
        "}",
        "auto content = build_widget(node[\"content\"], ctx);",
        "tuinator::TerminalFrameOptions opts{};",
        "fill_TerminalFrameOptions(opts, node, ctx);",
        'if (const nlohmann::json* frame_style = json::find(node, "frameStyle", "frame_style", "style")) {',
        "    fill_TerminalFrameStyle(opts.style, *frame_style, ctx);",
        "}",
        "auto widget = std::make_unique<tuinator::TerminalFrame>(title, std::move(content), opts);",
        "tuinator::TerminalFrame* raw = widget.get();",
        'if (const nlohmann::json* left = json::find(node, "statusLeft", "status_left")) {',
        "    auto segs = build_status_segments(*left, ctx);",
        "    ctx.schedule_post_init([raw, segs = std::move(segs)]() mutable {",
        "        raw->set_status_left(std::move(segs));",
        "    });",
        "}",
        'if (const nlohmann::json* center = json::find(node, "statusCenter", "status_center")) {',
        "    auto segs = build_status_segments(*center, ctx);",
        "    ctx.schedule_post_init([raw, segs = std::move(segs)]() mutable {",
        "        raw->set_status_center(std::move(segs));",
        "    });",
        "}",
        'if (const nlohmann::json* right = json::find(node, "statusRight", "status_right")) {',
        "    auto segs = build_status_segments(*right, ctx);",
        "    ctx.schedule_post_init([raw, segs = std::move(segs)]() mutable {",
        "        raw->set_status_right(std::move(segs));",
        "    });",
        "}",
        rt_register("TerminalFrame"),
    ])

    case("Label", [
        f"auto widget = std::make_unique<tuinator::Label>({rt_string('text')}, {rt_style('style')});",
        "tuinator::Label* raw = widget.get();",
        rt_register("Label"),
    ])

    case("Button", [
        f"const std::string label = {rt_string('label', 'text', default='"Button"')};",
        f"const tuinator::Style style = {rt_style('style')};",
        'const std::string on_click = json::node_string(node, "onClick", "on_click");',
        "auto widget = std::make_unique<tuinator::Button>(",
        "    label, [&ctx, on_click]() { if (!on_click.empty()) { ctx.invoke_handler(on_click); } }, style);",
        "tuinator::Button* raw = widget.get();",
        rt_register("Button"),
    ])

    case("TextInput", [
        "tuinator::TextInputOptions opts{};",
        "fill_TextInputOptions(opts, node, ctx);",
        f"const tuinator::Style style = {rt_style('style')};",
        f"const tuinator::Style focused = {rt_style('focusedStyle', 'text_input_focused')};",
        "auto widget = std::make_unique<tuinator::TextInput>(opts, style, focused);",
        f'if (const nlohmann::json* value = json::find(node, "value")) {{',
        "    widget->set_value(json::as_string(*value));",
        "}",
        "tuinator::TextInput* raw = widget.get();",
        rt_register("TextInput"),
    ])

    case("TextArea", [
        "tuinator::TextAreaOptions opts{};",
        "fill_TextAreaOptions(opts, node, ctx);",
        'if (const nlohmann::json* scrollbars = json::find(node, "scrollbars", "scrollbar")) {',
        "    fill_scrollbar_options(opts.scrollbars, *scrollbars, ctx);",
        "}",
        f"const tuinator::Style style = {rt_style('style')};",
        f"const tuinator::Style focused = {rt_style('focusedStyle', 'text_input_focused')};",
        "auto widget = std::make_unique<tuinator::TextArea>(opts, style, focused);",
        'if (const nlohmann::json* value = json::find(node, "value")) {',
        "    widget->set_value(json::as_string(*value));",
        "}",
        "tuinator::TextArea* raw = widget.get();",
        'const std::string on_change = json::node_string(node, "onChange", "on_change");',
        "if (!on_change.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_change]() {",
        "        raw->set_on_change([&ctx, on_change](const std::string& /*value*/) {",
        "            ctx.invoke_handler(on_change);",
        "        });",
        "    });",
        "}",
        rt_register("TextArea"),
    ])

    case("Separator", [
        f"auto widget = std::make_unique<tuinator::Separator>({rt_style('style', 'border')});",
        "tuinator::Separator* raw = widget.get();",
        rt_register("Separator"),
    ])

    case("Checkbox", [
        f"const std::string label = {rt_string('label', 'text')};",
        f"const bool checked = {rt_bool('checked')};",
        "tuinator::CheckboxOptions opts = tuinator::checkbox_options_default(ctx.theme);",
        'const std::string style_name = json::node_string(node, "styleName", "style_name", "checkboxStyle", "checkbox_style");',
        "if (!style_name.empty()) {",
        "    if (const tuinator::CheckboxStyle* named = tuinator::checkbox_style_named(style_name)) {",
        "        tuinator::apply_checkbox_style(opts, *named, ctx.theme);",
        "    }",
        "}",
        "fill_CheckboxOptions(opts, node, ctx);",
        'if (const nlohmann::json* glyphs = json::find(node, "glyphs")) {',
        "    fill_CheckboxGlyphs(opts.glyphs, *glyphs, ctx);",
        "}",
        'const std::string on_change = json::node_string(node, "onChange", "on_change");',
        "auto widget = std::make_unique<tuinator::Checkbox>(",
        "    label, checked, opts, [&ctx, on_change](bool /*value*/) {",
        "        if (!on_change.empty()) { ctx.invoke_handler(on_change); }",
        "    });",
        "tuinator::Checkbox* raw = widget.get();",
        rt_register("Checkbox"),
    ])

    case("Toggle", [
        f"const std::string label = {rt_string('label', 'text')};",
        f"const bool checked = {rt_bool('checked')};",
        f"const tuinator::Style style = {rt_style('style')};",
        f"const tuinator::Style checked_style = {rt_style('checkedStyle', 'accent')};",
        'const std::string on_change = json::node_string(node, "onChange", "on_change");',
        "auto widget = std::make_unique<tuinator::Toggle>(",
        "    label, checked, [&ctx, on_change](bool /*value*/) {",
        "        if (!on_change.empty()) { ctx.invoke_handler(on_change); }",
        "    }, style, checked_style);",
        "tuinator::Toggle* raw = widget.get();",
        rt_register("Toggle"),
    ])

    case("Slider", [
        f"const int min_value = {rt_int('min', 'minValue')};",
        f"const int max_value = {rt_int('max', 'maxValue', default=100)};",
        f"const int value = {rt_int('value')};",
        f"const int min_width = {rt_int('minWidth', 'min_width', default=20)};",
        f"const tuinator::Style style = {rt_style('style')};",
        'const std::string on_change = json::node_string(node, "onChange", "on_change");',
        "auto widget = std::make_unique<tuinator::Slider>(",
        "    min_value, max_value, value, [&ctx, on_change](int /*value*/) {",
        "        if (!on_change.empty()) { ctx.invoke_handler(on_change); }",
        "    }, style, min_width);",
        "tuinator::Slider* raw = widget.get();",
        rt_register("Slider"),
    ])

    case("Spinner", [
        f"const int min_value = {rt_int('min', 'minValue')};",
        f"const int max_value = {rt_int('max', 'maxValue', default=100)};",
        f"const int value = {rt_int('value')};",
        f"const int step = {rt_int('step', default=1)};",
        f"const tuinator::Style style = {rt_style('style')};",
        'const std::string on_change = json::node_string(node, "onChange", "on_change");',
        "auto widget = std::make_unique<tuinator::Spinner>(",
        "    min_value, max_value, value, step, [&ctx, on_change](int /*value*/) {",
        "        if (!on_change.empty()) { ctx.invoke_handler(on_change); }",
        "    }, style);",
        "tuinator::Spinner* raw = widget.get();",
        rt_register("Spinner"),
    ])

    case("ComboBox", [
        f"const tuinator::Style item_style = {rt_style('itemStyle')};",
        f"const tuinator::Style selected_style = {rt_style('selectedStyle', 'accent')};",
        "auto widget = std::make_unique<tuinator::ComboBox>(item_style, selected_style);",
        "tuinator::ComboBox* raw = widget.get();",
        'if (const nlohmann::json* items = json::find(node, "items")) {',
        "    std::vector<std::string> item_values;",
        "    for (const auto& item : *items) { item_values.push_back(json::as_string(item)); }",
        "    widget->set_items(std::move(item_values));",
        "}",
        'if (const nlohmann::json* selected = json::find(node, "selectedIndex", "selected_index")) {',
        "    widget->set_selected_index(json::as_int(*selected, 0));",
        "}",
        'const std::string on_select = json::node_string(node, "onSelect", "on_select");',
        "if (!on_select.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_select]() {",
        "        raw->set_on_select([&ctx, on_select](int /*index*/, const std::string& /*item*/) {",
        "            ctx.invoke_handler(on_select);",
        "        });",
        "    });",
        "}",
        rt_register("ComboBox"),
    ])

    progress_bar_lines = [
        f"const double value = {rt_double('value')};",
        "tuinator::ProgressBarOptions opts{};",
        'const std::string layout = json::node_string(node, "layout");',
        "if (!layout.empty()) {",
    ]
    progress_bar_lines.extend(
        rt_enum_switch("ProgressBarLayout", "layout", "opts.layout", indent="    ")
    )
    progress_bar_lines.extend([
        "    opts = tuinator::progress_bar_preset(opts.layout, ctx.theme.accent, ctx.theme.muted);",
        "}",
        "fill_ProgressBarOptions(opts, node, ctx);",
        'if (const nlohmann::json* stats = json::find(node, "stats")) {',
        '    opts.stats.current = json::node_int(*stats, 0, "current");',
        '    opts.stats.total = json::node_int(*stats, 100, "total");',
        '    opts.stats.unit = json::node_string(*stats, "step", "unit");',
        '    opts.stats.rate = json::node_double(*stats, 0.0, "rate");',
        '    opts.stats.elapsed_ms = json::node_int(*stats, 0, "elapsedMs", "elapsed_ms");',
        "}",
        'if (const nlohmann::json* glyphs = json::find(node, "glyphs")) {',
        "    fill_ProgressBarGlyphs(opts.glyphs, *glyphs, ctx);",
        "}",
        'if (const nlohmann::json* stops = json::find(node, "gradientStops", "gradient_stops")) {',
        "    opts.gradient_stops = json::build_progress_bar_gradient(*stops, ctx.theme);",
        "}",
        "auto widget = std::make_unique<tuinator::ProgressBar>(value, opts);",
        "tuinator::ProgressBar* raw = widget.get();",
        rt_register("ProgressBar"),
    ])
    case("ProgressBar", progress_bar_lines)

    case("Throbber", [
        f"const std::string name = {rt_string('name', 'set', default='"dots"')};",
        f"const tuinator::Style style = {rt_style('style')};",
        "auto widget = std::make_unique<tuinator::Throbber>(name, style);",
        "tuinator::Throbber* raw = widget.get();",
        rt_register("Throbber"),
    ])

    big_text_lines = [
        f"const std::string text = {rt_string('text')};",
        f"const tuinator::Style style = {rt_style('style')};",
        'const std::string look_name = json::node_string(node, "lookName", "look_name", "look");',
        'const std::string style_name = json::node_string(node, "styleName", "style_name");',
        'const std::string kind = json::node_string(node, "kind");',
        "auto widget = [&]() -> std::unique_ptr<tuinator::BigText> {",
        "    tuinator::BigTextOptions opts{};",
        "    if (!look_name.empty()) {",
        "        if (const tuinator::BigTextLook* look = tuinator::big_text_look_named(look_name)) {",
        "            tuinator::apply_big_text_look(opts, *look);",
        "        }",
        "    }",
        "    fill_BigTextOptions(opts, node, ctx);",
        '    if (const nlohmann::json* stops = json::find(node, "gradientStops", "gradient_stops")) {',
        "        opts.gradient_stops = json::build_big_text_gradient(*stops, ctx.theme);",
        "    }",
        '    if (const nlohmann::json* palette = json::find(node, "palette")) {',
        "        for (const auto& entry : *palette) {",
        "            opts.palette.push_back(style::resolve_style(entry, ctx.theme));",
        "        }",
        "    }",
        "    if (!look_name.empty() || json::has_any_option(node, {",
        '        "kind", "letterSpacing", "letter_spacing", "scale", "rainbow", "underline",',
        '        "gradientAxis", "gradient_axis", "gradientStops", "gradient_stops",',
        '        "shadowStyle", "shadow_style", "shadowDx", "shadow_dx", "shadowDy", "shadow_dy",',
        '        "shadowLayers", "shadow_layers", "palette", "look", "lookName", "look_name"})) {',
        "        return std::make_unique<tuinator::BigText>(text, opts, style);",
        "    }",
        "    if (!style_name.empty()) {",
        "        return std::make_unique<tuinator::BigText>(text, style_name, style);",
        "    }",
        "    if (!kind.empty()) {",
    ]
    big_text_lines.extend(
        rt_enum_switch("BigTextKind", "kind", "opts.kind", indent="        ")
    )
    big_text_lines.extend([
        "        return std::make_unique<tuinator::BigText>(text, opts.kind, style);",
        "    }",
        "    return std::make_unique<tuinator::BigText>(text, style);",
        "}();",
        "tuinator::BigText* raw = widget.get();",
        rt_register("BigText"),
    ])
    case("BigText", big_text_lines)

    case("ImageView", [
        f"const std::string path = {rt_string('path', 'image', default='"examples/assets/lenna.png"')};",
        f"const int width = {rt_int('width', 'displayWidth')};",
        f"const int height = {rt_int('height', 'displayHeight')};",
        "tuinator::TerminalImage image;",
        "if (auto loaded = tuinator::TerminalImage::load_png(path)) {",
        "    image = std::move(*loaded);",
        "} else if (auto loaded = tuinator::TerminalImage::load_ppm(path)) {",
        "    image = std::move(*loaded);",
        "}",
        "std::unique_ptr<tuinator::ImageView> widget;",
        "if (width > 0 && height > 0) {",
        "    widget = std::make_unique<tuinator::ImageView>(",
        "        std::move(image), tuinator::Size{width, height});",
        "} else {",
        "    widget = std::make_unique<tuinator::ImageView>(std::move(image));",
        "}",
        "tuinator::ImageView* raw = widget.get();",
        rt_register("ImageView"),
    ])

    case("NerdIconCatalogGallery", [
        "tuinator::NerdIconCatalogGalleryStyle style_opts{};",
        "fill_NerdIconCatalogGalleryStyle(style_opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::NerdIconCatalogGallery>(style_opts);",
        "tuinator::NerdIconCatalogGallery* raw = widget.get();",
        rt_register("NerdIconCatalogGallery"),
    ])

    case("NerdGlyphMatrixGallery", [
        f"const int columns = {rt_int('columns', default=32)};",
        f"const std::string data_root = {rt_string('dataRoot', 'data_root', default='"data"')};",
        "auto sections = tuinator::load_nerd_glyph_catalog(data_root);",
        "tuinator::NerdGlyphMatrixGalleryStyle style_opts{};",
        "fill_NerdGlyphMatrixGalleryStyle(style_opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::NerdGlyphMatrixGallery>(",
        "    std::move(sections), columns, style_opts);",
        "tuinator::NerdGlyphMatrixGallery* raw = widget.get();",
        rt_register("NerdGlyphMatrixGallery"),
    ])

    case("ListView", [
        f"const tuinator::Style item_style = {rt_style('itemStyle')};",
        f"const tuinator::Style selected_style = {rt_style('selectedStyle', 'accent')};",
        "auto widget = std::make_unique<tuinator::ListView>(item_style, selected_style);",
        "tuinator::ListView* raw = widget.get();",
        'if (const nlohmann::json* items = json::find(node, "items")) {',
        "    std::vector<std::string> item_values;",
        "    for (const auto& item : *items) { item_values.push_back(json::as_string(item)); }",
        "    widget->set_items(std::move(item_values));",
        "}",
        'if (const nlohmann::json* selected = json::find(node, "selectedIndex", "selected_index")) {',
        "    widget->set_selected_index(json::as_int(*selected, 0));",
        "}",
        'const std::string on_select = json::node_string(node, "onSelect", "on_select");',
        'const std::string on_activate = json::node_string(node, "onActivate", "on_activate");',
        "if (!on_select.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_select]() {",
        "        raw->set_on_select([&ctx, on_select](int /*index*/, const std::string& /*item*/) {",
        "            ctx.invoke_handler(on_select);",
        "        });",
        "    });",
        "}",
        "if (!on_activate.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_activate]() {",
        "        raw->set_on_activate([&ctx, on_activate](int /*index*/, const std::string& /*item*/) {",
        "            ctx.invoke_handler(on_activate);",
        "        });",
        "    });",
        "}",
        rt_register("ListView"),
    ])

    case("Table", [
        f"const tuinator::Style header_style = {rt_style('headerStyle', 'heading')};",
        f"const tuinator::Style cell_style = {rt_style('cellStyle')};",
        f"const tuinator::Style selected_style = {rt_style('selectedStyle', 'accent')};",
        "auto widget = std::make_unique<tuinator::Table>(header_style, cell_style, selected_style);",
        "tuinator::Table* raw = widget.get();",
        'if (const nlohmann::json* columns = json::find(node, "columns")) {',
        "    std::vector<tuinator::TableColumn> cols;",
        "    for (const auto& col : *columns) {",
        '        const std::string title = col.is_object() ? json::node_string(col, "title") : json::as_string(col);',
        '        const int width = col.is_object() ? json::node_int(col, 10, "width") : 10;',
        "        cols.push_back(tuinator::TableColumn{title, width});",
        "    }",
        "    widget->set_columns(std::move(cols));",
        "}",
        'if (const nlohmann::json* rows = json::find(node, "rows")) {',
        "    std::vector<std::vector<std::string>> table_rows;",
        "    for (const auto& row : *rows) {",
        "        std::vector<std::string> cells;",
        "        for (const auto& cell : row) { cells.push_back(json::as_string(cell)); }",
        "        table_rows.push_back(std::move(cells));",
        "    }",
        "    widget->set_rows(std::move(table_rows));",
        "}",
        'const std::string on_activate = json::node_string(node, "onActivate", "on_activate");',
        "if (!on_activate.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_activate]() {",
        "        raw->set_on_activate([&ctx, on_activate](int /*row*/, const std::vector<std::string>& /*cells*/) {",
        "            ctx.invoke_handler(on_activate);",
        "        });",
        "    });",
        "}",
        rt_register("Table"),
    ])

    case("TreeView", [
        f"const tuinator::Style item_style = {rt_style('itemStyle')};",
        f"const tuinator::Style selected_style = {rt_style('selectedStyle', 'accent')};",
        "auto widget = std::make_unique<tuinator::TreeView>(item_style, selected_style);",
        "tuinator::TreeView* raw = widget.get();",
        'if (const nlohmann::json* root = json::find(node, "root")) {',
        "    widget->set_root(build_tree_node(*root));",
        "}",
        'const std::string on_select = json::node_string(node, "onSelect", "on_select");',
        "if (!on_select.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_select]() {",
        "        raw->set_on_select([&ctx, on_select](const std::string& /*path*/) {",
        "            ctx.invoke_handler(on_select);",
        "        });",
        "    });",
        "}",
        rt_register("TreeView"),
    ])

    case("BarChart", [
        'const nlohmann::json& bars_json = node.contains("bars") ? node["bars"] : nlohmann::json::array();',
        "auto bars = build_bar_chart_bars(bars_json, ctx);",
        "tuinator::BarChartOptions opts{};",
        "fill_BarChartOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::BarChart>(std::move(bars), opts);",
        "tuinator::BarChart* raw = widget.get();",
        'const std::string on_change = json::node_string(node, "onChange", "on_change");',
        "if (!on_change.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_change]() {",
        "        raw->set_on_change([&ctx, on_change](const std::vector<tuinator::BarChartBar>& /*bars*/) {",
        "            ctx.invoke_handler(on_change);",
        "        });",
        "    });",
        "}",
        rt_register("BarChart"),
    ])

    case("LineChart", [
        'const nlohmann::json& series_json = node.contains("series") ? node["series"] : nlohmann::json::array();',
        "auto series = build_line_chart_series(series_json, ctx);",
        "tuinator::LineChartOptions opts{};",
        "fill_LineChartOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::LineChart>(std::move(series), opts);",
        "tuinator::LineChart* raw = widget.get();",
        rt_register("LineChart"),
    ])

    case("PieChart", [
        "std::vector<tuinator::PieChartSlice> slices;",
        'for (const auto& slice_node : json_array_or_empty(node, "slices")) {',
        '    const std::string label = json::node_string(slice_node, "label");',
        '    const double value = json::node_double(slice_node, 0.0, "value");',
        '    if (slice_node.contains("style")) {',
        "        slices.push_back(tuinator::PieChartSlice{",
        "            label, value, style::resolve_style(slice_node[\"style\"], ctx.theme)});",
        "    } else {",
        "        slices.push_back(tuinator::PieChartSlice{label, value});",
        "    }",
        "}",
        "tuinator::PieChartOptions opts{};",
        "fill_PieChartOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::PieChart>(std::move(slices), opts);",
        "tuinator::PieChart* raw = widget.get();",
        'const std::string on_change = json::node_string(node, "onChange", "on_change");',
        "if (!on_change.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_change]() {",
        "        raw->set_on_change([&ctx, on_change](const std::vector<tuinator::PieChartSlice>& /*slices*/) {",
        "            ctx.invoke_handler(on_change);",
        "        });",
        "    });",
        "}",
        rt_register("PieChart"),
    ])

    case("GaugeChart", [
        f"const double value = {rt_double('value')};",
        "tuinator::GaugeChartOptions opts{};",
        "fill_GaugeChartOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::GaugeChart>(value, opts);",
        "tuinator::GaugeChart* raw = widget.get();",
        rt_register("GaugeChart"),
    ])

    case("Histogram", [
        "std::vector<tuinator::HistogramBin> bins;",
        'for (const auto& bin_node : json_array_or_empty(node, "bins")) {',
        '    const std::string label = json::node_string(bin_node, "label");',
        '    const double count = json::node_double(bin_node, 0.0, "count");',
        '    if (bin_node.contains("style")) {',
        "        bins.push_back(tuinator::HistogramBin{",
        "            label, count, style::resolve_style(bin_node[\"style\"], ctx.theme)});",
        "    } else {",
        "        bins.push_back(tuinator::HistogramBin{label, count});",
        "    }",
        "}",
        "tuinator::HistogramOptions opts{};",
        "fill_HistogramOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::Histogram>(std::move(bins), opts);",
        "tuinator::Histogram* raw = widget.get();",
        rt_register("Histogram"),
    ])

    case("Heatmap", [
        "std::vector<std::vector<double>> values;",
        'for (const auto& row : json_array_or_empty(node, "values")) {',
        "    std::vector<double> row_values;",
        "    for (const auto& cell : row) { row_values.push_back(json::as_double(cell, 0.0)); }",
        "    values.push_back(std::move(row_values));",
        "}",
        "std::vector<std::string> row_labels;",
        'if (const nlohmann::json* rows = json::find(node, "rowLabels", "row_labels")) {',
        "    for (const auto& label : *rows) { row_labels.push_back(json::as_string(label)); }",
        "}",
        "std::vector<std::string> col_labels;",
        'if (const nlohmann::json* cols = json::find(node, "colLabels", "col_labels")) {',
        "    for (const auto& label : *cols) { col_labels.push_back(json::as_string(label)); }",
        "}",
        "tuinator::HeatmapOptions opts{};",
        "fill_HeatmapOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::Heatmap>(",
        "    std::move(values), std::move(row_labels), std::move(col_labels), opts);",
        "tuinator::Heatmap* raw = widget.get();",
        rt_register("Heatmap"),
    ])

    case("CandlestickChart", [
        "std::vector<tuinator::OhlcBar> bars;",
        'for (const auto& bar_node : json_array_or_empty(node, "bars")) {',
        "    if (!bar_node.is_object()) { continue; }",
        "    bars.push_back(tuinator::OhlcBar{",
        '        json::node_string(bar_node, "label"),',
        '        json::node_double(bar_node, 0.0, "open"),',
        '        json::node_double(bar_node, 0.0, "high"),',
        '        json::node_double(bar_node, 0.0, "low"),',
        '        json::node_double(bar_node, 0.0, "close"),',
        '        json::node_double(bar_node, 0.0, "volume")});',
        "}",
        "tuinator::CandlestickChartOptions opts{};",
        "fill_CandlestickChartOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::CandlestickChart>(std::move(bars), opts);",
        "tuinator::CandlestickChart* raw = widget.get();",
        rt_register("CandlestickChart"),
    ])

    case("StackedAreaChart", [
        'const nlohmann::json& series_json = node.contains("series") ? node["series"] : nlohmann::json::array();',
        "auto series = build_stacked_area_series(series_json, ctx);",
        "tuinator::StackedAreaChartOptions opts{};",
        "fill_StackedAreaChartOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::StackedAreaChart>(std::move(series), opts);",
        "tuinator::StackedAreaChart* raw = widget.get();",
        rt_register("StackedAreaChart"),
    ])

    case("WaterfallChart", [
        "std::vector<tuinator::WaterfallStep> steps;",
        'for (const auto& step_node : json_array_or_empty(node, "steps")) {',
        "    if (!step_node.is_object()) { continue; }",
        '    const std::string label = json::node_string(step_node, "label");',
        '    const double delta = json::node_double(step_node, 0.0, "delta");',
        '    const nlohmann::json* up = json::find(step_node, "upStyle", "up_style");',
        '    const nlohmann::json* down = json::find(step_node, "downStyle", "down_style");',
        "    if (up || down) {",
        "        steps.push_back(tuinator::WaterfallStep{",
        "            label, delta,",
        "            style::resolve_style(up ? *up : nlohmann::json{}, ctx.theme),",
        "            style::resolve_style(down ? *down : nlohmann::json{}, ctx.theme)});",
        "    } else {",
        "        steps.push_back(tuinator::WaterfallStep{label, delta});",
        "    }",
        "}",
        "tuinator::WaterfallChartOptions opts{};",
        "fill_WaterfallChartOptions(opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::WaterfallChart>(std::move(steps), opts);",
        "tuinator::WaterfallChart* raw = widget.get();",
        rt_register("WaterfallChart"),
    ])

    case("MenuBar", [
        f"const tuinator::Style style = {rt_style('style')};",
        f"const tuinator::Style active_style = {rt_style('activeStyle', 'accent')};",
        "auto widget = std::make_unique<tuinator::MenuBar>(style, active_style);",
        "tuinator::MenuBar* raw = widget.get();",
        'if (const nlohmann::json* menus = json::find(node, "menus")) {',
        "    std::vector<tuinator::Menu> menu_values;",
        "    for (const auto& menu : *menus) {",
        '        const std::string title = json::node_string(menu, "title");',
        "        std::vector<tuinator::MenuItem> items;",
        '        build_menu_items(json_array_or_empty(menu, "items"), items, ctx);',
        "        menu_values.push_back(tuinator::Menu{title, std::move(items)});",
        "    }",
        "    widget->set_menus(std::move(menu_values));",
        "}",
        'const std::string look = json::node_string(node, "look", "lookName", "look_name");',
        "if (!look.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, look]() { apply_menu_bar_look(*raw, look, ctx); });",
        "}",
        'const std::string on_action = json::node_string(node, "onAction", "on_action");',
        "if (!on_action.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_action]() {",
        "        raw->set_on_action([&ctx, on_action](const std::string& /*menu*/, const std::string& /*item*/) {",
        "            ctx.invoke_handler(on_action);",
        "        });",
        "    });",
        "}",
        rt_register("MenuBar"),
    ])

    case("ContextMenu", [
        "auto widget = std::make_unique<tuinator::ContextMenu>();",
        "tuinator::ContextMenu* raw = widget.get();",
        'if (const nlohmann::json* items = json::find(node, "items")) {',
        "    std::vector<tuinator::MenuItem> menu_items;",
        "    build_menu_items(*items, menu_items, ctx);",
        "    widget->set_items(std::move(menu_items));",
        "}",
        'const std::string look = json::node_string(node, "look", "lookName", "look_name");',
        "if (!look.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, look]() { apply_context_menu_look(*raw, look, ctx); });",
        "}",
        'const std::string on_action = json::node_string(node, "onAction", "on_action");',
        "if (!on_action.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_action]() {",
        "        raw->set_on_action([&ctx, on_action](const std::string& /*item*/) {",
        "            ctx.invoke_handler(on_action);",
        "        });",
        "    });",
        "}",
        rt_register("ContextMenu"),
    ])

    case("CommandPalette", [
        "auto widget = std::make_unique<tuinator::CommandPalette>();",
        "tuinator::CommandPalette* raw = widget.get();",
        'if (const nlohmann::json* entries = json::find(node, "entries")) {',
        "    std::vector<tuinator::CommandPaletteEntry> palette_entries;",
        "    for (const auto& entry : *entries) {",
        '        const std::string on_click = json::node_string(entry, "onClick", "on_click");',
        "        palette_entries.push_back(tuinator::CommandPaletteEntry{",
        '            json::node_string(entry, "id"),',
        '            json::node_string(entry, "label"),',
        '            json::node_string(entry, "category"),',
        '            json::node_string(entry, "shortcut"),',
        "            [&ctx, on_click]() { if (!on_click.empty()) { ctx.invoke_handler(on_click); } }});",
        "    }",
        "    widget->set_entries(std::move(palette_entries));",
        "}",
        'const std::string on_close = json::node_string(node, "onClose", "on_close");',
        "if (!on_close.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_close]() {",
        "        raw->set_on_close([&ctx, on_close]() { ctx.invoke_handler(on_close); });",
        "    });",
        "}",
        rt_register("CommandPalette"),
    ])

    case("StatusBar", [
        f"const std::string text = {rt_string('text')};",
        f"const tuinator::Style style = {rt_style('style', 'muted')};",
        "auto widget = std::make_unique<tuinator::StatusBar>(text, style);",
        "tuinator::StatusBar* raw = widget.get();",
        rt_register("StatusBar"),
    ])

    case("StatusLine", [
        "auto widget = std::make_unique<tuinator::StatusLine>();",
        "tuinator::StatusLine* raw = widget.get();",
        'for (const char* region : {"left", "center", "right"}) {',
        "    const nlohmann::json* segments = nullptr;",
        '    if (std::string(region) == "left") {',
        '        segments = json::find(node, "left", "statusLeft", "status_left");',
        '    } else if (std::string(region) == "center") {',
        '        segments = json::find(node, "center", "statusCenter", "status_center");',
        "    } else {",
        '        segments = json::find(node, "right", "statusRight", "status_right");',
        "    }",
        "    if (!segments || !segments->is_array()) { continue; }",
        "    auto built = build_status_segments(*segments, ctx);",
        "    ctx.schedule_post_init([raw, region = std::string(region), segs = std::move(built)]() mutable {",
        "        if (region == \"left\") { raw->set_left(std::move(segs)); }",
        "        else if (region == \"center\") { raw->set_center(std::move(segs)); }",
        "        else { raw->set_right(std::move(segs)); }",
        "    });",
        "}",
        rt_register("StatusLine"),
    ])

    case("ShellTerminal", [
        "tuinator::ShellTerminalStyle style_opts{};",
        "fill_ShellTerminalStyle(style_opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::ShellTerminal>(style_opts);",
        "tuinator::ShellTerminal* raw = widget.get();",
        'const std::string command = json::node_string(node, "command", "shell");',
        "if (!command.empty()) {",
        "    ctx.schedule_post_init([raw, command]() { raw->start(command); });",
        "}",
        'const std::string on_exit = json::node_string(node, "onExit", "on_exit");',
        "if (!on_exit.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_exit]() {",
        "        raw->set_on_exit([&ctx, on_exit]() { ctx.invoke_handler(on_exit); });",
        "    });",
        "}",
        rt_register("ShellTerminal"),
    ])

    source_control_lines = [
        "tuinator::SourceControlPanelStyle style_opts{};",
        "fill_SourceControlPanelStyle(style_opts, node, ctx);",
        "auto widget = std::make_unique<tuinator::SourceControlPanel>(style_opts);",
        "tuinator::SourceControlPanel* raw = widget.get();",
        'if (const nlohmann::json* header = json::find(node, "header")) {',
        "    ctx.schedule_post_init([raw, header = *header]() {",
        '        raw->set_header(json::node_string(header, "title"), json::node_string(header, "subtitle"));',
        "    });",
        "}",
        'if (const nlohmann::json* sections = json::find(node, "sections")) {',
        "    std::vector<tuinator::SourceControlSection> built_sections;",
        "    for (const auto& section : *sections) {",
        '        const std::string title = json::node_string(section, "title");',
        '        const bool expanded = json::node_bool(section, true, "expanded");',
        "        std::vector<tuinator::SourceControlEntry> entries;",
        '        for (const auto& entry : json_array_or_empty(section, "entries")) {',
        '            const std::string status = json::node_string(entry, "Modified", "status");',
        "            tuinator::GitChangeStatus status_enum = tuinator::GitChangeStatus::Modified;",
    ]
    source_control_lines.extend(
        rt_enum_switch("GitChangeStatus", "status", "status_enum", indent="            ")
    )
    source_control_lines.extend([
        "            entries.push_back(tuinator::SourceControlEntry{",
        '                json::node_string(entry, "path"), status_enum,',
        '                json::node_string(entry, "additions"), json::node_string(entry, "deletions"),',
        '                json::node_string(entry, "directoryTag", "directory_tag"),',
        '                json::node_bool(entry, false, "selected")});',
        "        }",
        "        built_sections.push_back(tuinator::SourceControlSection{title, std::move(entries), expanded});",
        "    }",
        "    ctx.schedule_post_init([raw, sections = std::move(built_sections)]() mutable {",
        "        raw->set_sections(std::move(sections));",
        "    });",
        "}",
        'const std::string on_select = json::node_string(node, "onSelect", "on_select");',
        "if (!on_select.empty()) {",
        "    ctx.schedule_post_init([raw, &ctx, on_select]() {",
        "        raw->set_on_select([&ctx, on_select](",
        "            int /*section_index*/, int /*entry_index*/, const tuinator::SourceControlEntry& /*entry*/) {",
        "            ctx.invoke_handler(on_select);",
        "        });",
        "    });",
        "}",
        rt_register("SourceControlPanel"),
    ])
    case("SourceControlPanel", source_control_lines)

    case("Desktop", [
        "auto widget = std::make_unique<tuinator::Desktop>();",
        "tuinator::Desktop* raw = widget.get();",
        'if (node.contains("background")) {',
        "    auto background = build_widget(node[\"background\"], ctx);",
        "    widget->set_background(std::move(background));",
        "}",
        'create_windows(*raw, "create_window", json_array_or_empty(node, "windows"), ctx, true);',
        'if (const nlohmann::json* context_menu = json::find(node, "contextMenu", "context_menu")) {',
        "    std::vector<tuinator::MenuItem> items;",
        '    build_menu_items(json_array_or_empty(*context_menu, "items"), items, ctx);',
        "    ctx.schedule_post_init([raw, items = std::move(items)]() mutable {",
        "        raw->set_context_menu_items(std::move(items));",
        "    });",
        "}",
        'if (ctx.actions) {',
        "    ctx.schedule_post_init([raw, actions = ctx.actions]() { raw->set_action_registry(actions); });",
        "}",
        'if (const nlohmann::json* palette = json::find(node, "commandPalette", "command_palette")) {',
        '    if (json::node_bool(*palette, false, "fromActions", "from_actions") && ctx.actions) {',
        "        ctx.schedule_post_init([raw, actions = ctx.actions]() {",
        "            raw->set_action_registry(actions);",
        "            std::vector<tuinator::CommandPaletteEntry> entries;",
        "            for (const tuinator::RegisteredAction* action : actions->all()) {",
        "                entries.push_back(tuinator::CommandPaletteEntry{",
        "                    action->id, action->label, action->category, action->shortcut, action->callback});",
        "            }",
        "            raw->set_command_palette_entries(std::move(entries));",
        "        });",
        '    } else if (palette->contains("entries")) {',
        "        std::vector<tuinator::CommandPaletteEntry> entries;",
        '        for (const auto& entry : (*palette)["entries"]) {',
        '            const std::string on_click = json::node_string(entry, "onClick", "on_click");',
        "            entries.push_back(tuinator::CommandPaletteEntry{",
        '                json::node_string(entry, "id"), json::node_string(entry, "label"),',
        '                json::node_string(entry, "category"), json::node_string(entry, "shortcut"),',
        "                [&ctx, on_click]() { if (!on_click.empty()) { ctx.invoke_handler(on_click); } }});",
        "        }",
        "        ctx.schedule_post_init([raw, entries = std::move(entries)]() mutable {",
        "            raw->set_command_palette_entries(std::move(entries));",
        "        });",
        "    }",
        "}",
        rt_register("Desktop"),
    ])

    case("WindowHost", [
        "auto widget = std::make_unique<tuinator::WindowHost>();",
        "tuinator::WindowHost* raw = widget.get();",
        'create_windows(*widget, "create_window", json_array_or_empty(node, "windows"), ctx, false);',
        rt_register("WindowHost"),
    ])

    case("MessageDialog", [
        f"const std::string message = {rt_string('message', 'text')};",
        'const std::string on_ok = json::node_string(node, "onOk", "on_ok", "onClick", "on_click");',
        "auto widget = tuinator::dialog::make_message(ctx.theme, message, [&ctx, on_ok]() {",
        "    if (!on_ok.empty()) { ctx.invoke_handler(on_ok); }",
        "});",
        "tuinator::Widget* raw = widget.get();",
        rt_register("Widget"),
    ])

    case("ConfirmDialog", [
        f"const std::string message = {rt_string('message', 'text')};",
        'const std::string on_result = json::node_string(node, "onResult", "on_result", "onConfirm", "on_confirm");',
        "auto widget = tuinator::dialog::make_confirm(ctx.theme, message, [&ctx, on_result](bool /*confirmed*/) {",
        "    if (!on_result.empty()) { ctx.invoke_handler(on_result); }",
        "});",
        "tuinator::Widget* raw = widget.get();",
        rt_register("Widget"),
    ])

    return cases


def generate_header() -> str:
    return """#pragma once

#include <memory>

#include <nlohmann/json_fwd.hpp>

#include <tuinator/scene/scene_context.hpp>

namespace tuinator {
class Widget;
}

namespace tuinator::scene::detail {

std::unique_ptr<Widget> build_widget(const nlohmann::json& node, SceneContext& ctx);

} // namespace tuinator::scene::detail
"""


def generate_source() -> str:
    helper_lines = gen_runtime_helpers()
    option_lines = gen_options_fill_functions()
    case_lines = gen_widget_cases()

    body = "\n".join(
        [
            "// Generated by scripts/generate_scene_runtime_cpp.py. Do not edit by hand.",
            "",
            '#include "tuinator/scene/detail/widget_builder.generated.hpp"',
            "",
            "#include <tuinator/tuinator.hpp>",
            "#include <tuinator/scene/scene_context.hpp>",
            "#include <tuinator/scene/detail/json_node.hpp>",
            "#include <tuinator/scene/detail/style_resolve.hpp>",
            "",
            "#include <nlohmann/json.hpp>",
            "",
            "#include <memory>",
            "#include <stdexcept>",
            "#include <string>",
            "#include <utility>",
            "#include <vector>",
            "",
            "namespace tuinator::scene::detail {",
            "",
            "namespace {",
            "",
            "#if defined(__GNUC__) || defined(__clang__)",
            "#pragma GCC diagnostic push",
            "#pragma GCC diagnostic ignored \"-Wunused-parameter\"",
            "#pragma GCC diagnostic ignored \"-Wunused-function\"",
            "#endif",
            "",
            "#if defined(_MSC_VER)",
            "#pragma warning(push)",
            "#pragma warning(disable : 4100)",
            "#pragma warning(disable : 4505)",
            "#endif",
            "",
            *option_lines,
            "",
            *helper_lines,
            "",
            "#if defined(_MSC_VER)",
            "#pragma warning(pop)",
            "#endif",
            "",
            "#if defined(__GNUC__) || defined(__clang__)",
            "#pragma GCC diagnostic pop",
            "#endif",
            "",
            "} // namespace",
            "",
            "std::unique_ptr<Widget> build_widget(const nlohmann::json& node, SceneContext& ctx) {",
            '    const std::string type = json::node_string(node, "type");',
            "",
            *case_lines,
            "",
            '    throw std::runtime_error("Unsupported widget type: " + type);',
            "}",
            "",
            "} // namespace tuinator::scene::detail",
            "",
        ]
    )
    return body


def write_outputs() -> tuple[Path, Path]:
    HEADER_PATH.parent.mkdir(parents=True, exist_ok=True)
    SOURCE_PATH.parent.mkdir(parents=True, exist_ok=True)
    HEADER_PATH.write_text(generate_header(), encoding="utf-8")
    SOURCE_PATH.write_text(generate_source(), encoding="utf-8")
    return HEADER_PATH, SOURCE_PATH


def main() -> int:
    header_path, source_path = write_outputs()
    header_lines = len(header_path.read_text(encoding="utf-8").splitlines())
    source_lines = len(source_path.read_text(encoding="utf-8").splitlines())
    print(f"Wrote {header_path} ({header_lines} lines)")
    print(f"Wrote {source_path} ({source_lines} lines)")
    print(f"Widget types: {len(SUPPORTED_WIDGETS)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
