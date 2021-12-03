#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Poco::NetSSL" for configuration "RelWithDebInfo"
set_property(TARGET Poco::NetSSL APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Poco::NetSSL PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libPocoNetSSL.so.50"
  IMPORTED_SONAME_RELWITHDEBINFO "libPocoNetSSL.so.50"
  )

list(APPEND _IMPORT_CHECK_TARGETS Poco::NetSSL )
list(APPEND _IMPORT_CHECK_FILES_FOR_Poco::NetSSL "${_IMPORT_PREFIX}/lib/libPocoNetSSL.so.50" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
