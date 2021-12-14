#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)

#2.project name
project(yolov5rt)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE Release)

#add_definitions(-DFEATURE_MATCH_EN)
#add_definitions(-DIS_SEND_DATA)

#include MOT headers
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/yolov5)
include_directories(${PROJECT_SOURCE_DIR}/deepsort)
include_directories(${PROJECT_SOURCE_DIR}/postprocess)
include_directories(${PROJECT_SOURCE_DIR}/../../3rd_party)
include_directories(${PROJECT_SOURCE_DIR}/../../utility)

#collect image acquisition files
file(GLOB YOLOV5_SRCS
    "${PROJECT_SOURCE_DIR}/yolov5/*.cpp"
	)

#collect source files
file(GLOB DEEPSORT_SRCS
    "${PROJECT_SOURCE_DIR}/deepsort/*.cpp"
	)

file(GLOB POSTPROCESS_SRCS
    "${PROJECT_SOURCE_DIR}/postprocess/*.cpp"
	)

file(GLOB UTILS_SRCS
    "${PROJECT_SOURCE_DIR}/../../utility/*.cpp"
	)

find_package(CUDA REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(test_yolov5rt "")

target_sources(test_yolov5rt
        PRIVATE
        "${PROJECT_SOURCE_DIR}/test_yolov5rt.cpp"
        ${DEEPSORT_SRCS}
        ${POSTPROCESS_SRCS}
        ${YOLOV5_SRCS}
        ${UTILS_SRCS}
        )

# link libraries
target_link_libraries(test_yolov5rt ${OpenCV_LIBS} ${CUDA_LIBS} -lglog)
