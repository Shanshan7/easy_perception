CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT(amba_arm LANGUAGES CXX C ASM)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#include dependencies
include(cmake/ambacv25_3_dependencies.cmake)

#include_directories
message(STATUS "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

#build cnn_runtime
add_subdirectory(cnn_runtime)

#build test cnn
#add_subdirectory(cnn_runtime/tests)

#build cnn_lpr
add_subdirectory(cnn_lpr)