BUILD_DIR := build
CMAKE := cmake
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release
CMAKE_CACHE := $(BUILD_DIR)/CMakeCache.txt

DEMOS := hello form colors layout counter buttons windows mouse-test scroll theme dashboard data controls menu image textarea throbber bigtext checkbox piechart charts diffview weather glyphs
RUNNABLE := $(DEMOS) profile

.PHONY: all build configure clean rebuild help demos test test-all unit-test profile profile-quick $(RUNNABLE)

all: $(DEMOS:%=$(BUILD_DIR)/tuinator-%)

build: all

# Reconfigure only when the build dir or CMake config changed.
$(CMAKE_CACHE): CMakeLists.txt
	@mkdir -p $(BUILD_DIR)
	$(CMAKE) -B $(BUILD_DIR) $(CMAKE_FLAGS)

# Build only the requested target (and its dependencies).
$(BUILD_DIR)/tuinator-%: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-$*

$(BUILD_DIR)/tuinator-tests: $(CMAKE_CACHE)
	@$(CMAKE) --build $(BUILD_DIR) --target tuinator-tests

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
	@echo "  make test       Run all smoke tests"
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
	@echo "  make mouse-test Test mouse clicks"
	@echo ""
	@echo "Profiling:"
	@echo "  make profile       Startup latency profile"
	@echo "  make profile-quick Profile without waiting for input"
	@echo ""
	@echo "Controls: Mouse click on buttons | Tab/Shift+Tab = focus | Enter/Space = activate | q = quit"

demos:
	@echo "Available demos:"
	@for demo in $(DEMOS); do echo "  make $$demo"; done
