#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "foonathan_memory" for configuration ""
set_property(TARGET foonathan_memory APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(foonathan_memory PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libfoonathan_memory-0.6.2.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS foonathan_memory )
list(APPEND _IMPORT_CHECK_FILES_FOR_foonathan_memory "${_IMPORT_PREFIX}/lib/libfoonathan_memory-0.6.2.a" )

# Import target "foonathan_memory_node_size_debugger" for configuration ""
set_property(TARGET foonathan_memory_node_size_debugger APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(foonathan_memory_node_size_debugger PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/nodesize_dbg"
  )

list(APPEND _IMPORT_CHECK_TARGETS foonathan_memory_node_size_debugger )
list(APPEND _IMPORT_CHECK_FILES_FOR_foonathan_memory_node_size_debugger "${_IMPORT_PREFIX}/bin/nodesize_dbg" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
