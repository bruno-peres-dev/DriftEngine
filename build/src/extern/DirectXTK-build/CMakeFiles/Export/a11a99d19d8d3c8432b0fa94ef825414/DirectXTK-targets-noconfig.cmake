#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Microsoft::DirectXTK" for configuration ""
set_property(TARGET Microsoft::DirectXTK APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(Microsoft::DirectXTK PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libDirectXTK.a"
  )

list(APPEND _cmake_import_check_targets Microsoft::DirectXTK )
list(APPEND _cmake_import_check_files_for_Microsoft::DirectXTK "${_IMPORT_PREFIX}/lib/libDirectXTK.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
