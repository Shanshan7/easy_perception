#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)

#2.project name
project(cnn_motrack)

set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

add_definitions(-DIS_SHOW)
add_definitions(-DIS_SEND_DATA)

#include amba_arm directories
include(${PROJECT_SOURCE_DIR}/../cmake/ambacv25_3_dependencies.cmake)

#include MOT headers
include_directories(${PROJECT_SOURCE_DIR}/fairmot)
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/network)
include_directories(${PROJECT_SOURCE_DIR}/postprocess)

add_executable(test_motrack "")

#collect image acquisition files
file(GLOB IMA_SRCS
    "${PROJECT_SOURCE_DIR}/image_acquisition/*.cpp"
	)

#collect source files
file(GLOB MOT_SRCS
    "${PROJECT_SOURCE_DIR}/fairmot/*.cpp"
    "${PROJECT_SOURCE_DIR}/fairmot/*.c"
	)

file(GLOB NETWORK_SRCS
    "${PROJECT_SOURCE_DIR}/network/*.cpp"
    )

file(GLOB POSTPROCESS_SRCS
    "${PROJECT_SOURCE_DIR}/postprocess/*.cpp"
    )

file(GLOB UTILS_SRCS
    "${PROJECT_SOURCE_DIR}/common/*.cpp"
    )

target_sources(test_motrack
        PRIVATE
        "${PROJECT_SOURCE_DIR}/test_motrack.cpp"
        ${MOT_SRCS}
        ${POSTPROCESS_SRCS}
        ${NETWORK_SRCS}
        ${UTILS_SRCS}
        )

# link libraries
target_link_libraries(test_motrack
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