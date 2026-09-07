#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# shellcheck source=scripts/quality/common.sh
source "${ROOT}/scripts/quality/common.sh"

mapfile -t FILES < <(quality_cpp_sources)
if ((${#FILES[@]} == 0)); then
    echo "No C++ source files found for cppcheck."
    exit 0
fi

cppcheck \
    --enable=warning,performance,portability \
    --error-exitcode=1 \
    --inline-suppr \
    --quiet \
    --std=c++17 \
    --language=c++ \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --suppress=unusedFunction \
    --suppress=duplInheritedMember \
    --suppress=noCopyConstructor \
    --suppress=noOperatorEq \
    --suppress=uninitMemberVarNoCtor \
    --suppress=passedByValue \
    --suppress=useInitializationList \
    -I "${ROOT}/include" \
    -I "${ROOT}/src" \
    -I "${ROOT}/tests" \
    -I "${ROOT}/tests/support" \
    -I "${ROOT}/examples" \
    "${FILES[@]}"
