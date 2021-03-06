#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)

#2.project name
project(cnn_motrack)

set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#add_definitions(-DIS_SHOW)
#add_definitions(-DIS_SEND_DATA)
#add_definitions(-DFAIRMOT)
#add_definitions(-DFEATURE_MATCH_EN)

#include amba_arm directories
include(${PROJECT_SOURCE_DIR}/../cmake/ambacv25_3_dependencies.cmake)

#include MOT headers
include_directories(${PROJECT_SOURCE_DIR}/3rdlibrary)
include_directories(${PROJECT_SOURCE_DIR}/3rdlibrary/json/include)
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/yolov5rt)
include_directories(${PROJECT_SOURCE_DIR}/yolov5rt/yolov5)
include_directories(${PROJECT_SOURCE_DIR}/yolov5rt/deepsort)
include_directories(${PROJECT_SOURCE_DIR}/postprocess)
include_directories(${PROJECT_SOURCE_DIR}/image_acquisition)
include_directories(${PROJECT_SOURCE_DIR}/network)

#collect amba cv files
file(GLOB AMBA_SRCS
    "${PROJECT_SOURCE_DIR}/yolov5rt/*.cpp"
	)

#collect yolov5 files
file(GLOB YOLOV5_SRCS
    "${PROJECT_SOURCE_DIR}/yolov5rt/yolov5/*.cpp"
    "${PROJECT_SOURCE_DIR}/yolov5rt/yolov5/*.c"
	)

#collect deepsort files
file(GLOB DEEPSORT_SRCS
    "${PROJECT_SOURCE_DIR}/yolov5rt/deepsort/*.cpp"
	)

#collect postprocess files
file(GLOB POSTPROCESS_SRCS
    "${PROJECT_SOURCE_DIR}/postprocess/*.cpp"
    )

#collect UTILS files
file(GLOB UTILS_SRCS
    "${PROJECT_SOURCE_DIR}/common/*.cpp"
    "${PROJECT_SOURCE_DIR}/3rdlibrary/json/src/*.cpp"
	)

#collect image files
file(GLOB IMAGE_SRCS
    "${PROJECT_SOURCE_DIR}/image_acquisition/*.cpp"
	)

#collect network files
file(GLOB NETWORK_SRCS
    "${PROJECT_SOURCE_DIR}/network/*.cpp"
    )

add_executable(test_yolov5rt "")

target_sources(test_yolov5rt
        PRIVATE
        "${PROJECT_SOURCE_DIR}/test_yolov5rt.cpp"
        ${YOLOV5_SRCS}
        ${DEEPSORT_SRCS}
        ${POSTPROCESS_SRCS}
        ${AMBA_SRCS}
        ${UTILS_SRCS}
        ${IMAGE_SRCS}
        ${NETWORK_SRCS}
        )

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
