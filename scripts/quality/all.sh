#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

"${ROOT}/scripts/quality/format-check.sh"
"${ROOT}/scripts/quality/clang-tidy.sh"
"${ROOT}/scripts/quality/cppcheck.sh"

echo "All quality checks passed."
