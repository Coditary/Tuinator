#!/usr/bin/env python3
"""Unit tests for scripts/generate_scene_cpp.py."""

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
GENERATOR = ROOT / "scripts" / "generate_scene_cpp.py"
SUPPORTED = [
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


def minimal_node(widget_type: str) -> dict:
    if widget_type in {"VBox", "HBox", "Grid"}:
        return {"type": widget_type, "children": [{"type": "Label", "text": "x"}]}
    if widget_type == "Panel":
        return {"type": widget_type, "title": "P", "content": {"type": "Label", "text": "x"}}
    if widget_type == "ScrollView":
        return {"type": widget_type, "content": {"type": "Label", "text": "x"}}
    if widget_type == "SplitPane":
        return {
            "type": widget_type,
            "first": {"type": "Label", "text": "a"},
            "second": {"type": "Label", "text": "b"},
        }
    if widget_type == "Tabs":
        return {"type": widget_type, "tabs": [{"title": "T", "content": {"type": "Label", "text": "x"}}]}
    if widget_type == "TerminalFrame":
        return {"type": widget_type, "title": "Frame", "content": {"type": "Label", "text": "x"}}
    if widget_type == "TreeView":
        return {"type": widget_type, "root": {"label": "Root", "children": [{"label": "Child"}]}}
    if widget_type == "Table":
        return {
            "type": widget_type,
            "columns": [{"title": "Name", "width": 8}],
            "rows": [["a"]],
        }
    if widget_type == "MenuBar":
        return {
            "type": widget_type,
            "menus": [{"title": "File", "items": [{"label": "Quit", "onClick": "quit"}]}],
        }
    if widget_type == "ContextMenu":
        return {"type": widget_type, "items": [{"label": "Quit", "onClick": "quit"}]}
    if widget_type == "CommandPalette":
        return {
            "type": widget_type,
            "entries": [{"id": "quit", "label": "Quit", "onClick": "quit"}],
        }
    if widget_type == "SourceControlPanel":
        return {
            "type": widget_type,
            "sections": [{"title": "Changes", "entries": [{"path": "a.cpp", "status": "Modified"}]}],
        }
    if widget_type == "Desktop":
        return {
            "type": widget_type,
            "background": {"type": "Label", "text": "Desktop"},
            "windows": [{
                "title": "App",
                "bounds": {"x": 2, "y": 2, "width": 20, "height": 8},
                "content": {"type": "Label", "text": "Window"},
            }],
        }
    if widget_type == "WindowHost":
        return {
            "type": widget_type,
            "windows": [{
                "title": "Inner",
                "bounds": [2, 2, 16, 6],
                "content": {"type": "Label", "text": "Nested"},
            }],
        }
    if widget_type == "MessageDialog":
        return {"type": widget_type, "message": "Hello"}
    if widget_type == "ConfirmDialog":
        return {"type": widget_type, "message": "Sure?"}
    if widget_type == "BarChart":
        return {"type": widget_type, "bars": [{"label": "A", "value": 1}]}
    if widget_type == "LineChart":
        return {"type": widget_type, "series": [{"label": "S", "values": [1, 2]}]}
    if widget_type == "PieChart":
        return {"type": widget_type, "slices": [{"label": "A", "value": 1}]}
    if widget_type == "Histogram":
        return {"type": widget_type, "bins": [{"label": "A", "count": 1}]}
    if widget_type == "Heatmap":
        return {"type": widget_type, "values": [[1, 2]], "rowLabels": ["r"], "colLabels": ["c"]}
    if widget_type == "CandlestickChart":
        return {
            "type": widget_type,
            "bars": [{"label": "A", "open": 1, "high": 2, "low": 0.5, "close": 1.5}],
        }
    if widget_type == "StackedAreaChart":
        return {"type": widget_type, "series": [{"label": "S", "values": [1, 2]}]}
    if widget_type == "WaterfallChart":
        return {"type": widget_type, "steps": [{"label": "A", "delta": 1}]}
    if widget_type == "ComboBox":
        return {"type": widget_type, "items": ["A", "B"]}
    if widget_type == "ListView":
        return {"type": widget_type, "items": ["A", "B"]}
    if widget_type == "StatusLine":
        return {"type": widget_type, "left": [{"text": "Ready"}]}
    return {"type": widget_type}


class GenerateSceneCppTest(unittest.TestCase):
    def run_generator(self, scene: dict, handlers: Path | None = None) -> tuple[str, str]:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            scene_path = tmp_path / "test.scene.json"
            scene_path.write_text(json.dumps(scene), encoding="utf-8")
            header_path = tmp_path / "test_scene.generated.hpp"
            source_path = tmp_path / "test_scene.generated.cpp"
            cmd = [
                sys.executable,
                str(GENERATOR),
                str(scene_path),
                "--header",
                str(header_path),
                "--source",
                str(source_path),
            ]
            if handlers is not None:
                cmd.extend(["--handlers", str(handlers)])
            result = subprocess.run(cmd, capture_output=True, text=True, check=False)
            self.assertEqual(result.returncode, 0, msg=result.stderr)
            return header_path.read_text(encoding="utf-8"), source_path.read_text(encoding="utf-8")

    def test_generates_label_and_button(self) -> None:
        handlers = ROOT / "examples" / "scene_handlers.hpp"
        scene = {
            "version": 1,
            "root": {
                "type": "VBox",
                "children": [
                    {"type": "Label", "text": "Hi", "style": "heading"},
                    {"type": "Button", "label": "Quit", "onClick": "quit"},
                ],
            },
        }
        header, source = self.run_generator(scene, handlers)
        self.assertIn("namespace test {", header)
        self.assertIn("BuildResult build", header)
        self.assertIn("std::make_unique<tuinator::Label>", source)
        self.assertIn("std::make_unique<tuinator::Button>", source)
        self.assertIn("scene_handlers::quit(app)", source)

    def test_binding_generation(self) -> None:
        scene = {
            "version": 1,
            "bindings": [
                {
                    "from": "username",
                    "to": "preview",
                    "template": "Hello {value}",
                    "fallback": "guest",
                }
            ],
            "root": {
                "type": "VBox",
                "children": [
                    {"type": "TextInput", "id": "username"},
                    {"type": "Label", "id": "preview", "text": "Hello guest"},
                ],
            },
        }
        _, source = self.run_generator(scene)
        self.assertIn("set_on_change", source)
        self.assertIn("set_text", source)
        self.assertIn("scene_username_ptr", source)

    def test_property_bindings_generate(self) -> None:
        scene = {
            "version": 1,
            "bindings": [
                {
                    "from": "volume",
                    "fromProperty": "value",
                    "to": "meter",
                    "toProperty": "value",
                    "converter": "percent01",
                },
                {
                    "from": "enabled",
                    "to": "status",
                    "template": "Enabled: {value}",
                    "converter": "yesNo",
                },
                {
                    "from": "mode",
                    "event": "select",
                    "to": "mode_label",
                    "fromProperty": "selectedItem",
                },
            ],
            "root": {
                "type": "VBox",
                "children": [
                    {"type": "Slider", "id": "volume", "min": 0, "max": 100, "value": 40},
                    {"type": "ProgressBar", "id": "meter", "value": 0},
                    {"type": "Checkbox", "id": "enabled", "label": "Enable", "checked": True},
                    {"type": "Label", "id": "status", "text": "Enabled: Yes"},
                    {
                        "type": "ComboBox",
                        "id": "mode",
                        "items": ["Fast", "Slow"],
                        "selectedIndex": 0,
                    },
                    {"type": "Label", "id": "mode_label", "text": "Fast"},
                ],
            },
        }
        _, source = self.run_generator(scene)
        self.assertIn("scene_volume_ptr", source)
        self.assertIn("static_cast<double>(value) / 100.0", source)
        self.assertIn("std::string(\"Yes\")", source)
        self.assertIn("set_on_select", source)
        self.assertIn("combo_selected_item(scene_mode_ptr)", source)

    def test_all_widgets_generate(self) -> None:
        handlers = ROOT / "examples" / "scene_handlers.hpp"
        for widget_type in SUPPORTED:
            scene = {"version": 1, "root": minimal_node(widget_type)}
            _, source = self.run_generator(scene, handlers)
            if widget_type == "MessageDialog":
                self.assertIn("dialog::make_message", source)
            elif widget_type == "ConfirmDialog":
                self.assertIn("dialog::make_confirm", source)
            else:
                self.assertIn(f"tuinator::{widget_type}", source, msg=widget_type)

    def test_rejects_unknown_widget(self) -> None:
        scene = {"version": 1, "root": {"type": "UnknownWidget"}}
        with tempfile.TemporaryDirectory() as tmp:
            scene_path = Path(tmp) / "bad.scene.json"
            scene_path.write_text(json.dumps(scene), encoding="utf-8")
            result = subprocess.run(
                [sys.executable, str(GENERATOR), str(scene_path)],
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertTrue(
                "Unsupported widget type" in result.stderr
                or "unsupported widget type" in result.stderr,
                msg=result.stderr,
            )

    def test_advanced_options_generate(self) -> None:
        scene = {
            "version": 1,
            "theme": {
                "preset": "dark",
                "glyphs": "Unicode",
                "borderStyle": "Rounded",
            },
            "root": {
                "type": "VBox",
                "children": [
                    {
                        "type": "ScrollView",
                        "scrollbars": {"preset": "Minimal", "horizontal": False},
                        "content": {"type": "Label", "text": "scroll"},
                    },
                    {
                        "type": "Panel",
                        "title": "Box",
                        "borderGlyphs": {
                            "topLeft": "+",
                            "topRight": "+",
                            "bottomLeft": "+",
                            "bottomRight": "+",
                            "horizontal": "-",
                            "vertical": "|",
                        },
                        "content": {"type": "Label", "text": "panel"},
                    },
                    {
                        "type": "BigText",
                        "text": "HI",
                        "lookName": "neon",
                        "scale": 2,
                        "rainbow": True,
                        "gradientStops": [
                            {"position": 0, "color": "#ff0000"},
                            {"position": 1, "color": {"r": 0, "g": 255, "b": 0}},
                        ],
                    },
                    {
                        "type": "ProgressBar",
                        "value": 0.5,
                        "gradientStops": [
                            {"position": 0, "color": "#3366ff"},
                            {"position": 1, "color": "#66ff99"},
                        ],
                    },
                    {
                        "type": "BarChart",
                        "bars": [
                            {"label": "A", "value": 1, "style": "accent"},
                            {"label": "B", "value": 2, "style": {"foreground": "Cyan"}},
                        ],
                    },
                    {
                        "type": "MenuBar",
                        "look": "mac",
                        "menus": [{"title": "File", "items": [{"label": "Quit"}]}],
                    },
                ],
            },
        }
        _, source = self.run_generator(scene)
        self.assertIn("scrollbar_options", source)
        self.assertIn("BorderGlyphs", source)
        self.assertIn("big_text_look_named", source)
        self.assertIn("progress_bar_gradient", source)
        self.assertIn("menu_bar_look_mac", source)
        self.assertIn("BarChartBar", source)

    def test_example_scenes_validate_basic(self) -> None:
        sys.path.insert(0, str(ROOT / "scripts"))
        from validate_scene_json import load_and_validate

        for path in (ROOT / "examples" / "scenes").glob("*.scene.json"):
            load_and_validate(path, schema_path=None)


if __name__ == "__main__":
    raise SystemExit(unittest.main())
