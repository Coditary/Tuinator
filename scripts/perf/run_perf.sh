#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="${TUINATOR_PERF_BUILD:-build}"
FILTER="${1:-}"

cd "$ROOT"

if [[ ! -x "${BUILD}/tuinator-perf-tests" ]]; then
  cmake -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DTUINATOR_BUILD_TESTS=ON
  cmake --build "$BUILD" --target tuinator-perf-tests
fi

export TUINATOR_PERF_REPORT_ONLY="${TUINATOR_PERF_REPORT_ONLY:-1}"
export TUINATOR_PERF_FACTOR="${TUINATOR_PERF_FACTOR:-1}"

JSON_OUT="${TUINATOR_PERF_JSON:-}"
if [[ -n "${JSON_OUT}" ]]; then
  rm -f "$JSON_OUT"
  export TUINATOR_PERF_JSON="$JSON_OUT"
fi

if [[ -n "$FILTER" ]]; then
  exec "./${BUILD}/tuinator-perf-tests" "$FILTER"
fi

exec "./${BUILD}/tuinator-perf-tests"
