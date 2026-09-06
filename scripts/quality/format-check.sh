#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# shellcheck source=scripts/quality/common.sh
source "${ROOT}/scripts/quality/common.sh"

mapfile -t FILES < <(quality_sources)
if ((${#FILES[@]} == 0)); then
    echo "No source files found for format check."
    exit 0
fi

clang-format --dry-run --Werror --style=file "${FILES[@]}"
