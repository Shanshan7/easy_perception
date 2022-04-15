#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)
project(cnn_lights_recognize)

set(TOOLS_CHAIN_DIR /usr/local/linaro-aarch64-2020.09-gcc10.2-linux5.4) # 交叉工具链所在目录

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_FIND_ROOT_PATH ${TOOLS_CHAIN_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_C_COMPILER ${TOOLS_CHAIN_DIR}/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_CHAIN_DIR}/bin/aarch64-linux-gnu-g++)

include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/traffic_lights_classifier)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# opencv
include_directories(${PROJECT_SOURCE_DIR}/3rd_party/opencv/include)
link_directories(${PROJECT_SOURCE_DIR}/3rd_party/opencv/lib)

set(SRC_FILES
    ialg_error_code.h
    ialg_open_app.h

    impl.cpp
    json11.cpp
    json11.hpp
    task_queue.h
    ctx.h
    channel.h
)

set(LIGHTSREC_SRCS
    ${PROJECT_SOURCE_DIR}/common/image.cpp
    ${PROJECT_SOURCE_DIR}/traffic_lights_classifier/traffic_lights_classifier.cpp
)

set(LIB_NAME "tlr")
set(LIB_FULL_NAME "lib${LIB_NAME}.so")

add_library(${LIB_NAME} SHARED
    ${SRC_FILES} ${LIGHTSREC_SRCS}
)

# link libraries
target_link_libraries(${LIB_NAME}
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_videoio)

set(PACK_DIR_NAME "EdgeTLR")

add_custom_command(TARGET ${LIB_NAME}
    POST_BUILD
    COMMENT "make package ${CMAKE_BINARY_DIR}/${PACK_DIR_NAME}.tar.gz ..."

    COMMAND mkdir -p ${PACK_DIR_NAME}
    COMMAND mkdir -p ${PACK_DIR_NAME}/conf
    COMMAND mkdir -p ${PACK_DIR_NAME}/lib
    COMMAND mkdir -p ${PACK_DIR_NAME}/model

    COMMAND cp -rf ${LIB_FULL_NAME} ${PACK_DIR_NAME}/lib/
    COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/conf/* ${PACK_DIR_NAME}/conf/
    COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/model/* ${PACK_DIR_NAME}/model/
    COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/ALG-OpenAI ${PACK_DIR_NAME}/

    COMMAND tar zcf  ${CMAKE_BINARY_DIR}/${PACK_DIR_NAME}.tar.gz ${PACK_DIR_NAME}
)
