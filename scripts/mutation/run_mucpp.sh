#!/usr/bin/env bash
# Run MuCPP mutation testing against Tuinator (use a git worktree or copy — MuCPP owns .git in the SUT root).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

MUCpp_BUILD="${MUCpp_BUILD:-build-mucpp}"
MUCpp_CLANG_STD="${MUCpp_CLANG_STD:-c++20}"
TARGETS=("${@}")

if ! command -v mucpp >/dev/null 2>&1; then
  echo "mucpp not found. Install from https://ucase.uca.es/mucpp/download.html"
  echo "Pick a build whose Clang major version matches: $(clang++ --version 2>/dev/null | head -1 || echo 'unknown')"
  exit 1
fi

if [[ ${#TARGETS[@]} -eq 0 ]]; then
  TARGETS=(
    src/widgets/views/list_view.cpp
  )
fi

if [[ -d .git ]] && [[ -z "${MUCpp_ALLOW_MAIN_REPO:-}" ]]; then
  echo "Warning: MuCPP initializes git branches for mutants in the system-under-test root."
  echo "Use a dedicated worktree/copy, or export MUCpp_ALLOW_MAIN_REPO=1 to continue on this tree."
  exit 1
fi

for artifact in reports_analyze tests_output.txt tests_output.tmp times_output.txt comparison_results.txt; do
  if [[ -e "$artifact" ]]; then
    echo "Remove or rename '$artifact' before MuCPP (see MuCPP requirements)."
    exit 1
  fi
done

if [[ ! -f list_operators.txt ]]; then
  cp mutation/list_operators.txt list_operators.txt
  echo "Copied mutation/list_operators.txt -> list_operators.txt"
fi

echo "Configuring ${MUCpp_BUILD} (compile_commands for Clang/MuCPP)..."
cmake -B "$MUCpp_BUILD" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DTUINATOR_BUILD_TESTS=ON \
  -DTUINATOR_ENABLE_TEST_IDS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

ln -sf "$MUCpp_BUILD/compile_commands.json" compile_commands.json

CLANG_ARGS=(-- "-std=${MUCpp_CLANG_STD}")

for src in "${TARGETS[@]}"; do
  if [[ ! -f "$src" ]]; then
    echo "Source not found: $src"
    exit 1
  fi
  echo "=== analyze $src ==="
  mucpp analyze "$src" "${CLANG_ARGS[@]}"
done

echo "=== applyall (mutant branches) ==="
for src in "${TARGETS[@]}"; do
  mucpp applyall "$src" "${CLANG_ARGS[@]}"
done

echo "=== compare (kill / alive / timeout) ==="
mucpp compare . -- "${CLANG_ARGS[@]}"

echo "Done. See comparison_results.txt and git branches m*_*."
