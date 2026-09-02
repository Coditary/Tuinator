#!/usr/bin/env python3
"""Regenerate weather_icon from data/nerd_icons/weather.tsv (110 core weather icons)."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    cmd = [sys.executable, str(ROOT / "scripts" / "generate_nerd_icon_catalog.py"), "Weather"]
    if "--verify" in sys.argv:
        cmd.append("--verify")
    return subprocess.call(cmd)


if __name__ == "__main__":
    raise SystemExit(main())
