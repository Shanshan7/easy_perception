#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)

#2.project name
project(yolov5rt)

set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

#add_definitions(-DIS_SHOW)
#add_definitions(-DIS_SEND_DATA)

#include amba_arm directories
include(${PROJECT_SOURCE_DIR}/../cmake/ambacv25_3_dependencies.cmake)

#include MOT headers
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/yolov5rt/yolov5)

#collect yolov5 files
file(GLOB YOLOV5_SRCS
    "${PROJECT_SOURCE_DIR}/yolov5rt/yolov5/*.cpp"
	)

#collect UTILS files
file(GLOB UTILS_SRCS
    "${PROJECT_SOURCE_DIR}/common/*.cpp"
	)


add_executable(test_yolov5rt "")

target_sources(test_yolov5rt
        PRIVATE
        "${PROJECT_SOURCE_DIR}/test_yolov5rt.cpp"
        ${YOLOV5_SRCS}
        ${UTILS_SRCS}
        )

# link libraries
# link libraries
target_link_libraries(test_yolov5rt
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
