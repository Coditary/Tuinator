#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# shellcheck source=scripts/quality/common.sh
source "${ROOT}/scripts/quality/common.sh"

BUILD_DIR="${1:-${ROOT}/build-quality}"
JOBS="${JOBS:-$(nproc)}"

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
    cmake -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DTUINATOR_BUILD_EXAMPLES=OFF \
        -DTUINATOR_BUILD_TESTS=ON
fi

mapfile -t FILES < <(quality_cpp_sources)
if ((${#FILES[@]} == 0)); then
    echo "No C++ source files found for clang-tidy."
    exit 0
fi

run-clang-tidy -p "${BUILD_DIR}" -j "${JOBS}" -quiet "${FILES[@]}"
