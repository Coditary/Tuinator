include(FetchContent)

enable_language(C)

FetchContent_Declare(
    libvterm
    GIT_REPOSITORY https://github.com/neovim/libvterm.git
    GIT_TAG v0.3.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(libvterm)

set(LIBVTERM_GENERATED_SRC ${CMAKE_CURRENT_LIST_DIR}/libvterm/generated/src)

add_library(tuinator_libvterm STATIC
    ${libvterm_SOURCE_DIR}/src/encoding.c
    ${libvterm_SOURCE_DIR}/src/keyboard.c
    ${libvterm_SOURCE_DIR}/src/mouse.c
    ${libvterm_SOURCE_DIR}/src/parser.c
    ${libvterm_SOURCE_DIR}/src/pen.c
    ${libvterm_SOURCE_DIR}/src/screen.c
    ${libvterm_SOURCE_DIR}/src/state.c
    ${libvterm_SOURCE_DIR}/src/unicode.c
    ${libvterm_SOURCE_DIR}/src/vterm.c
)

target_include_directories(tuinator_libvterm PUBLIC
    ${LIBVTERM_GENERATED_SRC}
    ${libvterm_SOURCE_DIR}/include
    ${libvterm_SOURCE_DIR}/src
)

target_compile_options(tuinator_libvterm PRIVATE -Wno-unused-parameter)
