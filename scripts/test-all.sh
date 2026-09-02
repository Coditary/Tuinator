#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${ROOT}/build"
TIMEOUT="${TUINATOR_TEST_TIMEOUT:-3}"

reset_terminal() {
    if [[ -w /dev/tty ]] 2>/dev/null; then
        printf '\033[?1000l\033[?1002l\033[?1003l\033[?1006l\033[0m\033[?25h' >/dev/tty 2>/dev/null || true
    fi
}

DEMOS=(
    hello
    form
    colors
    layout
    counter
    buttons
    windows
    scroll
    theme
    dashboard
    controls
    data
)

passed=0
failed=0

echo "Tuinator smoke tests"
echo "===================="
echo ""

if [[ -x "${BUILD}/tuinator-tests" ]]; then
    printf "[....] %-10s " "unit"
    if "${BUILD}/tuinator-tests"; then
        echo ""
    else
        echo "Unit tests failed."
        exit 1
    fi
    echo ""
fi

reset_terminal

if [[ ! -x "${BUILD}/tuinator-hello" ]]; then
    echo "Building demos first..."
    cmake --build "${BUILD}"
fi

for demo in "${DEMOS[@]}"; do
    binary="${BUILD}/tuinator-${demo}"
    if [[ ! -x "${binary}" ]]; then
        echo "[FAIL] ${demo} (binary missing)"
        failed=$((failed + 1))
        continue
    fi

    printf "[....] %-10s " "${demo}"
    if printf 'q' | timeout "${TIMEOUT}" env TUINATOR_HEADLESS=1 "${binary}" >/dev/null 2>&1; then
        echo "OK"
        passed=$((passed + 1))
    else
        echo "FAIL (exit $?)"
        failed=$((failed + 1))
    fi
    reset_terminal
done

printf "[....] %-10s " "profile"
if TUINATOR_PROFILE_QUICK=1 TUINATOR_HEADLESS=1 timeout "${TIMEOUT}" "${BUILD}/tuinator-startup-profile" >/dev/null 2>&1; then
    echo "OK"
    passed=$((passed + 1))
else
    echo "FAIL"
    failed=$((failed + 1))
fi

reset_terminal

echo ""
echo "Result: ${passed} passed, ${failed} failed"
echo ""
echo "Interactive demos to try manually:"
echo "  make windows    floating windows + modal"
echo "  make counter    interactive buttons"
echo "  make buttons    tab focus"

exit $(( failed > 0 ? 1 : 0 ))
