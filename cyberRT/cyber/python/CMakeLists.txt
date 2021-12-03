project(cyber_py)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})

add_subdirectory(internal)

install(DIRECTORY cyber_py3 DESTINATION ${CMAKE_INSTALL_LIBDIR}/cyber/python)