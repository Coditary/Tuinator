BUILD_DIR := build
MUCpp_BUILD ?= build-mucpp
MUCpp_CLANG_STD ?= c++20
DEMO_DIR := $(BUILD_DIR)/examples
CMAKE := cmake
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release
MUCpp_CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Debug -DTUINATOR_BUILD_TESTS=ON -DTUINATOR_ENABLE_TEST_IDS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
CMAKE_CACHE := $(BUILD_DIR)/CMakeCache.txt
MUCpp_CACHE := $(MUCpp_BUILD)/CMakeCache.txt

DEMOS := hello form colors layout counter buttons windows mouse-test scroll scroll-hover theme dashboard data controls menu image textarea throbber bigtext checkbox piechart charts diffview weather glyphs terminal-frame scene scene-runtime split-panes markdown markdown-image markdown-gif asciidoc video
RUNNABLE := $(DEMOS) profile

.PHONY: all build configure clean rebuild help demos test test-all unit-test perf-test profile profile-quick scene-codegen scene-runtime-codegen scene-validate format format-check lint quality mucpp_clean mucpp_compilation mucpp_test mucpp_execute $(RUNNABLE)

all: $(DEMOS:%=$(DEMO_DIR)/tuinator-%)

build: all

# Reconfigure only when the build dir or CMake config changed.
$(CMAKE_CACHE): CMakeLists.txt cmake/*.cmake examples/CMakeLists.txt tests/CMakeLists.txt
	@mkdir -p $(BUILD_DIR)
	$(CMAKE) -B $(BUILD_DIR) $(CMAKE_FLAGS)

# Build only the requested target (and its dependencies).
$(DEMO_DIR)/tuinator-scene: examples/scenes/form.scene.json examples/scene_handlers.hpp scripts/generate_scene_cpp.py $(CMAKE_CACHE)
	@python3 scripts/generate_scene_cpp.py examples/scenes/form.scene.json --handlers examples/scene_handlers.hpp
	@python3 scripts/generate_scene_cpp.py examples/scenes/hello.scene.json --handlers examples/scene_handlers.hpp
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-scene

$(DEMO_DIR)/tuinator-scene-runtime: scripts/generate_scene_runtime_cpp.py scripts/scene_options.py $(CMAKE_CACHE)
	@python3 scripts/generate_scene_runtime_cpp.py
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-scene-runtime

$(DEMO_DIR)/tuinator-%: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-$*

$(BUILD_DIR)/tuinator-tests: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-tests

$(BUILD_DIR)/tuinator-perf-tests: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-perf-tests

define RUN_DEMO
$(1): $(DEMO_DIR)/tuinator-$(1)
	@./$(DEMO_DIR)/tuinator-$(1)
endef

$(foreach demo,$(DEMOS),$(eval $(call RUN_DEMO,$(demo))))

profile: $(DEMO_DIR)/tuinator-startup-profile
	@./$(DEMO_DIR)/tuinator-startup-profile

profile-quick: $(DEMO_DIR)/tuinator-startup-profile
	@TUINATOR_PROFILE_QUICK=1 ./$(DEMO_DIR)/tuinator-startup-profile

test test-all: build
	@chmod +x scripts/test-all.sh
	@./scripts/test-all.sh

unit-test: $(BUILD_DIR)/tuinator-tests
	@./$(BUILD_DIR)/tuinator-tests

# --- MuCPP mutation testing (https://ucase.uca.es/mucpp/) ---
mucpp_clean:
	@rm -f tests_output.tmp tests_output.txt times_output.tmp times_output.txt comparison_results.txt
	@rm -rf reports_analyze

$(MUCpp_CACHE): CMakeLists.txt cmake/*.cmake tests/CMakeLists.txt
	@mkdir -p $(MUCpp_BUILD)
	$(CMAKE) -B $(MUCpp_BUILD) $(MUCpp_CMAKE_FLAGS)
	@ln -sf $(MUCpp_BUILD)/compile_commands.json compile_commands.json

mucpp_compilation: $(MUCpp_CACHE)
	@$(CMAKE) --build $(MUCpp_BUILD) --target tuinator

mucpp_test: $(MUCpp_CACHE)
	@$(CMAKE) --build $(MUCpp_BUILD) --target tuinator-tests

mucpp_execute: mucpp_test
	@TUINATOR_MUCPP_OUTPUT=tests_output.tmp ./$(MUCpp_BUILD)/tuinator-tests

perf-test: $(BUILD_DIR)/tuinator-perf-tests
	@./$(BUILD_DIR)/tuinator-perf-tests

perf-report: $(BUILD_DIR)/tuinator-perf-tests
	@TUINATOR_PERF_REPORT_ONLY=1 ./$(BUILD_DIR)/tuinator-perf-tests

perf-ci: $(BUILD_DIR)/tuinator-perf-tests
	@TUINATOR_PERF_FACTOR=2 ./$(BUILD_DIR)/tuinator-perf-tests

format:
	@./scripts/quality/format.sh

format-check:
	@./scripts/quality/format-check.sh

lint:
	@./scripts/quality/clang-tidy.sh
	@./scripts/quality/cppcheck.sh

quality: format-check lint
	@echo "All quality checks passed."

rebuild: clean all

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Tuinator Makefile"
	@echo ""
	@echo "Build:"
	@echo "  make            Build all demos"
	@echo "  make build      Same as above"
	@echo "  make counter    Build (if needed) and run counter demo"
	@echo "  make clean      Remove build directory"
	@echo "  make rebuild    Clean + build"
	@echo ""
	@echo "Test:"
	@echo "  make unit-test  Run headless unit tests"
	@echo "  make perf-test    Run performance regression tests (strict)"
	@echo "  make perf-report  Perf tests, log over-budget without failing"
	@echo "  make perf-ci      Perf tests with 2x budget factor"
	@echo "  make test       Run all smoke tests"
	@echo ""
	@echo "Mutation (MuCPP, use a git worktree — see docs/mutation-testing.md):"
	@echo "  make mucpp_compilation mucpp_test mucpp_execute"
	@echo ""
	@echo "Quality:"
	@echo "  make format       Apply clang-format to tracked sources"
	@echo "  make format-check Verify formatting (clang-format)"
	@echo "  make lint         Run clang-tidy + cppcheck"
	@echo "  make quality      Run all quality checks"
	@echo ""
	@echo "Run demos:"
	@echo "  make hello      Centered label"
	@echo "  make colors     Color and style showcase"
	@echo "  make layout     VBox/HBox layout demo"
	@echo "  make counter    Interactive counter"
	@echo "  make buttons    Tab focus between buttons"
	@echo "  make form       Small login-style form"
	@echo "  make scroll     ScrollView with toggles + add rows"
	@echo "  make dashboard  App shell (SplitPane, Tabs, Grid, Panel)"
	@echo "  make data       ListView + Table + timers"
	@echo "  make controls   Showcase: ComboBox, Slider, TreeView, MenuBar, Dialog"
	@echo "  make menu       Full menu catalog: submenus, context menu, palette"
	@echo "  make image      Terminal image test (Kitty/Sixel/iTerm2)"
	@echo "  make textarea   Multi-line editor with line numbers"
	@echo "  make throbber   Loading spinners / throbber sets"
	@echo "  make bigtext    Large pixel / FIGlet banner text"
	@echo "  make piechart   Interactive pie charts (dots, braille, …)"
	@echo "  make charts     Bar/line graph gallery (many styles)"
	@echo "  make theme      Theme presets"
	@echo "  make scene          Login form built from generated C++"
	@echo "  make scene-runtime  Load form.scene.json at runtime"
	@echo "  make split-panes    Nested IDE-style split pane stress test"
	@echo "  make markdown       Streaming MarkdownView demo (AI-style pipe)"
	@echo "  make markdown-image Markdown image cases: block, inline, missing file"
	@echo "  make markdown-gif   Animated GIF inside a markdown stream"
	@echo "  make video          Video playback via ffmpeg pipe (terminal graphics)"
	@echo "  make asciidoc       Streaming AsciiDocView demo (AI-style pipe)"
	@echo ""
	@echo "Scene codegen:"
	@echo "  make scene-codegen  Regenerate C++ from examples/scenes/*.scene.json"
	@echo "  make scene-runtime-codegen  Regenerate runtime widget factory"
	@echo "  make scene-validate Validate example scene JSON files"
	@echo ""
	@echo "Profiling:"
	@echo "  make profile       Startup latency profile"
	@echo "  make profile-quick Profile without waiting for input"
	@echo ""
	@echo "Controls: Mouse click on buttons | Tab/Shift+Tab = focus | Enter/Space = activate | q = quit"

demos:
	@echo "Available demos:"
	@for demo in $(DEMOS); do echo "  make $$demo"; done

scene-codegen:
	@python3 scripts/generate_scene_cpp.py examples/scenes/hello.scene.json --handlers examples/scene_handlers.hpp
	@python3 scripts/generate_scene_cpp.py examples/scenes/form.scene.json --handlers examples/scene_handlers.hpp

scene-runtime-codegen:
	@python3 scripts/generate_scene_runtime_cpp.py

scene-validate:
	@python3 scripts/validate_scene_json.py --basic-only examples/scenes/*.scene.json
