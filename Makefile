BUILD_DIR := build
CMAKE := cmake
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release
CMAKE_CACHE := $(BUILD_DIR)/CMakeCache.txt

DEMOS := hello form colors layout counter buttons windows mouse-test scroll theme dashboard data controls menu image textarea throbber bigtext checkbox piechart charts diffview weather glyphs terminal-frame scene scene-runtime split-panes
RUNNABLE := $(DEMOS) profile

.PHONY: all build configure clean rebuild help demos test test-all unit-test perf-test profile profile-quick scene-codegen scene-runtime-codegen scene-validate format format-check lint quality $(RUNNABLE)

all: $(DEMOS:%=$(BUILD_DIR)/tuinator-%)

build: all

# Reconfigure only when the build dir or CMake config changed.
$(CMAKE_CACHE): CMakeLists.txt cmake/*.cmake examples/CMakeLists.txt tests/CMakeLists.txt
	@mkdir -p $(BUILD_DIR)
	$(CMAKE) -B $(BUILD_DIR) $(CMAKE_FLAGS)

# Build only the requested target (and its dependencies).
$(BUILD_DIR)/tuinator-scene: examples/scenes/form.scene.json examples/scene_handlers.hpp scripts/generate_scene_cpp.py $(CMAKE_CACHE)
	@python3 scripts/generate_scene_cpp.py examples/scenes/form.scene.json --handlers examples/scene_handlers.hpp
	@python3 scripts/generate_scene_cpp.py examples/scenes/hello.scene.json --handlers examples/scene_handlers.hpp
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-scene

$(BUILD_DIR)/tuinator-scene-runtime: scripts/generate_scene_runtime_cpp.py scripts/scene_options.py $(CMAKE_CACHE)
	@python3 scripts/generate_scene_runtime_cpp.py
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-scene-runtime

$(BUILD_DIR)/tuinator-%: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-$*

$(BUILD_DIR)/tuinator-tests: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-tests

$(BUILD_DIR)/tuinator-perf-tests: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-perf-tests

define RUN_DEMO
$(1): $(BUILD_DIR)/tuinator-$(1)
	@./$(BUILD_DIR)/tuinator-$(1)
endef

$(foreach demo,$(DEMOS),$(eval $(call RUN_DEMO,$(demo))))

profile: $(BUILD_DIR)/tuinator-startup-profile
	@./$(BUILD_DIR)/tuinator-startup-profile

profile-quick: $(BUILD_DIR)/tuinator-startup-profile
	@TUINATOR_PROFILE_QUICK=1 ./$(BUILD_DIR)/tuinator-startup-profile

test test-all: build
	@chmod +x scripts/test-all.sh
	@./scripts/test-all.sh

unit-test: $(BUILD_DIR)/tuinator-tests
	@./$(BUILD_DIR)/tuinator-tests

perf-test: $(BUILD_DIR)/tuinator-perf-tests
	@./$(BUILD_DIR)/tuinator-perf-tests

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
	@echo "  make perf-test  Run performance regression tests"
	@echo "  make test       Run all smoke tests"
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
