#!/usr/bin/env python3
"""Unit tests for scripts/scene_bindings.py."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from scene_bindings import (
    BindingSpec,
    emit_assignment,
    parse_bindings,
    resolve_source_property,
    resolve_target_property,
)


class SceneBindingsTest(unittest.TestCase):
    def test_parse_legacy_binding(self) -> None:
        specs = parse_bindings(
            [{"from": "username", "to": "preview", "template": "Hi {value}", "fallback": "guest"}]
        )
        self.assertEqual(len(specs), 1)
        self.assertEqual(specs[0].source_id, "username")
        self.assertEqual(specs[0].template, "Hi {value}")

    def test_resolve_defaults(self) -> None:
        self.assertEqual(resolve_source_property("Slider", None, "change"), "value")
        self.assertEqual(resolve_source_property("ComboBox", None, "select"), "selectedItem")
        self.assertEqual(resolve_target_property("ProgressBar", None), "value")
        self.assertEqual(resolve_target_property("Label", None), "text")

    def test_emit_slider_to_progress_binding(self) -> None:
        spec = BindingSpec(
            source_id="volume",
            target_id="meter",
            source_property="value",
            target_property="value",
            event="change",
            template=None,
            fallback=None,
            converter="percent01",
            initial=True,
        )
        source = SimpleNamespace(ptr_name="scene_volume_ptr", cpp_type="Slider")
        target = SimpleNamespace(ptr_name="scene_meter_ptr", cpp_type="ProgressBar")
        stmt = emit_assignment(source, target, spec, "value")
        self.assertIn("static_cast<double>(value) / 100.0", stmt)


if __name__ == "__main__":
    raise SystemExit(unittest.main())
