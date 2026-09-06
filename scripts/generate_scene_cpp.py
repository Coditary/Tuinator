#!/usr/bin/env python3
"""Generate Tuinator C++ UI code from a declarative JSON scene file."""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Callable

from scene_bindings import (
    binding_helper_header,
    generate_bindings as emit_scene_bindings,
    parse_bindings,
    wrap_constructor_callback,
)
from scene_options import (
    assign_nested_style_fields,
    assign_options_fields,
    assign_scrollbar_options,
    build_options_var,
    emit_border_glyphs,
    emit_gradient_stops,
    emit_rect,
    lookup_field_value,
    merge_options_source,
)

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent))

THEME_STYLES = frozenset({
    "heading", "label", "muted", "button", "button_focused",
    "text_input", "text_input_focused", "border", "accent", "success", "danger",
})

COLORS = frozenset({
    "Default", "Black", "Red", "Green", "Yellow",
    "Blue", "Magenta", "Cyan", "White",
})

ADD_CHILD_CONTAINERS = frozenset({"VBox", "HBox", "Grid"})
SET_CONTENT_CONTAINERS = frozenset({"Panel", "TerminalFrame"})
COMPOSITE_CONTAINERS = frozenset({
    "VBox", "HBox", "Grid", "Panel", "ScrollView", "SplitPane", "Tabs", "TerminalFrame",
    "Desktop", "WindowHost",
})

CPP_ENUMS: dict[str, dict[str, str]] = {
    "SplitOrientation": {"Horizontal": "Horizontal", "Vertical": "Vertical"},
    "ChartGlyphStyle": {
        "Dots": "Dots", "FineDots": "FineDots", "SmallDots": "SmallDots",
        "Stars": "Stars", "Hash": "Hash", "Plus": "Plus", "Blocks": "Blocks",
        "Braille": "Braille", "Custom": "Custom",
    },
    "BarChartOrientation": {"Vertical": "Vertical", "Horizontal": "Horizontal"},
    "LineChartMode": {
        "Line": "Line", "Area": "Area", "Sparkline": "Sparkline",
        "Scatter": "Scatter", "Step": "Step",
    },
    "PieChartStyle": {
        "Dots": "Dots", "FineDots": "FineDots", "SmallDots": "SmallDots",
        "Stars": "Stars", "Hash": "Hash", "Plus": "Plus", "Blocks": "Blocks",
        "Braille": "Braille", "Custom": "Custom",
    },
    "GaugeStyle": {"Arc": "Arc", "Semicircle": "Semicircle", "Horizontal": "Horizontal"},
    "ProgressBarLayout": {
        "Plain": "Plain", "Bracketed": "Bracketed", "Blocks": "Blocks", "Dots": "Dots",
        "Pill": "Pill", "Labeled": "Labeled", "FilledLabel": "FilledLabel", "Tqdm": "Tqdm",
        "BrailleMetric": "BrailleMetric", "BrailleWave": "BrailleWave", "Pulse": "Pulse",
        "Shimmer": "Shimmer", "Bounce": "Bounce", "SlideBlock": "SlideBlock",
        "MovingDot": "MovingDot", "TaskRow": "TaskRow",
    },
    "BigTextKind": {
        "Block": "Block", "Half": "Half", "Narrow": "Narrow", "Quadrant": "Quadrant",
        "Third": "Third", "Sextant": "Sextant", "Quarter": "Quarter", "Octant": "Octant",
        "Braille": "Braille", "Shade": "Shade", "Letter": "Letter", "Banner": "Banner",
        "Outline": "Outline", "Stacked": "Stacked", "Isometric": "Isometric", "Slant": "Slant",
        "Standard": "Standard", "Small": "Small", "Big": "Big", "Doom": "Doom",
        "FigletShadow": "FigletShadow",
    },
    "MenuItemKind": {
        "Action": "Action", "Separator": "Separator", "Checkbox": "Checkbox", "Submenu": "Submenu",
    },
    "StatusSegmentKind": {
        "Text": "Text", "Pill": "Pill", "Box": "Box", "Separator": "Separator",
    },
    "GitChangeStatus": {
        "Modified": "Modified", "Added": "Added", "Deleted": "Deleted", "Renamed": "Renamed",
        "Copied": "Copied", "Untracked": "Untracked", "Ignored": "Ignored",
        "Unmerged": "Unmerged", "Unknown": "Unknown",
    },
    "BorderStyle": {
        "Ascii": "Ascii", "Light": "Light", "Heavy": "Heavy", "Double": "Double", "Rounded": "Rounded",
    },
    "ProgressBarPercentPosition": {
        "None": "None", "Right": "Right", "Inside": "Inside",
    },
    "ScrollbarPreset": {
        "Classic": "Classic", "Ascii": "Ascii", "Minimal": "Minimal",
        "Bold": "Bold", "Thin": "Thin",
    },
    "BigTextGradientAxis": {
        "Vertical": "Vertical", "Horizontal": "Horizontal",
    },
    "GlyphSet": {
        "Auto": "Auto", "Ascii": "Ascii", "Unicode": "Unicode",
        "UnicodeRounded": "UnicodeRounded",
    },
}

SUPPORTED_WIDGETS = [
    "VBox", "HBox", "Grid", "Panel", "ScrollView", "SplitPane", "Tabs", "TerminalFrame",
    "Label", "Button", "TextInput", "TextArea", "Separator", "Checkbox", "Toggle",
    "Slider", "Spinner", "ComboBox", "ProgressBar", "Throbber", "BigText", "ImageView",
    "NerdIconCatalogGallery", "NerdGlyphMatrixGallery",
    "ListView", "Table", "TreeView",
    "BarChart", "LineChart", "PieChart", "GaugeChart", "Histogram", "Heatmap",
    "CandlestickChart", "StackedAreaChart", "WaterfallChart",
    "MenuBar", "ContextMenu", "CommandPalette",
    "StatusBar", "StatusLine", "ShellTerminal", "SourceControlPanel",
    "Desktop", "WindowHost", "MessageDialog", "ConfirmDialog",
]


@dataclass
class NodeContext:
    var_name: str
    ptr_name: str
    cpp_type: str


@dataclass
class GenerationState:
    scene_path: Path
    handlers_header: str | None
    namespace: str
    nodes: list[str] = field(default_factory=list)
    bindings: list[str] = field(default_factory=list)
    post_init: list[str] = field(default_factory=list)
    preamble: list[str] = field(default_factory=list)
    build_result_fields: list[str] = field(default_factory=list)
    handler_refs: set[str] = field(default_factory=set)
    extra_includes: set[str] = field(default_factory=set)
    counter: int = 0
    ids: dict[str, NodeContext] = field(default_factory=dict)
    has_actions: bool = False
    binding_specs: list[Any] = field(default_factory=list)

    def next_var(self, prefix: str) -> str:
        self.counter += 1
        safe = re.sub(r"[^a-zA-Z0-9_]", "_", prefix)
        if safe and safe[0].isdigit():
            safe = f"n_{safe}"
        return f"{safe}_{self.counter}"

    def add_include(self, header: str) -> None:
        self.extra_includes.add(header)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scene", type=Path, help="Input .scene.json file")
    parser.add_argument("--handlers", type=Path, default=None)
    parser.add_argument("--namespace", default=None)
    parser.add_argument("--header", type=Path, default=None)
    parser.add_argument("--source", type=Path, default=None)
    return parser.parse_args(argv)


def load_scene(path: Path) -> dict[str, Any]:
    data = json.loads(path.read_text(encoding="utf-8"))
    try:
        from validate_scene_json import load_and_validate

        return load_and_validate(path)
    except ImportError:
        if not isinstance(data, dict):
            raise ValueError("Scene root must be a JSON object")
        if data.get("version", 1) != 1:
            raise ValueError("Only scene version 1 is supported")
        if "root" not in data:
            raise ValueError("Scene must contain a 'root' object")
        return data


def scene_namespace(scene_path: Path, override: str | None) -> str:
    if override:
        return override
    stem = scene_path.name
    if stem.endswith(".scene.json"):
        stem = stem[: -len(".scene.json")]
    else:
        stem = scene_path.stem
    safe = re.sub(r"[^a-zA-Z0-9_]", "_", stem) or "scene"
    return f"tuinator::scene::{safe}"


def cpp_string(value: str) -> str:
    return json.dumps(value, ensure_ascii=True)


def cpp_bool(value: Any) -> str:
    return "true" if value else "false"


def emit_enum(enum_type: str, value: str) -> str:
    mapping = CPP_ENUMS.get(enum_type)
    if mapping is None:
        raise ValueError(f"Unknown enum type: {enum_type}")
    if value not in mapping:
        allowed = ", ".join(sorted(mapping))
        raise ValueError(f"Invalid {enum_type} value '{value}'. Allowed: {allowed}")
    return f"tuinator::{enum_type}::{mapping[value]}"


def emit_style(style: Any, theme_var: str = "theme") -> str:
    if isinstance(style, str):
        if style in THEME_STYLES:
            return f"{theme_var}.{style}"
        if style in COLORS:
            return f"tuinator::Style{{.foreground = tuinator::Color::{style}}}"
        raise ValueError(f"Unknown style preset: {style}")
    if not isinstance(style, dict):
        raise ValueError(f"Style must be a string preset or object, got {type(style)}")
    parts: list[str] = []
    if "foreground" in style:
        fg = style["foreground"]
        if isinstance(fg, str) and fg in COLORS:
            parts.append(f".foreground = tuinator::Color::{fg}")
        elif isinstance(fg, dict):
            parts.append(
                f".foreground_rgb = tuinator::Rgb{{{int(fg['r'])}, {int(fg['g'])}, {int(fg['b'])}}}"
            )
        else:
            raise ValueError(f"Invalid foreground style: {fg}")
    if "background" in style:
        bg = style["background"]
        if isinstance(bg, str) and bg in COLORS:
            parts.append(f".background = tuinator::Color::{bg}")
        elif isinstance(bg, dict):
            parts.append(
                f".background_rgb = tuinator::Rgb{{{int(bg['r'])}, {int(bg['g'])}, {int(bg['b'])}}}"
            )
        else:
            raise ValueError(f"Invalid background style: {bg}")
    for flag in ("bold", "dim", "reverse"):
        if style.get(flag):
            parts.append(f".{flag} = true")
    return "tuinator::Style{" + ", ".join(parts) + "}" if parts else "tuinator::Style{}"


def opts_builder(
    state: GenerationState,
    prefix: str,
    struct_name: str,
    node: dict[str, Any],
    theme_var: str,
    init_expr: str | None = None,
) -> str:
    return build_options_var(
        state.nodes, prefix, struct_name, node, theme_var, emit_style, emit_enum, init_expr
    )


def emit_menu_bar_look(var: str, look: str, theme_var: str) -> str:
    preset = look.lower()
    if preset == "classic":
        return f"{var}->apply_look(tuinator::menu_bar_look_classic({theme_var}));"
    if preset == "mac":
        return f"{var}->apply_look(tuinator::menu_bar_look_mac({theme_var}));"
    if preset == "minimal":
        return f"{var}->apply_look(tuinator::menu_bar_look_minimal({theme_var}));"
    return (
        f"if (const tuinator::MenuBarLook* named = tuinator::menu_bar_look_named({cpp_string(look)})) {{"
        f"\n        {var}->apply_look(*named);"
        f"\n    }}"
    )


def emit_style_vector(
    state_nodes: list[str],
    var: str,
    styles: list[Any],
    theme_var: str,
) -> None:
    state_nodes.append(f"    std::vector<tuinator::Style> {var};")
    for style in styles:
        state_nodes.append(f"    {var}.push_back({emit_style(style, theme_var)});")


def generate_theme_setup(scene: dict[str, Any], state: GenerationState) -> str:
    theme_block = scene.get("theme")
    if not isinstance(theme_block, dict):
        return "theme"
    state.nodes.append("    tuinator::ThemeOptions theme_options{};")
    glyphs = lookup_field_value(theme_block, ("glyphs", "glyphSet", "glyph_set"))
    if glyphs is not None:
        state.nodes.append(
            f"    theme_options.glyphs = {emit_enum('GlyphSet', str(glyphs))};"
        )
    border_style = lookup_field_value(theme_block, ("borderStyle", "border_style"))
    if border_style is not None:
        state.nodes.append(
            f"    theme_options.border_style = {emit_enum('BorderStyle', str(border_style))};"
        )
    preset = str(lookup_field_value(theme_block, ("preset", "name")) or "dark").lower()
    factory = "tuinator::light_theme" if preset == "light" else "tuinator::dark_theme"
    state.nodes.append(f"    tuinator::Theme scene_theme = {factory}(theme_options);")
    return "scene_theme"


def handler_call(name: str, state: GenerationState, app_var: str, extra_args: str = "") -> str:
    state.handler_refs.add(name)
    prefix = "scene_handlers::" if state.handlers_header else ""
    comma = ", " if extra_args else ""
    return f"{prefix}{name}({app_var}{comma}{extra_args})"


def node_property(node: dict[str, Any], *keys: str, default: Any = None) -> Any:
    props = node.get("properties")
    if isinstance(props, dict):
        for key in keys:
            if key in props:
                return props[key]
    for key in keys:
        if key in node:
            return node[key]
    return default


def ptr_name_for_id(node_id: str) -> str:
    safe = re.sub(r"[^a-zA-Z0-9_]", "_", node_id) or "node"
    return f"scene_{safe}_ptr"


def prescan_scene_ids(node: dict[str, Any], state: GenerationState) -> None:
    node_type = node.get("type")
    if not isinstance(node_type, str):
        raise ValueError("Each widget node requires a 'type' string")
    node_id = node.get("id")
    if node_id:
        if node_id in state.ids:
            raise ValueError(f"Duplicate node id: {node_id}")
        state.ids[node_id] = NodeContext(
            var_name="",
            ptr_name=ptr_name_for_id(str(node_id)),
            cpp_type=node_type,
        )
    for child in node.get("children", []):
        prescan_scene_ids(child, state)
    content = node.get("content")
    if content is not None:
        prescan_scene_ids(content, state)
    first = node.get("first")
    second = node.get("second")
    if first is not None:
        prescan_scene_ids(first, state)
    if second is not None:
        prescan_scene_ids(second, state)
    for tab in node.get("tabs", []):
        if isinstance(tab, dict) and tab.get("content") is not None:
            prescan_scene_ids(tab["content"], state)
    for window in node.get("windows", []):
        if isinstance(window, dict) and window.get("content") is not None:
            prescan_scene_ids(window["content"], state)
    background = node.get("background")
    if isinstance(background, dict):
        prescan_scene_ids(background, state)
    root = node.get("root")
    if isinstance(root, dict) and "type" in root:
        prescan_scene_ids(root, state)


def register_id(node: dict[str, Any], var: str, cpp_type: str, state: GenerationState) -> None:
    node_id = node.get("id")
    if not node_id:
        return
    if node_id in state.ids:
        state.ids[node_id].var_name = var
        state.ids[node_id].cpp_type = cpp_type
        return
    state.ids[node_id] = NodeContext(var_name=var, ptr_name=ptr_name_for_id(str(node_id)), cpp_type=cpp_type)


def capture_id_ptr(child_node: dict[str, Any], child_var: str, state: GenerationState) -> None:
    node_id = child_node.get("id")
    if not node_id:
        return
    ctx = state.ids[node_id]
    state.nodes.append(f"    tuinator::{ctx.cpp_type}* {ctx.ptr_name} = {child_var}.get();")


def attach_child(parent_var: str, child_node: dict[str, Any], child_var: str, state: GenerationState) -> None:
    capture_id_ptr(child_node, child_var, state)
    state.nodes.append(f"    {parent_var}->add_child(std::move({child_var}));")


def attach_content(parent_var: str, child_node: dict[str, Any], child_var: str, state: GenerationState) -> None:
    capture_id_ptr(child_node, child_var, state)
    state.nodes.append(f"    {parent_var}->set_content(std::move({child_var}));")


def emit_string_vector(values: list[Any], name: str, state: GenerationState) -> str:
    if not values:
        return f"std::vector<std::string>{{}}"
    lines = [f"    std::vector<std::string> {name};"]
    for value in values:
        lines.append(f"    {name}.push_back({cpp_string(str(value))});")
    state.nodes.extend(lines)
    return name


def emit_tree_node(node: dict[str, Any], var: str, state: GenerationState) -> None:
    label = node_property(node, "label", "text", default="")
    expanded = cpp_bool(node_property(node, "expanded", default=True))
    state.nodes.append(
        f"    tuinator::TreeNode {var}{{"
        f"{cpp_string(label)}, {{}}, {expanded}}};"
    )
    for index, child in enumerate(node.get("children", [])):
        child_var = f"{var}_child_{index}"
        emit_tree_node(child, child_var, state)
        state.nodes.append(f"    {var}.children.push_back(std::move({child_var}));")


def emit_menu_items(items: list[Any], var: str, state: GenerationState, app_var: str) -> None:
    state.nodes.append(f"    std::vector<tuinator::MenuItem> {var};")
    for item in items:
        if not isinstance(item, dict):
            raise ValueError("Menu items must be objects")
        kind = item.get("kind", "Action")
        if kind == "Separator":
            state.nodes.append(f"    {var}.push_back(tuinator::MenuItem::separator());")
            continue
        label = item.get("label", "")
        shortcut = item.get("shortcut", "")
        enabled = cpp_bool(item.get("enabled", True))
        checked = cpp_bool(item.get("checked", False))
        on_click = item.get("onClick", item.get("on_click"))
        if kind == "Checkbox":
            action = "{}"
            if on_click:
                action = f"[&{app_var}]() {{ {handler_call(on_click, state, app_var)}; }}"
            state.nodes.append(
                f"    {var}.push_back(tuinator::MenuItem::checkbox("
                f"{cpp_string(label)}, {checked}, {action}));"
            )
            continue
        if kind == "Submenu":
            sub_var = state.next_var("menu_items")
            children = item.get("children", item.get("items", []))
            emit_menu_items(children, sub_var, state, app_var)
            state.nodes.append(
                f"    {var}.push_back(tuinator::MenuItem::submenu("
                f"{cpp_string(label)}, std::move({sub_var}), {cpp_string(shortcut)}, "
                f"{cpp_string(item.get('icon', ''))}));"
            )
            continue
        action = "{}"
        if on_click:
            action = f"[&{app_var}]() {{ {handler_call(on_click, state, app_var)}; }}"
        state.nodes.append(
            f"    {var}.push_back(tuinator::MenuItem({cpp_string(label)}, {action}, "
            f"{cpp_string(shortcut)}, {enabled}));"
        )


def emit_status_segments(segments: list[Any], var: str, state: GenerationState) -> None:
    state.nodes.append(f"    std::vector<tuinator::StatusSegment> {var};")
    for segment in segments:
        if not isinstance(segment, dict):
            raise ValueError("Status segments must be objects")
        kind = segment.get("kind", "Text")
        text = segment.get("text", "")
        bold = cpp_bool(segment.get("bold", False))
        dim = cpp_bool(segment.get("dim", False))
        state.nodes.append("    {")
        state.nodes.append("        tuinator::StatusSegment seg;")
        state.nodes.append(f"        seg.kind = {emit_enum('StatusSegmentKind', kind)};")
        state.nodes.append(f"        seg.text = {cpp_string(text)};")
        state.nodes.append(f"        seg.bold = {bold};")
        state.nodes.append(f"        seg.dim = {dim};")
        if "foreground" in segment and isinstance(segment["foreground"], dict):
            fg = segment["foreground"]
            state.nodes.append(
                f"        seg.foreground_rgb = tuinator::Rgb{{{int(fg['r'])}, {int(fg['g'])}, {int(fg['b'])}}};"
            )
        state.nodes.append(f"        {var}.push_back(seg);")
        state.nodes.append("    }")


def schedule_ptr_callback(
    node: dict[str, Any],
    var: str,
    cpp_type: str,
    state: GenerationState,
    line: str,
) -> None:
    node_id = node.get("id")
    if node_id:
        ctx = state.ids[node_id]
        state.post_init.append(f"    {ctx.ptr_name}->{line}")
    else:
        state.post_init.append(f"    {var}->{line}")


def generate_vbox_hbox(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str, cpp_type: str) -> str:
    var = state.next_var(cpp_type)
    gap = node_property(node, "gap", default=1)
    padding = node_property(node, "padding", default=0)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::{cpp_type}>("
        f"tuinator::BoxOptions{{.gap = {gap}, .padding = {padding}}});"
    )
    flex = node_property(node, "flex")
    if flex is not None:
        state.nodes.append(f"    {var}->set_flex({int(flex)});")
    register_id(node, var, cpp_type, state)
    for child in node.get("children", []):
        attach_child(var, child, generate_node(child, state, theme_var, app_var), state)
    return var


def generate_grid(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Grid")
    columns = node_property(node, "columns", default=2)
    gap = node_property(node, "gap", default=1)
    padding = node_property(node, "padding", default=0)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Grid>("
        f"tuinator::GridOptions{{.columns = {columns}, .gap = {gap}, .padding = {padding}}});"
    )
    register_id(node, var, "Grid", state)
    for child in node.get("children", []):
        attach_child(var, child, generate_node(child, state, theme_var, app_var), state)
    return var


def generate_panel(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Panel")
    title = node_property(node, "title", default="")
    border_style = emit_style(node_property(node, "borderStyle", "border", default="border"), theme_var)
    title_style = emit_style(node_property(node, "titleStyle", default="heading"), theme_var)
    border_glyphs = node_property(node, "borderGlyphs", "border_glyphs", "glyphs")
    glyphs_arg = ""
    if isinstance(border_glyphs, dict):
        glyphs_var = f"{var}_glyphs"
        state.nodes.append(
            f"    tuinator::BorderGlyphs {glyphs_var} = "
            f"{emit_border_glyphs(border_glyphs, theme_var, emit_style, emit_enum)};"
        )
        glyphs_arg = f", {glyphs_var}"
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Panel>("
        f"{cpp_string(title)}, {border_style}, {title_style}{glyphs_arg});"
    )
    register_id(node, var, "Panel", state)
    content = node.get("content")
    if content is None and node.get("children"):
        raise ValueError("Panel supports a single 'content' node, not 'children'")
    if content is not None:
        child_var = generate_node(content, state, theme_var, app_var)
        attach_content(var, content, child_var, state)
    return var


def generate_scroll_view(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("ScrollView")
    content = node.get("content")
    if content is None:
        raise ValueError("ScrollView requires a 'content' node")
    child_var = generate_node(content, state, theme_var, app_var)
    capture_id_ptr(content, child_var, state)
    opts_var = opts_builder(state, var, "ScrollViewOptions", node, theme_var)
    scrollbars = node_property(node, "scrollbars", "scrollbar")
    if isinstance(scrollbars, dict):
        assign_scrollbar_options(
            state.nodes, f"{opts_var}.scrollbars", scrollbars, theme_var, emit_style, emit_enum
        )
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::ScrollView>(std::move({child_var}), {opts_var});"
    )
    register_id(node, var, "ScrollView", state)
    return var


def generate_split_pane(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    first = node.get("first")
    second = node.get("second")
    if first is None or second is None:
        raise ValueError("SplitPane requires 'first' and 'second' nodes")
    first_var = generate_node(first, state, theme_var, app_var)
    second_var = generate_node(second, state, theme_var, app_var)
    var = state.next_var("SplitPane")
    opts_var = opts_builder(state, var, "SplitPaneOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::SplitPane>("
        f"std::move({first_var}), std::move({second_var}), {opts_var});"
    )
    register_id(node, var, "SplitPane", state)
    return var


def generate_tabs(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Tabs")
    opts_var = opts_builder(state, var, "TabsOptions", node, theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::Tabs>({opts_var});")
    register_id(node, var, "Tabs", state)
    for tab in node.get("tabs", []):
        if not isinstance(tab, dict):
            raise ValueError("Each tab must be an object")
        title = tab.get("title", "Tab")
        content = tab.get("content")
        if content is None:
            raise ValueError("Each tab requires a 'content' node")
        child_var = generate_node(content, state, theme_var, app_var)
        capture_id_ptr(content, child_var, state)
        state.nodes.append(
            f"    {var}->add_tab({cpp_string(title)}, std::move({child_var}));"
        )
    selected = node_property(node, "selectedIndex", "selected_index")
    if selected is not None:
        state.post_init.append(f"    {var}->set_selected_index({int(selected)});")
    return var


def generate_terminal_frame(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    title = node_property(node, "title", default="Terminal")
    content = node.get("content")
    if content is None:
        raise ValueError("TerminalFrame requires a 'content' node")
    child_var = generate_node(content, state, theme_var, app_var)
    capture_id_ptr(content, child_var, state)
    var = state.next_var("TerminalFrame")
    opts_var = opts_builder(state, var, "TerminalFrameOptions", node, theme_var)
    frame_style = node_property(node, "frameStyle", "frame_style", "style")
    if isinstance(frame_style, dict):
        assign_nested_style_fields(
            state.nodes, f"{opts_var}.style", frame_style, "TerminalFrameStyle", theme_var, emit_style, emit_enum
        )
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::TerminalFrame>("
        f"{cpp_string(title)}, std::move({child_var}), {opts_var});"
    )
    register_id(node, var, "TerminalFrame", state)
    for region in ("left", "center", "right"):
        segments = node_property(node, f"status{region.capitalize()}", f"status_{region}")
        if segments:
            seg_var = state.next_var("status_segments")
            emit_status_segments(segments, seg_var, state)
            state.post_init.append(f"    {var}->set_status_{region}(std::move({seg_var}));")
    return var


def generate_label(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("Label")
    text = node_property(node, "text", default="")
    style = emit_style(node_property(node, "style", default={}), theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::Label>({cpp_string(text)}, {style});")
    register_id(node, var, "Label", state)
    return var


def generate_button(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Button")
    label = node_property(node, "label", "text", default="Button")
    style = emit_style(node_property(node, "style", default="button"), theme_var)
    on_click = node_property(node, "onClick", "on_click")
    callback = f"[&{app_var}]() {{ {handler_call(on_click, state, app_var)}; }}" if on_click else "{}"
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Button>({cpp_string(label)}, {callback}, {style});"
    )
    register_id(node, var, "Button", state)
    return var


def generate_text_input(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("TextInput")
    opts_var = opts_builder(state, var, "TextInputOptions", node, theme_var)
    style = emit_style(node_property(node, "style", default="text_input"), theme_var)
    focused_style = emit_style(node_property(node, "focusedStyle", default="text_input_focused"), theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::TextInput>({opts_var}, {style}, {focused_style});"
    )
    value = node_property(node, "value")
    if value is not None:
        state.nodes.append(f"    {var}->set_value({cpp_string(str(value))});")
    register_id(node, var, "TextInput", state)
    return var


def generate_text_area(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("TextArea")
    opts_var = opts_builder(state, var, "TextAreaOptions", node, theme_var)
    scrollbars = node_property(node, "scrollbars", "scrollbar")
    if isinstance(scrollbars, dict):
        assign_scrollbar_options(
            state.nodes, f"{opts_var}.scrollbars", scrollbars, theme_var, emit_style, emit_enum
        )
    style = emit_style(node_property(node, "style", default="text_input"), theme_var)
    focused_style = emit_style(node_property(node, "focusedStyle", default="text_input_focused"), theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::TextArea>({opts_var}, {style}, {focused_style});"
    )
    value = node_property(node, "value")
    if value is not None:
        state.nodes.append(f"    {var}->set_value({cpp_string(str(value))});")
    on_change = node_property(node, "onChange", "on_change")
    if on_change:
        schedule_ptr_callback(
            node, var, "TextArea", state,
            f"set_on_change([&{app_var}](const std::string& value) {{ "
            f"{handler_call(on_change, state, app_var, 'value')}; }});",
        )
    register_id(node, var, "TextArea", state)
    return var


def generate_separator(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("Separator")
    style = emit_style(node_property(node, "style", default="border"), theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::Separator>({style});")
    register_id(node, var, "Separator", state)
    return var


def generate_checkbox(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Checkbox")
    label = node_property(node, "label", "text", default="")
    checked = cpp_bool(node_property(node, "checked", default=False))
    style_name = node_property(node, "styleName", "style_name", "checkboxStyle", "checkbox_style")
    if style_name:
        opts_var = f"{var}_options"
        state.nodes.append(f"    tuinator::CheckboxOptions {opts_var} = tuinator::checkbox_options_default({theme_var});")
        state.nodes.append(
            f"    if (const tuinator::CheckboxStyle* named = "
            f"tuinator::checkbox_style_named({cpp_string(str(style_name))})) {{"
            f"\n        tuinator::apply_checkbox_style({opts_var}, *named, {theme_var});"
            f"\n    }}"
        )
        assign_options_fields(
            state.nodes, opts_var, merge_options_source(node), "CheckboxOptions",
            theme_var, emit_style, emit_enum,
        )
    else:
        opts_var = opts_builder(state, var, "CheckboxOptions", node, theme_var)
    glyphs = node_property(node, "glyphs")
    if isinstance(glyphs, dict):
        assign_options_fields(
            state.nodes, f"{opts_var}.glyphs", glyphs, "CheckboxGlyphs", theme_var, emit_style, emit_enum
        )
    on_change = node_property(node, "onChange", "on_change")
    handler_body = f"{handler_call(on_change, state, app_var)};" if on_change else None
    callback = wrap_constructor_callback(
        node.get("id"), state.binding_specs, state.ids, app_var, "bool", "value", handler_body
    )
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Checkbox>("
        f"{cpp_string(label)}, {checked}, {opts_var}, {callback});"
    )
    register_id(node, var, "Checkbox", state)
    return var


def generate_toggle(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Toggle")
    label = node_property(node, "label", "text", default="")
    checked = cpp_bool(node_property(node, "checked", default=False))
    style = emit_style(node_property(node, "style", default={}), theme_var)
    checked_style = emit_style(node_property(node, "checkedStyle", default="accent"), theme_var)
    on_change = node_property(node, "onChange", "on_change")
    handler_body = f"{handler_call(on_change, state, app_var)};" if on_change else None
    callback = wrap_constructor_callback(
        node.get("id"), state.binding_specs, state.ids, app_var, "bool", "value", handler_body
    )
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Toggle>("
        f"{cpp_string(label)}, {checked}, {callback}, {style}, {checked_style});"
    )
    register_id(node, var, "Toggle", state)
    return var


def generate_slider(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Slider")
    min_value = int(node_property(node, "min", "minValue", default=0))
    max_value = int(node_property(node, "max", "maxValue", default=100))
    value = int(node_property(node, "value", default=0))
    min_width = int(node_property(node, "minWidth", "min_width", default=20))
    style = emit_style(node_property(node, "style", default={}), theme_var)
    on_change = node_property(node, "onChange", "on_change")
    handler_body = f"{handler_call(on_change, state, app_var)};" if on_change else None
    callback = wrap_constructor_callback(
        node.get("id"), state.binding_specs, state.ids, app_var, "int", "value", handler_body
    )
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Slider>("
        f"{min_value}, {max_value}, {value}, {callback}, {style}, {min_width});"
    )
    register_id(node, var, "Slider", state)
    return var


def generate_spinner(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Spinner")
    min_value = int(node_property(node, "min", "minValue", default=0))
    max_value = int(node_property(node, "max", "maxValue", default=100))
    value = int(node_property(node, "value", default=0))
    step = int(node_property(node, "step", default=1))
    style = emit_style(node_property(node, "style", default={}), theme_var)
    on_change = node_property(node, "onChange", "on_change")
    handler_body = f"{handler_call(on_change, state, app_var)};" if on_change else None
    callback = wrap_constructor_callback(
        node.get("id"), state.binding_specs, state.ids, app_var, "int", "value", handler_body
    )
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Spinner>("
        f"{min_value}, {max_value}, {value}, {step}, {callback}, {style});"
    )
    register_id(node, var, "Spinner", state)
    return var


def generate_combo_box(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("ComboBox")
    item_style = emit_style(node_property(node, "itemStyle", default={}), theme_var)
    selected_style = emit_style(node_property(node, "selectedStyle", default="accent"), theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::ComboBox>({item_style}, {selected_style});"
    )
    items = node_property(node, "items", default=[])
    if items:
        items_var = emit_string_vector(items, state.next_var("combo_items"), state)
        state.nodes.append(f"    {var}->set_items(std::move({items_var}));")
    selected = node_property(node, "selectedIndex", "selected_index")
    if selected is not None:
        state.nodes.append(f"    {var}->set_selected_index({int(selected)});")
    on_select = node_property(node, "onSelect", "on_select")
    if on_select:
        schedule_ptr_callback(
            node, var, "ComboBox", state,
            f"set_on_select([&{app_var}](int index, const std::string& item) {{ "
            f"{handler_call(on_select, state, app_var, 'index, item')}; }});",
        )
    register_id(node, var, "ComboBox", state)
    return var


def generate_progress_bar(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("ProgressBar")
    value = float(node_property(node, "value", default=0.0))
    layout = node_property(node, "layout")
    if layout:
        layout_enum = emit_enum("ProgressBarLayout", layout)
        init_expr = (
            f"tuinator::progress_bar_preset({layout_enum}, {theme_var}.accent, {theme_var}.muted)"
        )
        opts_var = opts_builder(state, var, "ProgressBarOptions", node, theme_var, init_expr)
    else:
        opts_var = opts_builder(state, var, "ProgressBarOptions", node, theme_var)
    stats = node_property(node, "stats")
    if isinstance(stats, dict):
        state.nodes.append(
            f"    {opts_var}.stats.current = {int(stats.get('current', 0))};"
            f"\n    {opts_var}.stats.total = {int(stats.get('total', 100))};"
            f"\n    {opts_var}.stats.unit = {cpp_string(str(stats.get('unit', 'step')))};"
            f"\n    {opts_var}.stats.rate = {float(stats.get('rate', 0.0))};"
            f"\n    {opts_var}.stats.elapsed_ms = {int(stats.get('elapsedMs', stats.get('elapsed_ms', 0)))};"
        )
    glyphs = node_property(node, "glyphs")
    if isinstance(glyphs, dict):
        assign_options_fields(
            state.nodes, f"{opts_var}.glyphs", glyphs, "ProgressBarGlyphs",
            theme_var, emit_style, emit_enum,
        )
    gradient_stops = node_property(node, "gradientStops", "gradient_stops")
    if isinstance(gradient_stops, list) and gradient_stops:
        state.nodes.append(
            f"    {opts_var}.gradient_stops = "
            f"{emit_gradient_stops(gradient_stops, 'progress_bar_gradient', theme_var, emit_style)};"
        )
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::ProgressBar>({value}, {opts_var});")
    register_id(node, var, "ProgressBar", state)
    return var


def generate_throbber(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("Throbber")
    name = node_property(node, "name", "set", default="dots")
    style = emit_style(node_property(node, "style", default={}), theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Throbber>({cpp_string(name)}, {style});"
    )
    register_id(node, var, "Throbber", state)
    return var


def generate_big_text(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("BigText")
    text = node_property(node, "text", default="")
    style = emit_style(node_property(node, "style", default={}), theme_var)
    source = merge_options_source(node)
    look_name = lookup_field_value(source, ("lookName", "look_name", "look"))
    style_name = node_property(node, "styleName", "style_name")
    kind = node_property(node, "kind")
    option_keys = {
        "kind", "letterSpacing", "letter_spacing", "scale", "rainbow", "underline",
        "gradientAxis", "gradient_axis", "gradientStops", "gradient_stops",
        "shadowStyle", "shadow_style", "shadowDx", "shadow_dx", "shadowDy", "shadow_dy",
        "shadowLayers", "shadow_layers", "palette", "look", "lookName", "look_name",
    }
    has_options = look_name is not None or any(key in source for key in option_keys)
    if has_options:
        opts_var = f"{var}_options"
        state.nodes.append(f"    tuinator::BigTextOptions {opts_var};")
        if look_name:
            state.nodes.append(
                f"    if (const tuinator::BigTextLook* look = "
                f"tuinator::big_text_look_named({cpp_string(str(look_name))})) {{"
                f"\n        tuinator::apply_big_text_look({opts_var}, *look);"
                f"\n    }}"
            )
        assign_options_fields(
            state.nodes, opts_var, source, "BigTextOptions", theme_var, emit_style, emit_enum
        )
        gradient_stops = lookup_field_value(source, ("gradientStops", "gradient_stops"))
        if isinstance(gradient_stops, list) and gradient_stops:
            state.nodes.append(
                f"    {opts_var}.gradient_stops = "
                f"{emit_gradient_stops(gradient_stops, 'big_text_gradient', theme_var, emit_style)};"
            )
        palette = source.get("palette")
        if isinstance(palette, list) and palette:
            palette_var = f"{var}_palette"
            emit_style_vector(state.nodes, palette_var, palette, theme_var)
            state.nodes.append(f"    {opts_var}.palette = std::move({palette_var});")
        state.nodes.append(
            f"    auto {var} = std::make_unique<tuinator::BigText>("
            f"{cpp_string(text)}, {opts_var}, {style});"
        )
    elif style_name:
        state.nodes.append(
            f"    auto {var} = std::make_unique<tuinator::BigText>("
            f"{cpp_string(text)}, {cpp_string(style_name)}, {style});"
        )
    elif kind:
        state.nodes.append(
            f"    auto {var} = std::make_unique<tuinator::BigText>("
            f"{cpp_string(text)}, {emit_enum('BigTextKind', kind)}, {style});"
        )
    else:
        state.nodes.append(
            f"    auto {var} = std::make_unique<tuinator::BigText>({cpp_string(text)}, {style});"
        )
    register_id(node, var, "BigText", state)
    return var


def generate_image_view(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("ImageView")
    path = node_property(node, "path", "image", default="examples/assets/lenna.png")
    width = node_property(node, "width", "displayWidth", default=0)
    height = node_property(node, "height", "displayHeight", default=0)
    state.add_include("<optional>")
    state.nodes.append(f"    tuinator::TerminalImage {var}_image;")
    state.nodes.append("    {")
    state.nodes.append(f"        const std::string {var}_path = {cpp_string(path)};")
    state.nodes.append(f"        if (auto loaded = tuinator::TerminalImage::load_png({var}_path)) {{")
    state.nodes.append(f"            {var}_image = std::move(*loaded);")
    state.nodes.append("        } else if (auto loaded = tuinator::TerminalImage::load_ppm(" + f"{var}_path)) {{")
    state.nodes.append(f"            {var}_image = std::move(*loaded);")
    state.nodes.append("        }")
    state.nodes.append("    }")
    if width and height:
        state.nodes.append(
            f"    auto {var} = std::make_unique<tuinator::ImageView>("
            f"std::move({var}_image), tuinator::Size{{{int(width)}, {int(height)}}});"
        )
    else:
        state.nodes.append(
            f"    auto {var} = std::make_unique<tuinator::ImageView>(std::move({var}_image));"
        )
    register_id(node, var, "ImageView", state)
    return var


def generate_nerd_icon_catalog_gallery(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("NerdIconCatalogGallery")
    style_var = opts_builder(state, var, "NerdIconCatalogGalleryStyle", node, theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::NerdIconCatalogGallery>({style_var});")
    register_id(node, var, "NerdIconCatalogGallery", state)
    return var


def generate_nerd_glyph_matrix_gallery(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("NerdGlyphMatrixGallery")
    columns = int(node_property(node, "columns", default=32))
    data_root = node_property(node, "dataRoot", "data_root", default="data")
    sections_var = state.next_var("glyph_sections")
    state.nodes.append(
        f"    auto {sections_var} = tuinator::load_nerd_glyph_catalog({cpp_string(data_root)});"
    )
    style_var = opts_builder(state, var, "NerdGlyphMatrixGalleryStyle", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::NerdGlyphMatrixGallery>("
        f"std::move({sections_var}), {columns}, {style_var});"
    )
    register_id(node, var, "NerdGlyphMatrixGallery", state)
    return var


def generate_list_view(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("ListView")
    item_style = emit_style(node_property(node, "itemStyle", default={}), theme_var)
    selected_style = emit_style(node_property(node, "selectedStyle", default="accent"), theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::ListView>({item_style}, {selected_style});"
    )
    items = node_property(node, "items", default=[])
    if items:
        items_var = emit_string_vector(items, state.next_var("list_items"), state)
        state.nodes.append(f"    {var}->set_items(std::move({items_var}));")
    selected = node_property(node, "selectedIndex", "selected_index")
    if selected is not None:
        state.nodes.append(f"    {var}->set_selected_index({int(selected)});")
    on_select = node_property(node, "onSelect", "on_select")
    if on_select:
        schedule_ptr_callback(
            node, var, "ListView", state,
            f"set_on_select([&{app_var}](int index, const std::string& item) {{ "
            f"{handler_call(on_select, state, app_var, 'index, item')}; }});",
        )
    on_activate = node_property(node, "onActivate", "on_activate")
    if on_activate:
        schedule_ptr_callback(
            node, var, "ListView", state,
            f"set_on_activate([&{app_var}](int index, const std::string& item) {{ "
            f"{handler_call(on_activate, state, app_var, 'index, item')}; }});",
        )
    register_id(node, var, "ListView", state)
    return var


def generate_table(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Table")
    header_style = emit_style(node_property(node, "headerStyle", default="heading"), theme_var)
    cell_style = emit_style(node_property(node, "cellStyle", default={}), theme_var)
    selected_style = emit_style(node_property(node, "selectedStyle", default="accent"), theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Table>({header_style}, {cell_style}, {selected_style});"
    )
    columns = node_property(node, "columns", default=[])
    if columns:
        cols_var = state.next_var("table_columns")
        state.nodes.append(f"    std::vector<tuinator::TableColumn> {cols_var};")
        for col in columns:
            title = col.get("title", "") if isinstance(col, dict) else str(col)
            width = int(col.get("width", 10)) if isinstance(col, dict) else 10
            state.nodes.append(
                f"    {cols_var}.push_back(tuinator::TableColumn{{{cpp_string(title)}, {width}}});"
            )
        state.nodes.append(f"    {var}->set_columns(std::move({cols_var}));")
    rows = node_property(node, "rows", default=[])
    if rows:
        rows_var = state.next_var("table_rows")
        state.nodes.append(f"    std::vector<std::vector<std::string>> {rows_var};")
        for row in rows:
            row_var = state.next_var("table_row")
            items_var = emit_string_vector(row, row_var, state)
            state.nodes.append(f"    {rows_var}.push_back(std::move({items_var}));")
        state.nodes.append(f"    {var}->set_rows(std::move({rows_var}));")
    on_activate = node_property(node, "onActivate", "on_activate")
    if on_activate:
        schedule_ptr_callback(
            node, var, "Table", state,
            f"set_on_activate([&{app_var}](int row, const std::vector<std::string>& cells) {{ "
            f"{handler_call(on_activate, state, app_var, 'row, cells')}; }});",
        )
    register_id(node, var, "Table", state)
    return var


def generate_tree_view(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("TreeView")
    item_style = emit_style(node_property(node, "itemStyle", default={}), theme_var)
    selected_style = emit_style(node_property(node, "selectedStyle", default="accent"), theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::TreeView>({item_style}, {selected_style});"
    )
    root = node_property(node, "root")
    if root:
        root_var = state.next_var("tree_root")
        emit_tree_node(root, root_var, state)
        state.nodes.append(f"    {var}->set_root(std::move({root_var}));")
    on_select = node_property(node, "onSelect", "on_select")
    if on_select:
        schedule_ptr_callback(
            node, var, "TreeView", state,
            f"set_on_select([&{app_var}](const std::string& path) {{ "
            f"{handler_call(on_select, state, app_var, 'path')}; }});",
        )
    register_id(node, var, "TreeView", state)
    return var


def emit_chart_bars(bars: list[Any], var: str, state: GenerationState, theme_var: str) -> None:
    state.nodes.append(f"    std::vector<tuinator::BarChartBar> {var};")
    for bar in bars:
        if not isinstance(bar, dict):
            raise ValueError("BarChart bars must be objects")
        label = bar.get("label", "")
        value = float(bar.get("value", 0.0))
        bar_style = bar.get("style")
        if bar_style is not None:
            style_expr = emit_style(bar_style, theme_var)
            state.nodes.append(
                f"    {var}.push_back(tuinator::BarChartBar{{{cpp_string(label)}, {value}, {style_expr}}});"
            )
        else:
            state.nodes.append(
                f"    {var}.push_back(tuinator::BarChartBar{{{cpp_string(label)}, {value}}});"
            )


def generate_bar_chart(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("BarChart")
    bars_var = state.next_var("bar_chart_bars")
    emit_chart_bars(node_property(node, "bars", default=[]), bars_var, state, theme_var)
    opts_var = opts_builder(state, var, "BarChartOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::BarChart>(std::move({bars_var}), {opts_var});"
    )
    on_change = node_property(node, "onChange", "on_change")
    if on_change:
        schedule_ptr_callback(
            node, var, "BarChart", state,
            f"set_on_change([&{app_var}](const std::vector<tuinator::BarChartBar>& bars) {{ "
            f"{handler_call(on_change, state, app_var, 'bars')}; }});",
        )
    register_id(node, var, "BarChart", state)
    return var


def emit_line_series(series: list[Any], var: str, state: GenerationState, theme_var: str) -> None:
    state.nodes.append(f"    std::vector<tuinator::LineChartSeries> {var};")
    for entry in series:
        if not isinstance(entry, dict):
            raise ValueError("LineChart series must be objects")
        label = entry.get("label", "")
        values = ", ".join(str(float(v)) for v in entry.get("values", []))
        style_expr = emit_style(entry.get("style", {}), theme_var)
        state.nodes.append(
            f"    {var}.push_back(tuinator::LineChartSeries{{{cpp_string(label)}, "
            f"{{{values}}}, {style_expr}}});"
        )


def generate_line_chart(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("LineChart")
    series_var = state.next_var("line_chart_series")
    emit_line_series(node_property(node, "series", default=[]), series_var, state, theme_var)
    opts_var = opts_builder(state, var, "LineChartOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::LineChart>(std::move({series_var}), {opts_var});"
    )
    register_id(node, var, "LineChart", state)
    return var


def generate_pie_chart(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("PieChart")
    slices_var = state.next_var("pie_slices")
    state.nodes.append(f"    std::vector<tuinator::PieChartSlice> {slices_var};")
    for slice_ in node_property(node, "slices", default=[]):
        label = slice_.get("label", "") if isinstance(slice_, dict) else ""
        value = float(slice_.get("value", 0.0)) if isinstance(slice_, dict) else 0.0
        if isinstance(slice_, dict) and slice_.get("style") is not None:
            style_expr = emit_style(slice_["style"], theme_var)
            state.nodes.append(
                f"    {slices_var}.push_back(tuinator::PieChartSlice{{{cpp_string(label)}, {value}, {style_expr}}});"
            )
        else:
            state.nodes.append(
                f"    {slices_var}.push_back(tuinator::PieChartSlice{{{cpp_string(label)}, {value}}});"
            )
    opts_var = opts_builder(state, var, "PieChartOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::PieChart>(std::move({slices_var}), {opts_var});"
    )
    on_change = node_property(node, "onChange", "on_change")
    if on_change:
        schedule_ptr_callback(
            node, var, "PieChart", state,
            f"set_on_change([&{app_var}](const std::vector<tuinator::PieChartSlice>& slices) {{ "
            f"{handler_call(on_change, state, app_var, 'slices')}; }});",
        )
    register_id(node, var, "PieChart", state)
    return var


def generate_gauge_chart(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("GaugeChart")
    value = float(node_property(node, "value", default=0.0))
    opts_var = opts_builder(state, var, "GaugeChartOptions", node, theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::GaugeChart>({value}, {opts_var});")
    register_id(node, var, "GaugeChart", state)
    return var


def generate_histogram(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("Histogram")
    bins_var = state.next_var("histogram_bins")
    state.nodes.append(f"    std::vector<tuinator::HistogramBin> {bins_var};")
    for bin_ in node_property(node, "bins", default=[]):
        label = bin_.get("label", "") if isinstance(bin_, dict) else ""
        count = float(bin_.get("count", 0.0)) if isinstance(bin_, dict) else 0.0
        if isinstance(bin_, dict) and bin_.get("style") is not None:
            style_expr = emit_style(bin_["style"], theme_var)
            state.nodes.append(
                f"    {bins_var}.push_back(tuinator::HistogramBin{{{cpp_string(label)}, {count}, {style_expr}}});"
            )
        else:
            state.nodes.append(
                f"    {bins_var}.push_back(tuinator::HistogramBin{{{cpp_string(label)}, {count}}});"
            )
    opts_var = opts_builder(state, var, "HistogramOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Histogram>(std::move({bins_var}), {opts_var});"
    )
    register_id(node, var, "Histogram", state)
    return var


def generate_heatmap(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("Heatmap")
    values = node_property(node, "values", default=[])
    row_labels = node_property(node, "rowLabels", "row_labels", default=[])
    col_labels = node_property(node, "colLabels", "col_labels", default=[])
    values_var = state.next_var("heatmap_values")
    state.nodes.append(f"    std::vector<std::vector<double>> {values_var};")
    for row in values:
        row_expr = ", ".join(str(float(v)) for v in row)
        state.nodes.append(f"    {values_var}.push_back({{{row_expr}}});")
    rows_var = emit_string_vector(row_labels, state.next_var("heatmap_rows"), state)
    cols_var = emit_string_vector(col_labels, state.next_var("heatmap_cols"), state)
    opts_var = opts_builder(state, var, "HeatmapOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::Heatmap>("
        f"std::move({values_var}), std::move({rows_var}), std::move({cols_var}), {opts_var});"
    )
    register_id(node, var, "Heatmap", state)
    return var


def generate_candlestick_chart(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("CandlestickChart")
    bars_var = state.next_var("ohlc_bars")
    state.nodes.append(f"    std::vector<tuinator::OhlcBar> {bars_var};")
    for bar in node_property(node, "bars", default=[]):
        if not isinstance(bar, dict):
            continue
        state.nodes.append(
            f"    {bars_var}.push_back(tuinator::OhlcBar{{"
            f"{cpp_string(bar.get('label', ''))}, "
            f"{float(bar.get('open', 0))}, {float(bar.get('high', 0))}, "
            f"{float(bar.get('low', 0))}, {float(bar.get('close', 0))}, "
            f"{float(bar.get('volume', 0))}}});"
        )
    opts_var = opts_builder(state, var, "CandlestickChartOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::CandlestickChart>(std::move({bars_var}), {opts_var});"
    )
    register_id(node, var, "CandlestickChart", state)
    return var


def emit_stacked_series(series: list[Any], var: str, state: GenerationState, theme_var: str) -> None:
    state.nodes.append(f"    std::vector<tuinator::StackedAreaSeries> {var};")
    for entry in series:
        label = entry.get("label", "") if isinstance(entry, dict) else ""
        values = ", ".join(str(float(v)) for v in entry.get("values", [])) if isinstance(entry, dict) else ""
        style_expr = emit_style(entry.get("style", {}), theme_var) if isinstance(entry, dict) else "tuinator::Style{}"
        state.nodes.append(
            f"    {var}.push_back(tuinator::StackedAreaSeries{{{cpp_string(label)}, {{{values}}}, {style_expr}}});"
        )


def generate_stacked_area_chart(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("StackedAreaChart")
    series_var = state.next_var("stacked_series")
    emit_stacked_series(node_property(node, "series", default=[]), series_var, state, theme_var)
    opts_var = opts_builder(state, var, "StackedAreaChartOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::StackedAreaChart>(std::move({series_var}), {opts_var});"
    )
    register_id(node, var, "StackedAreaChart", state)
    return var


def generate_waterfall_chart(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("WaterfallChart")
    steps_var = state.next_var("waterfall_steps")
    state.nodes.append(f"    std::vector<tuinator::WaterfallStep> {steps_var};")
    for step in node_property(node, "steps", default=[]):
        if not isinstance(step, dict):
            continue
        delta = float(step.get("delta", 0.0))
        up_style = step.get("upStyle", step.get("up_style"))
        down_style = step.get("downStyle", step.get("down_style"))
        if up_style is not None or down_style is not None:
            up_expr = emit_style(up_style or {}, theme_var)
            down_expr = emit_style(down_style or {}, theme_var)
            state.nodes.append(
                f"    {steps_var}.push_back(tuinator::WaterfallStep{{"
                f"{cpp_string(step.get('label', ''))}, {delta}, {up_expr}, {down_expr}}});"
            )
        else:
            state.nodes.append(
                f"    {steps_var}.push_back(tuinator::WaterfallStep{{"
                f"{cpp_string(step.get('label', ''))}, {delta}}});"
            )
    opts_var = opts_builder(state, var, "WaterfallChartOptions", node, theme_var)
    state.nodes.append(
        f"    auto {var} = std::make_unique<tuinator::WaterfallChart>(std::move({steps_var}), {opts_var});"
    )
    register_id(node, var, "WaterfallChart", state)
    return var


def generate_menu_bar(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("MenuBar")
    style = emit_style(node_property(node, "style", default={}), theme_var)
    active_style = emit_style(node_property(node, "activeStyle", default="accent"), theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::MenuBar>({style}, {active_style});")
    menus = node_property(node, "menus", default=[])
    if menus:
        menus_var = state.next_var("menus")
        state.nodes.append(f"    std::vector<tuinator::Menu> {menus_var};")
        for menu in menus:
            title = menu.get("title", "") if isinstance(menu, dict) else ""
            items_var = state.next_var("menu_items")
            emit_menu_items(menu.get("items", []) if isinstance(menu, dict) else [], items_var, state, app_var)
            state.nodes.append(
                f"    {menus_var}.push_back(tuinator::Menu{{{cpp_string(title)}, std::move({items_var})}});"
            )
        state.nodes.append(f"    {var}->set_menus(std::move({menus_var}));")
    look = node_property(node, "look", "lookName", "look_name")
    if look:
        state.post_init.append(emit_menu_bar_look(var, str(look), theme_var))
    on_action = node_property(node, "onAction", "on_action")
    if on_action:
        schedule_ptr_callback(
            node, var, "MenuBar", state,
            f"set_on_action([&{app_var}](const std::string& menu, const std::string& item) {{ "
            f"{handler_call(on_action, state, app_var, 'menu, item')}; }});",
        )
    register_id(node, var, "MenuBar", state)
    return var


def generate_context_menu(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("ContextMenu")
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::ContextMenu>();")
    items = node_property(node, "items", default=[])
    if items:
        items_var = state.next_var("context_items")
        emit_menu_items(items, items_var, state, app_var)
        state.nodes.append(f"    {var}->set_items(std::move({items_var}));")
    look = node_property(node, "look", "lookName", "look_name")
    if look:
        preset = str(look).lower()
        if preset in {"classic", "mac", "minimal"}:
            factory = {
                "classic": "menu_bar_look_classic",
                "mac": "menu_bar_look_mac",
                "minimal": "menu_bar_look_minimal",
            }[preset]
            state.post_init.append(f"    {var}->set_look(tuinator::{factory}({theme_var}));")
        else:
            state.post_init.append(
                f"    if (const tuinator::MenuBarLook* named = tuinator::menu_bar_look_named({cpp_string(str(look))})) {{"
                f"\n        {var}->set_look(*named);"
                f"\n    }}"
            )
    on_action = node_property(node, "onAction", "on_action")
    if on_action:
        schedule_ptr_callback(
            node, var, "ContextMenu", state,
            f"set_on_action([&{app_var}](const std::string& item) {{ "
            f"{handler_call(on_action, state, app_var, 'item')}; }});",
        )
    register_id(node, var, "ContextMenu", state)
    return var


def generate_command_palette(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("CommandPalette")
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::CommandPalette>();")
    entries = node_property(node, "entries", default=[])
    if entries:
        entries_var = state.next_var("palette_entries")
        state.nodes.append(f"    std::vector<tuinator::CommandPaletteEntry> {entries_var};")
        for entry in entries:
            if not isinstance(entry, dict):
                continue
            on_click = entry.get("onClick", entry.get("on_click"))
            action = f"[&{app_var}]() {{ {handler_call(on_click, state, app_var)}; }}" if on_click else "{}"
            state.nodes.append(
                f"    {entries_var}.push_back(tuinator::CommandPaletteEntry{{"
                f"{cpp_string(entry.get('id', ''))}, {cpp_string(entry.get('label', ''))}, "
                f"{cpp_string(entry.get('category', ''))}, {cpp_string(entry.get('shortcut', ''))}, "
                f"{action}}});"
            )
        state.nodes.append(f"    {var}->set_entries(std::move({entries_var}));")
    on_close = node_property(node, "onClose", "on_close")
    if on_close:
        schedule_ptr_callback(
            node, var, "CommandPalette", state,
            f"set_on_close([&{app_var}]() {{ {handler_call(on_close, state, app_var)}; }});",
        )
    register_id(node, var, "CommandPalette", state)
    return var


def generate_status_bar(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("StatusBar")
    text = node_property(node, "text", default="")
    style = emit_style(node_property(node, "style", default="muted"), theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::StatusBar>({cpp_string(text)}, {style});")
    register_id(node, var, "StatusBar", state)
    return var


def generate_status_line(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    var = state.next_var("StatusLine")
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::StatusLine>();")
    for region in ("left", "center", "right"):
        segments = node_property(node, region, f"status{region.capitalize()}")
        if segments:
            seg_var = state.next_var("status_segments")
            emit_status_segments(segments, seg_var, state)
            state.post_init.append(f"    {var}->set_{region}(std::move({seg_var}));")
    register_id(node, var, "StatusLine", state)
    return var


def generate_shell_terminal(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("ShellTerminal")
    style_var = opts_builder(state, var, "ShellTerminalStyle", node, theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::ShellTerminal>({style_var});")
    command = node_property(node, "command", "shell")
    if command:
        state.post_init.append(f"    {var}->start({cpp_string(command)});")
    on_exit = node_property(node, "onExit", "on_exit")
    if on_exit:
        schedule_ptr_callback(
            node, var, "ShellTerminal", state,
            f"set_on_exit([&{app_var}]() {{ {handler_call(on_exit, state, app_var)}; }});",
        )
    register_id(node, var, "ShellTerminal", state)
    return var


def emit_window_options(node: dict[str, Any], state: GenerationState, theme_var: str, prefix: str) -> str:
    return opts_builder(state, prefix, "WindowOptions", node, theme_var)


def emit_create_windows(
    host_var: str,
    method: str,
    windows: list[Any],
    state: GenerationState,
    theme_var: str,
    app_var: str,
    allow_modal: bool = False,
) -> None:
    for index, win in enumerate(windows):
        if not isinstance(win, dict):
            raise ValueError("Each window must be an object")
        content = win.get("content")
        if content is None:
            raise ValueError("Each window requires 'content'")
        content_var = generate_node(content, state, theme_var, app_var)
        bounds = emit_rect(win.get("bounds", {"x": 0, "y": 0, "width": 40, "height": 12}))
        title = win.get("title", "Window")
        opts_var = emit_window_options(win, state, theme_var, f"{host_var}_win_{index}")
        if win.get("modal") and allow_modal:
            state.nodes.append(
                f"    {host_var}->show_modal({cpp_string(title)}, {bounds}, std::move({content_var}));"
            )
            continue
        win_ptr = state.next_var("window_ptr")
        state.nodes.append(
            f"    tuinator::Window* {win_ptr} = {host_var}->{method}("
            f"{cpp_string(title)}, {bounds}, std::move({content_var}), {opts_var});"
        )
        on_close = win.get("onClose", win.get("on_close"))
        if on_close:
            state.post_init.append(
                f"    {win_ptr}->set_on_close([&{app_var}]() {{ {handler_call(on_close, state, app_var)}; }});"
            )


def generate_actions(scene: dict[str, Any], state: GenerationState, app_var: str) -> str | None:
    actions = scene.get("actions", [])
    if not actions:
        return None
    actions_var = "scene_actions"
    state.preamble.append(f"    auto {actions_var} = std::make_shared<tuinator::ActionRegistry>();")
    for action in actions:
        if not isinstance(action, dict):
            raise ValueError("Each action must be an object")
        on_trigger = action.get("onTrigger", action.get("on_trigger", action.get("onClick")))
        callback = "{}"
        if on_trigger:
            callback = f"[&{app_var}]() {{ {handler_call(on_trigger, state, app_var)}; }}"
        enabled = cpp_bool(action.get("enabled", True))
        state.preamble.append(
            f"    {actions_var}->register_action(tuinator::RegisteredAction{{"
            f"{cpp_string(action.get('id', ''))}, {cpp_string(action.get('label', ''))}, "
            f"{cpp_string(action.get('category', ''))}, {cpp_string(action.get('shortcut', ''))}, "
            f"{cpp_string(action.get('hint', ''))}, {cpp_string(action.get('icon', ''))}, "
            f"{callback}, {enabled}}});"
        )
    state.build_result_fields.append(f".actions = {actions_var}")
    return actions_var


def generate_timers(scene: dict[str, Any], state: GenerationState, app_var: str) -> None:
    for timer in scene.get("timers", []):
        if not isinstance(timer, dict):
            raise ValueError("Each timer must be an object")
        interval = int(timer.get("interval", timer.get("intervalMs", 1000)))
        repeat = timer.get("repeat", True)
        on_tick = timer.get("onTick", timer.get("on_tick"))
        if not on_tick:
            raise ValueError("Timer requires onTick handler")
        callback = f"[&{app_var}]() {{ {handler_call(on_tick, state, app_var)}; }}"
        if repeat:
            state.post_init.append(f"    app.set_interval({interval}, {callback});")
        else:
            state.post_init.append(f"    app.set_timeout({interval}, {callback});")


def populate_palette_from_actions(desktop_var: str, actions_var: str, state: GenerationState) -> None:
    state.post_init.append(f"    {{")
    state.post_init.append(f"        std::vector<tuinator::CommandPaletteEntry> entries;")
    state.post_init.append(
        f"        for (const tuinator::RegisteredAction* action : {actions_var}->all()) {{"
    )
    state.post_init.append(
        f"            entries.push_back(tuinator::CommandPaletteEntry{{"
        f"action->id, action->label, action->category, action->shortcut, action->callback}});"
    )
    state.post_init.append(f"        }}")
    state.post_init.append(f"        {desktop_var}->set_command_palette_entries(std::move(entries));")
    state.post_init.append(f"    }}")


def generate_desktop(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("Desktop")
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::Desktop>();")
    register_id(node, var, "Desktop", state)
    state.build_result_fields.append(f".desktop = {var}.get()")
    background = node.get("background")
    if background is not None:
        bg_var = generate_node(background, state, theme_var, app_var)
        state.nodes.append(f"    {var}->set_background(std::move({bg_var}));")
    emit_create_windows(var, "create_window", node.get("windows", []), state, theme_var, app_var, allow_modal=True)
    actions_var = node_property(node, "actionsVar", "actions_var")
    if actions_var:
        state.post_init.append(f"    {var}->set_action_registry({actions_var});")
    elif node_property(node, "useActions", "use_actions", default=state.has_actions):
        state.post_init.append(f"    {var}->set_action_registry(scene_actions);")
    context_menu = node_property(node, "contextMenu", "context_menu")
    if isinstance(context_menu, dict):
        items_var = state.next_var("context_menu_items")
        emit_menu_items(context_menu.get("items", []), items_var, state, app_var)
        state.post_init.append(f"    {var}->set_context_menu_items(std::move({items_var}));")
    palette = node_property(node, "commandPalette", "command_palette")
    if isinstance(palette, dict):
        if palette.get("fromActions", palette.get("from_actions")):
            state.post_init.append(f"    {var}->set_action_registry(scene_actions);")
            populate_palette_from_actions(var, "scene_actions", state)
        elif palette.get("entries"):
            entries_var = state.next_var("palette_entries")
            state.nodes.append(f"    std::vector<tuinator::CommandPaletteEntry> {entries_var};")
            for entry in palette["entries"]:
                on_click = entry.get("onClick", entry.get("on_click"))
                action = f"[&{app_var}]() {{ {handler_call(on_click, state, app_var)}; }}" if on_click else "{}"
                state.nodes.append(
                    f"    {entries_var}.push_back(tuinator::CommandPaletteEntry{{"
                    f"{cpp_string(entry.get('id', ''))}, {cpp_string(entry.get('label', ''))}, "
                    f"{cpp_string(entry.get('category', ''))}, {cpp_string(entry.get('shortcut', ''))}, "
                    f"{action}}});"
                )
            state.post_init.append(f"    {var}->set_command_palette_entries(std::move({entries_var}));")
    return var


def generate_window_host(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("WindowHost")
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::WindowHost>();")
    register_id(node, var, "WindowHost", state)
    state.build_result_fields.append(f".window_host = {var}.get()")
    emit_create_windows(var, "create_window", node.get("windows", []), state, theme_var, app_var)
    return var


def generate_message_dialog(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("MessageDialog")
    message = node_property(node, "message", "text", default="")
    on_ok = node_property(node, "onOk", "on_ok", "onClick", "on_click")
    callback = f"[&{app_var}]() {{ {handler_call(on_ok, state, app_var)}; }}" if on_ok else "{}"
    state.nodes.append(
        f"    auto {var} = tuinator::dialog::make_message(theme, {cpp_string(message)}, {callback});"
    )
    register_id(node, var, "Widget", state)
    return var


def generate_confirm_dialog(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("ConfirmDialog")
    message = node_property(node, "message", "text", default="")
    on_result = node_property(node, "onResult", "on_result", "onConfirm", "on_confirm")
    callback = (
        f"[&{app_var}](bool confirmed) {{ {handler_call(on_result, state, app_var, 'confirmed')}; }}"
        if on_result else "{}"
    )
    state.nodes.append(
        f"    auto {var} = tuinator::dialog::make_confirm(theme, {cpp_string(message)}, {callback});"
    )
    register_id(node, var, "Widget", state)
    return var


def generate_source_control_panel(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str) -> str:
    var = state.next_var("SourceControlPanel")
    style_var = opts_builder(state, var, "SourceControlPanelStyle", node, theme_var)
    state.nodes.append(f"    auto {var} = std::make_unique<tuinator::SourceControlPanel>({style_var});")
    header = node_property(node, "header")
    if isinstance(header, dict):
        state.post_init.append(
            f"    {var}->set_header({cpp_string(header.get('title', ''))}, "
            f"{cpp_string(header.get('subtitle', ''))});"
        )
    sections = node_property(node, "sections", default=[])
    if sections:
        sections_var = state.next_var("sc_sections")
        state.nodes.append(f"    std::vector<tuinator::SourceControlSection> {sections_var};")
        for section in sections:
            if not isinstance(section, dict):
                continue
            title = section.get("title", "")
            expanded = cpp_bool(section.get("expanded", True))
            entries_var = state.next_var("sc_entries")
            state.nodes.append(f"    std::vector<tuinator::SourceControlEntry> {entries_var};")
            for entry in section.get("entries", []):
                if not isinstance(entry, dict):
                    continue
                status = entry.get("status", "Modified")
                state.nodes.append(
                    f"    {entries_var}.push_back(tuinator::SourceControlEntry{{"
                    f"{cpp_string(entry.get('path', ''))}, "
                    f"{emit_enum('GitChangeStatus', status)}, "
                    f"{cpp_string(entry.get('additions', ''))}, "
                    f"{cpp_string(entry.get('deletions', ''))}, "
                    f"{cpp_string(entry.get('directoryTag', entry.get('directory_tag', '')))}, "
                    f"{cpp_bool(entry.get('selected', False))}}});"
                )
            state.nodes.append(
                f"    {sections_var}.push_back(tuinator::SourceControlSection{{"
                f"{cpp_string(title)}, std::move({entries_var}), {expanded}}});"
            )
        state.post_init.append(f"    {var}->set_sections(std::move({sections_var}));")
    on_select = node_property(node, "onSelect", "on_select")
    if on_select:
        schedule_ptr_callback(
            node, var, "SourceControlPanel", state,
            f"set_on_select([&{app_var}](int section_index, int entry_index, "
            f"const tuinator::SourceControlEntry& entry) {{ "
            f"{handler_call(on_select, state, app_var, 'section_index, entry_index, entry')}; }});",
        )
    register_id(node, var, "SourceControlPanel", state)
    return var


GENERATORS: dict[str, Callable[..., str]] = {}


def _register_generators() -> None:
    global GENERATORS
    GENERATORS = {
        "VBox": lambda n, s, t, a: generate_vbox_hbox(n, s, t, a, "VBox"),
        "HBox": lambda n, s, t, a: generate_vbox_hbox(n, s, t, a, "HBox"),
        "Grid": generate_grid,
        "Panel": generate_panel,
        "ScrollView": generate_scroll_view,
        "SplitPane": generate_split_pane,
        "Tabs": generate_tabs,
        "TerminalFrame": generate_terminal_frame,
        "Label": generate_label,
        "Button": generate_button,
        "TextInput": generate_text_input,
        "TextArea": generate_text_area,
        "Separator": generate_separator,
        "Checkbox": generate_checkbox,
        "Toggle": generate_toggle,
        "Slider": generate_slider,
        "Spinner": generate_spinner,
        "ComboBox": generate_combo_box,
        "ProgressBar": generate_progress_bar,
        "Throbber": generate_throbber,
        "BigText": generate_big_text,
        "ImageView": generate_image_view,
        "NerdIconCatalogGallery": generate_nerd_icon_catalog_gallery,
        "NerdGlyphMatrixGallery": generate_nerd_glyph_matrix_gallery,
        "ListView": generate_list_view,
        "Table": generate_table,
        "TreeView": generate_tree_view,
        "BarChart": generate_bar_chart,
        "LineChart": generate_line_chart,
        "PieChart": generate_pie_chart,
        "GaugeChart": generate_gauge_chart,
        "Histogram": generate_histogram,
        "Heatmap": generate_heatmap,
        "CandlestickChart": generate_candlestick_chart,
        "StackedAreaChart": generate_stacked_area_chart,
        "WaterfallChart": generate_waterfall_chart,
        "MenuBar": generate_menu_bar,
        "ContextMenu": generate_context_menu,
        "CommandPalette": generate_command_palette,
        "StatusBar": generate_status_bar,
        "StatusLine": generate_status_line,
        "ShellTerminal": generate_shell_terminal,
        "SourceControlPanel": generate_source_control_panel,
        "Desktop": generate_desktop,
        "WindowHost": generate_window_host,
        "MessageDialog": generate_message_dialog,
        "ConfirmDialog": generate_confirm_dialog,
    }


_register_generators()


def generate_node(node: dict[str, Any], state: GenerationState, theme_var: str, app_var: str = "app") -> str:
    node_type = node.get("type")
    if not node_type:
        raise ValueError("Each scene node requires a 'type' field")
    if node_type not in GENERATORS:
        supported = ", ".join(SUPPORTED_WIDGETS)
        raise ValueError(f"Unsupported widget type '{node_type}'. Supported: {supported}")
    if node_type not in COMPOSITE_CONTAINERS and node.get("children"):
        raise ValueError(f"Widget type '{node_type}' cannot have 'children'")
    return GENERATORS[node_type](node, state, theme_var, app_var)


def generate_bindings(state: GenerationState) -> None:
    if not state.binding_specs:
        return
    state.bindings.extend(emit_scene_bindings(state.binding_specs, state.ids))


def write_header(path: Path, namespace: str, handlers_header: str | None, extra_includes: set[str]) -> None:
    includes = ["#include <tuinator/tuinator.hpp>", "#include <memory>", "#include <functional>"]
    includes.extend(sorted(extra_includes))
    if handlers_header:
        includes.append(f'#include "{handlers_header}"')
    ns_parts = namespace.split("::")
    ns_open = "\n".join(f"namespace {part} {{" for part in ns_parts[:-1])
    if len(ns_parts) > 1:
        ns_open += f"\nnamespace {ns_parts[-1]} {{"
    ns_close = "\n".join("} // namespace" for _ in ns_parts)
    content = f"""#pragma once

{chr(10).join(includes)}

{ns_open}

struct BuildResult {{
    std::unique_ptr<tuinator::Widget> root;
    std::shared_ptr<tuinator::ActionRegistry> actions;
    tuinator::Desktop* desktop = nullptr;
    tuinator::WindowHost* window_host = nullptr;
}};

BuildResult build(tuinator::Application& app, const tuinator::Theme& theme);

{ns_close}
"""
    path.write_text(content, encoding="utf-8")


def write_source(
    path: Path,
    namespace: str,
    state: GenerationState,
    root_var: str,
    header_name: str,
) -> None:
    ns_parts = namespace.split("::")
    ns_open = "\n".join(f"namespace {part} {{" for part in ns_parts[:-1])
    if len(ns_parts) > 1:
        ns_open += f"\nnamespace {ns_parts[-1]} {{"
    ns_close = "\n".join("} // namespace" for _ in ns_parts)
    binding_helpers = ""
    if state.binding_specs:
        binding_helpers = binding_helper_header() + "\n"
    body = "\n".join(state.nodes)
    setup = "\n".join(state.post_init)
    bindings = "\n".join(state.bindings)
    trailing = "\n".join(part for part in (setup, bindings) if part)
    if trailing:
        trailing = "\n" + trailing
    preamble = "\n".join(state.preamble)
    if preamble:
        preamble += "\n"
    extra = sorted(state.extra_includes)
    source_includes = [
        "#include <memory>",
        "#include <string>",
        "#include <utility>",
        "#include <vector>",
        "#include <functional>",
    ]
    source_includes.extend(extra)
    result_lines = ["    BuildResult result;", f"    result.root = std::move({root_var});"]
    for field in state.build_result_fields:
        result_lines.append(f"    result{field};")
    result_lines.append("    return result;")
    result_block = "\n".join(result_lines)
    content = f"""// Generated from {state.scene_path.name}. Do not edit by hand.

#include "{header_name}"

{chr(10).join(source_includes)}

{binding_helpers}{ns_open}

BuildResult build(tuinator::Application& app, const tuinator::Theme& theme) {{
{preamble}{body}
{trailing}
{result_block}
}}

{ns_close}
"""
    path.write_text(content, encoding="utf-8")


def generate(
    scene_path: Path,
    handlers_header: str | None,
    namespace: str,
    header_path: Path,
    source_path: Path,
) -> None:
    scene = load_scene(scene_path)
    state = GenerationState(scene_path=scene_path, handlers_header=handlers_header, namespace=namespace)
    state.has_actions = bool(scene.get("actions"))
    state.binding_specs = parse_bindings(scene.get("bindings", []))
    theme_var = generate_theme_setup(scene, state)
    generate_actions(scene, state, "app")
    generate_timers(scene, state, "app")
    root_node = scene["root"]
    prescan_scene_ids(root_node, state)
    root_var = generate_node(root_node, state, theme_var, "app")
    if root_node.get("id"):
        capture_id_ptr(root_node, root_var, state)
    generate_bindings(state)
    write_header(header_path, namespace, handlers_header, state.extra_includes)
    write_source(source_path, namespace, state, root_var, header_path.name)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    scene_path = args.scene.resolve()
    if not scene_path.exists():
        print(f"Scene file not found: {scene_path}", file=sys.stderr)
        return 1
    namespace = scene_namespace(scene_path, args.namespace)
    stem = scene_path.name.replace(".scene.json", "_scene.generated")
    header_path = (args.header or scene_path.with_name(f"{stem}.hpp")).resolve()
    source_path = (args.source or scene_path.with_name(f"{stem}.cpp")).resolve()
    handlers_header = None
    if args.handlers:
        handlers_path = args.handlers.resolve()
        try:
            handlers_header = str(handlers_path.relative_to(header_path.parent))
        except ValueError:
            handlers_header = str(Path(os.path.relpath(handlers_path, header_path.parent)).as_posix())
    generate(scene_path, handlers_header, namespace, header_path, source_path)
    print(f"Wrote {header_path}")
    print(f"Wrote {source_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
