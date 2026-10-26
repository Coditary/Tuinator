# Mutation testing with MuCPP

[MuCPP](https://ucase.uca.es/mucpp/) injects first-order mutants into C++ translation units (via Clang) and scores your test suite by how many mutants **fail** at least one test (“killed”).

Tuinator wires MuCPP through **Make targets** and a **MuCPP-specific test runner** that writes `tests_output.tmp` (one `0`/`1` line per registered test).

## Requirements

1. **MuCPP binary** matching your Clang major version ([download](https://ucase.uca.es/mucpp/download.html)). Fedora may need an Ubuntu container or a matching mucpp build.
2. **Separate tree**: MuCPP uses **git branches** for mutants. Do **not** run on your main dev clone unless you accept mutant branches and history noise. Prefer:

   ```bash
   git worktree add ../Tuinator-mucpp -b mucpp-run
   cd ../Tuinator-mucpp
   ```

3. No pre-existing `reports_analyze/`, `tests_output.txt`, `comparison_results.txt`, etc. (see [MuCPP requirements](https://ucase.uca.es/mucpp/requirements.html)).

## Quick start

```bash
chmod +x scripts/mutation/run_mucpp.sh

# Default: mutate list_view.cpp only
MUCpp_ALLOW_MAIN_REPO=1 ./scripts/mutation/run_mucpp.sh

# Explicit sources
MUCpp_ALLOW_MAIN_REPO=1 ./scripts/mutation/run_mucpp.sh \
  src/widgets/views/list_view.cpp \
  src/widgets/views/table.cpp
```

Operator subset: edit `mutation/list_operators.txt` (copied to `list_operators.txt` on first run).

## Make targets (MuCPP contract)

From the repo root (in a mutation worktree):

| Target | Purpose |
|--------|---------|
| `make mucpp_clean` | Remove MuCPP result files |
| `make mucpp_compilation` | Configure `build-mucpp` and build `tuinator` library |
| `make mucpp_test` | Build `tuinator-tests` |
| `make mucpp_execute` | Run suite → `tests_output.tmp` |

```bash
make mucpp_compilation mucpp_test mucpp_execute   # manual smoke
mucpp compare . --                                # after applyall
```

Build dir override: `MUCpp_BUILD=build-mucpp make mucpp_test`.

## Test output format

When `TUINATOR_MUCPP_OUTPUT` is set, `tuinator-tests` runs **all** tests (no early exit) and writes:

- `0` — test passed  
- `1` — test failed  

This matches MuCPP’s expected `tests_output.tmp` format.

Normal runs are unchanged: `./build-agent/tuinator-tests`.

## Interpreting results

`mucpp compare` prints per-mutant lines; a `1` in the test column means that test **detected** the mutant (good). Surviving mutants suggest missing assertions or equivalent mutants — add param/negative tests in `tests/widgets/edge/`.

## Suggested mutation targets

Start small (fast):

- `src/widgets/views/list_view.cpp`
- `src/widgets/views/table.cpp`
- `src/widgets/controls/combo_box.cpp`
- `src/widgets/containers/tabs.cpp`

Pair with filters when debugging: `./build-mucpp/tuinator-tests list_view` (substring filter).
