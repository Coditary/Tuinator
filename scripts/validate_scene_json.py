#!/usr/bin/env python3
"""Validate Tuinator scene JSON files against schema/scene.schema.json."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SCHEMA_PATH = ROOT / "schema" / "scene.schema.json"

SUPPORTED_WIDGETS = frozenset({
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
})

COMPOSITE_KEYS = {
    "VBox": ("children",),
    "HBox": ("children",),
    "Grid": ("children",),
    "Panel": ("content",),
    "ScrollView": ("content",),
    "SplitPane": ("first", "second"),
    "Tabs": ("tabs",),
    "TerminalFrame": ("content",),
    "Desktop": ("background", "windows"),
    "WindowHost": ("windows",),
}


class SceneValidationError(ValueError):
    pass


def validate_with_jsonschema(scene: dict[str, Any], schema_path: Path) -> bool:
    try:
        import jsonschema
    except ImportError:
        return False
    schema = json.loads(schema_path.read_text(encoding="utf-8"))
    jsonschema.validate(scene, schema)
    return True


def validate_basic(scene: Any, path: str = "$") -> None:
    if not isinstance(scene, dict):
        raise SceneValidationError(f"{path}: scene root must be an object")
    if scene.get("version", 1) != 1:
        raise SceneValidationError(f"{path}: only version 1 is supported")
    if "root" not in scene:
        raise SceneValidationError(f"{path}: missing required property 'root'")
    validate_node(scene["root"], f"{path}.root")


def validate_node(node: Any, path: str) -> None:
    if not isinstance(node, dict):
        raise SceneValidationError(f"{path}: widget node must be an object")
    widget_type = node.get("type")
    if not isinstance(widget_type, str):
        raise SceneValidationError(f"{path}: missing or invalid 'type'")
    if widget_type not in SUPPORTED_WIDGETS:
        raise SceneValidationError(f"{path}: unsupported widget type '{widget_type}'")
    for key in COMPOSITE_KEYS.get(widget_type, ()):
        if key == "tabs":
            tabs = node.get("tabs")
            if tabs is None:
                raise SceneValidationError(f"{path}: Tabs requires 'tabs'")
            if not isinstance(tabs, list):
                raise SceneValidationError(f"{path}.tabs: must be an array")
            for index, tab in enumerate(tabs):
                if not isinstance(tab, dict):
                    raise SceneValidationError(f"{path}.tabs[{index}]: must be an object")
                content = tab.get("content")
                if content is None:
                    raise SceneValidationError(f"{path}.tabs[{index}]: missing 'content'")
                validate_node(content, f"{path}.tabs[{index}].content")
            continue
        if key == "windows":
            windows = node.get("windows", [])
            if not isinstance(windows, list):
                raise SceneValidationError(f"{path}.windows: must be an array")
            for index, window in enumerate(windows):
                if not isinstance(window, dict):
                    raise SceneValidationError(f"{path}.windows[{index}]: must be an object")
                content = window.get("content")
                if content is not None:
                    validate_node(content, f"{path}.windows[{index}].content")
            continue
        child = node.get(key)
        if child is None:
            raise SceneValidationError(f"{path}: {widget_type} requires '{key}'")
        if key in {"children"}:
            if not isinstance(child, list):
                raise SceneValidationError(f"{path}.{key}: must be an array")
            for index, item in enumerate(child):
                validate_node(item, f"{path}.{key}[{index}]")
        else:
            validate_node(child, f"{path}.{key}")


def validate_scene(scene: dict[str, Any], schema_path: Path | None = SCHEMA_PATH) -> None:
    if schema_path and schema_path.exists():
        if validate_with_jsonschema(scene, schema_path):
            return
    validate_basic(scene)


def load_and_validate(path: Path, schema_path: Path | None = SCHEMA_PATH) -> dict[str, Any]:
    data = json.loads(path.read_text(encoding="utf-8"))
    validate_scene(data, schema_path)
    return data


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scenes", nargs="+", type=Path, help="Scene JSON files to validate")
    parser.add_argument(
        "--schema",
        type=Path,
        default=SCHEMA_PATH,
        help=f"JSON schema path (default: {SCHEMA_PATH})",
    )
    parser.add_argument(
        "--basic-only",
        action="store_true",
        help="Skip jsonschema and run structural checks only",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    schema_path = None if args.basic_only else args.schema.resolve()
    errors = 0
    for scene_path in args.scenes:
        try:
            load_and_validate(scene_path.resolve(), schema_path)
            print(f"OK {scene_path}")
        except (json.JSONDecodeError, SceneValidationError, ValueError) as exc:
            errors += 1
            print(f"FAIL {scene_path}: {exc}", file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
