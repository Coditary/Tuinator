include(${CMAKE_CURRENT_LIST_DIR}/TuinatorSources.cmake)

add_library(tuinator ${TUINATOR_LIBRARY_TYPE} ${TUINATOR_SOURCES})
add_library(tuinator::tuinator ALIAS tuinator)

target_sources(tuinator PRIVATE $<TARGET_OBJECTS:tuinator_libvterm>)

target_include_directories(tuinator
    PUBLIC
        $<BUILD_INTERFACE:${TUINATOR_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${TUINATOR_SOURCE_DIR}/src
        ${TUINATOR_CURSES_INCLUDE}
        ${TUINATOR_JSON_INCLUDE_DIR}
        ${LIBVTERM_GENERATED_SRC}
        ${libvterm_SOURCE_DIR}/include
        ${libvterm_SOURCE_DIR}/src
)

if(TUINATOR_CURSES_TARGET)
    target_link_libraries(tuinator PUBLIC ${TUINATOR_CURSES_TARGET})
else()
    target_link_libraries(tuinator PUBLIC ${TUINATOR_CURSES_LIBS})
endif()

target_link_libraries(tuinator
    PRIVATE
        ZLIB::ZLIB
)

if(NOT WIN32)
    find_library(TUINATOR_UTIL_LIBRARY util)
    if(TUINATOR_UTIL_LIBRARY)
        target_link_libraries(tuinator PUBLIC ${TUINATOR_UTIL_LIBRARY})
    endif()
endif()

target_compile_definitions(tuinator
    PUBLIC
        $<$<BOOL:${WIN32}>:TUINATOR_PLATFORM_WINDOWS=1>
    PRIVATE
        ${TUINATOR_BACKEND_DEF}
)

target_compile_options(tuinator PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

set_target_properties(tuinator PROPERTIES
    EXPORT_NAME tuinator
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(TUINATOR_INSTALL)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS tuinator
        EXPORT TuinatorTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(
        DIRECTORY ${TUINATOR_SOURCE_DIR}/include/tuinator
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT TuinatorTargets
        FILE TuinatorTargets.cmake
        NAMESPACE tuinator::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tuinator
    )

    configure_package_config_file(
        ${TUINATOR_SOURCE_DIR}/cmake/TuinatorConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/TuinatorConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tuinator
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/TuinatorConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/TuinatorConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/TuinatorConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tuinator
    )
endif()
