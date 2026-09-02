#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${ROOT}/build"

cmake --build "${BUILD}"

echo ""
echo "Tuinator demos — pick one to run:"
echo ""
echo "  ./build/tuinator-hello    Simple centered label"
echo "  ./build/tuinator-colors   Color and style showcase"
echo "  ./build/tuinator-layout   VBox/HBox nesting"
echo "  ./build/tuinator-counter  Interactive +/- counter"
echo "  ./build/tuinator-buttons  Tab focus between buttons"
echo "  ./build/tuinator-form     Small login-style form"
echo "  ./build/tuinator-scroll   ScrollView with long list"
echo "  ./build/tuinator-dashboard Full app shell demo"
echo "  ./build/tuinator-theme    Dark/light theme preview"
echo "  ./build/tuinator-textarea Multi-line editor with line numbers"
echo ""
echo "All demos: q to quit (except Quit button in buttons demo)"
echo ""

if [[ $# -gt 0 ]]; then
    exec "${BUILD}/tuinator-${1}"
fi
