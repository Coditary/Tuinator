
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was TuinatorConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

find_dependency(ZLIB)

if(WIN32)
    if(NOT TUINATOR_PDCURSES_ROOT)
        find_path(PDCURSES_INCLUDE_DIR NAMES curses.h)
        find_library(PDCURSES_LIBRARY NAMES pdcurses pdcursesmod)
        if(NOT PDCURSES_INCLUDE_DIR OR NOT PDCURSES_LIBRARY)
            message(FATAL_ERROR
                "PDCurses not found. Set TUINATOR_PDCURSES_ROOT or install PDCurses.")
        endif()
        add_library(tuinator::pdcurses UNKNOWN IMPORTED)
        set_target_properties(tuinator::pdcurses PROPERTIES
            IMPORTED_LOCATION "${PDCURSES_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${PDCURSES_INCLUDE_DIR}"
        )
        set(_TUINATOR_CURSES_TARGET tuinator::pdcurses)
    endif()
else()
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(NCURSES REQUIRED IMPORTED_TARGET ncursesw)
        set(_TUINATOR_CURSES_TARGET PkgConfig::NCURSES)
    else()
        set(CURSES_NEED_NCURSES TRUE)
        set(CURSES_NEED_WIDE TRUE)
        find_package(Curses REQUIRED)
        if(NOT TARGET tuinator::ncurses)
            add_library(tuinator::ncurses UNKNOWN IMPORTED)
            set_target_properties(tuinator::ncurses PROPERTIES
                IMPORTED_LOCATION "${CURSES_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
            )
        endif()
        set(_TUINATOR_CURSES_TARGET tuinator::ncurses)
    endif()

    find_library(TUINATOR_UTIL_LIBRARY util)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/TuinatorTargets.cmake")

check_required_components(Tuinator)
