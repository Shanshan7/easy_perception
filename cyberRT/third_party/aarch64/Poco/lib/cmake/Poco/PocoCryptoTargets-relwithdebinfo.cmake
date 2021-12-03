#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Poco::Crypto" for configuration "RelWithDebInfo"
set_property(TARGET Poco::Crypto APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Poco::Crypto PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libPocoCrypto.so.50"
  IMPORTED_SONAME_RELWITHDEBINFO "libPocoCrypto.so.50"
  )

list(APPEND _IMPORT_CHECK_TARGETS Poco::Crypto )
list(APPEND _IMPORT_CHECK_FILES_FOR_Poco::Crypto "${_IMPORT_PREFIX}/lib/libPocoCrypto.so.50" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
