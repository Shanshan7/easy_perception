cmake_minimum_required(VERSION 3.12)

include(GNUInstallDirs)

#project setting
project(cyberRT_project)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-I")
set( CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-I" CACHE STRING "" FORCE )
set( CMAKE_INCLUDE_SYSTEM_FLAG_CUDA "-I" CACHE STRING "" FORCE )

option(BUILD_DOCKER "build in docker images" OFF)

#set(CMAKE_INSTALL_BINDIR /home/allen/cyber-xavier/build/bin)
#set(CMAKE_INSTALL_LIBDIR /home/allen/cyber-xavier/build/lib)
#set(CMAKE_INSTALL_INCLUDEDIR /home/allen/cyber-xavier/build/include)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
	message(STATUS "Platform: ${CMAKE_SYSTEM_PROCESSOR}")
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
	message(STATUS "Platform: ${CMAKE_SYSTEM_PROCESSOR}")
else()
	message(STATUS "Platform: ${CMAKE_SYSTEM_PROCESSOR}")
	message(FATAL_ERROR "Can not set swap source file, please check")
	return()
endif()

#include dependencies
if (BUILD_DOCKER)
    include(cmake/docker_cyber_dependencies.cmake)
else (BUILD_DOCKER)
	include(cmake/cyber_dependencies.cmake)
endif (BUILD_DOCKER)

#include_directories
message(STATUS "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

#build proto files
add_subdirectory(cyber/proto)

add_subdirectory(cyber)

add_subdirectory(cyber/mainboard)

add_subdirectory(cyber/io/example)

#build examples
add_subdirectory(cyber/examples/proto)
add_subdirectory(cyber/examples)

#build python wrapper
add_subdirectory(cyber/python)

#build tools 
add_subdirectory(cyber/tools)

#install
install(DIRECTORY cyber DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${PROJECT_BINARY_DIR}/cyber DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")

install(DIRECTORY cyber/conf DESTINATION ${CMAKE_INSTALL_PREFIX})
if (BUILD_DOCKER)
    install(FILES cyber/docker_setup.bash DESTINATION ${CMAKE_INSTALL_BINDIR})
	message(STATUS "third_party is installed")
else (BUILD_DOCKER)
    install(FILES cyber/setup.bash DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(DIRECTORY third_party/${CMAKE_SYSTEM_PROCESSOR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/third_party)
endif (BUILD_DOCKER)
