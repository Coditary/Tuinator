#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${ROOT}/build"
DEMO_DIR="${BUILD}/examples"

cmake --build "${BUILD}"

echo ""
echo "Tuinator demos — pick one to run:"
echo ""
echo "  ./build/examples/tuinator-hello    Simple centered label"
echo "  ./build/examples/tuinator-colors   Color and style showcase"
echo "  ./build/examples/tuinator-layout   VBox/HBox nesting"
echo "  ./build/examples/tuinator-counter  Interactive +/- counter"
echo "  ./build/examples/tuinator-buttons  Tab focus between buttons"
echo "  ./build/examples/tuinator-form     Small login-style form"
echo "  ./build/examples/tuinator-scroll   ScrollView with long list"
echo "  ./build/examples/tuinator-dashboard Full app shell demo"
echo "  ./build/examples/tuinator-theme    Dark/light theme preview"
echo "  ./build/examples/tuinator-textarea Multi-line editor with line numbers"
echo ""
echo "All demos: q to quit (except Quit button in buttons demo)"
echo ""

if [[ $# -gt 0 ]]; then
    exec "${DEMO_DIR}/tuinator-${1}"
fi
