#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)

#2.project name
project(cnn_motrack)

set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

#add_definitions(-DIS_SHOW)

#include amba_arm directories
#include(${PROJECT_SOURCE_DIR}/../cmake/ambacv25_3_dependencies.cmake)

#include MOT headers
# include_directories(${PROJECT_SOURCE_DIR}/fairmot)
# include_directories(${PROJECT_SOURCE_DIR}/)

add_executable(test_socket "")

# file(GLOB NETWORK_SRCS
#    "${PROJECT_SOURCE_DIR}/*.cpp"
#    )

target_sources(test_socket
        PRIVATE
        "${PROJECT_SOURCE_DIR}/test_socket.cpp"
        ${NETWORK_SRCS}
        )

# link libraries
target_link_libraries(test_socket)