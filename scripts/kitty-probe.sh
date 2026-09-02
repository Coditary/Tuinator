#!/bin/sh
# Test whether your terminal (Ghostty/Kitty) shows Kitty graphics at all.
# Usage: ./scripts/kitty-probe.sh
# Expect: a small colored rectangle near the top-left of the terminal.

set -eu

send_chunked() {
    first="y"
    while IFS= read -r chunk; do
        metadata=""
        if [ "$first" = "y" ]; then
            metadata="a=T,f=100,"
            first="n"
        fi
        printf '\033_G%sm=1;%s\033\\' "$metadata" "$chunk"
    done
    if [ "$first" = "n" ]; then
        printf '\033_Gm=0;\033\\'
        return 0
    fi
    return 1
}

transmit_png() {
    png="$1"
    if command -v base64 >/dev/null 2>&1; then
        if base64 --help 2>&1 | grep -q -- '-w'; then
            base64 -w 4096 "$png" | send_chunked
            return
        fi
        if base64 --help 2>&1 | grep -q -- '-b'; then
            base64 -b 4096 "$png" | send_chunked
            return
        fi
    fi
    base64 "$png" | fold -b -w 4096 | send_chunked
}

tmpdir="${TMPDIR:-/tmp}"
png="$tmpdir/tuinator-kitty-probe.png"

python3 - <<'PY' "$png"
import sys
from pathlib import Path
try:
    from PIL import Image
except ImportError:
    raise SystemExit("Install pillow: pip install pillow")

path = Path(sys.argv[1])
img = Image.new("RGB", (80, 40))
for x in range(80):
    for y in range(40):
        img.putpixel((x, y), (60 + x * 2, 120 + y, 200 - y * 2))
img.save(path)
PY

printf '\033[2J\033[H\033[1;1H'
printf 'Kitty probe: if you see a gradient box below, graphics work.\n\n'
transmit_png "$png"
printf '\n\nProbe done. Press Enter to clear.'
read -r _
printf '\033_Ga=d,d=A;\033\\\033[2J\033[H'
