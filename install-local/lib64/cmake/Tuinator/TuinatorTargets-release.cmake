#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "tuinator::tuinator" for configuration "Release"
set_property(TARGET tuinator::tuinator APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(tuinator::tuinator PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libtuinator.a"
  )

list(APPEND _cmake_import_check_targets tuinator::tuinator )
list(APPEND _cmake_import_check_files_for_tuinator::tuinator "${_IMPORT_PREFIX}/lib64/libtuinator.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
