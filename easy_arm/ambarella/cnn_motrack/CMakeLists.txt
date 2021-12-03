#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)

#2.project name
project(cnn_motrack)

set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

#include amba_arm directories
include(${PROJECT_SOURCE_DIR}/../cmake/ambacv25_3_dependencies.cmake)

#include MOT headers
#include_directories(${PROJECT_SOURCE_DIR}/fairmot)

add_executable(test_socket "")

#collect source files
file(GLOB MOT_SRCS
    "${PROJECT_SOURCE_DIR}/fairmot/*.cpp"
    "${PROJECT_SOURCE_DIR}/fairmot/*.c"
	)

target_sources(test_socket
        PRIVATE
        "${PROJECT_SOURCE_DIR}/test_socket.cpp"
        # ${MOT_SRCS}
        )

# link libraries
target_link_libraries(test_socket
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_videoio
                      gflags
                      glog
                      tbb
                      jpeg
                      png
                      z
                      eazyai_opencv
                      nnctrl
                      vproc
                      cavalry_mem
                      smartfb
                      textinsert_v2
                      datatx
                      bz2
                      freetype
                      cvlib
                      pthread)