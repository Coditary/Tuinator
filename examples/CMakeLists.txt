set(TUINATOR_DEMOS
    golden-app hello form colors layout counter buttons windows mouse-test scroll theme startup-profile
    dashboard data controls menu image textarea throbber bigtext checkbox piechart charts
    diffview weather glyphs terminal-frame scene scene-runtime inline-tasks split-panes stylesheet
)

foreach(demo IN LISTS TUINATOR_DEMOS)
    if(demo STREQUAL "scene")
        add_executable(tuinator-scene
            scene.cpp
            scenes/form_scene.generated.cpp
        )
        target_include_directories(tuinator-scene PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    elseif(demo STREQUAL "scene-runtime")
        add_executable(tuinator-scene-runtime scene_runtime.cpp)
        target_include_directories(tuinator-scene-runtime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    else()
        add_executable(tuinator-${demo} ${demo}.cpp)
    endif()
    target_link_libraries(tuinator-${demo} PRIVATE tuinator::tuinator)
    set_target_properties(tuinator-${demo} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach()

target_compile_definitions(tuinator-image PRIVATE
    TUINATOR_LENNA_PATH="${TUINATOR_SOURCE_DIR}/examples/assets/lenna.png")

target_compile_definitions(tuinator-stylesheet PRIVATE
    TUINATOR_SOURCE_DIR="${TUINATOR_SOURCE_DIR}")
