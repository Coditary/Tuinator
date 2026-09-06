#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

quality_sources() {
    git -C "${ROOT}" ls-files '*.cpp' '*.hpp' \
        | grep -E '^(include|src|tests|examples)/' \
        | grep -Ev '\.generated\.(cpp|hpp)$' || true
}

quality_cpp_sources() {
    quality_sources | grep -E '\.cpp$' || true
}
