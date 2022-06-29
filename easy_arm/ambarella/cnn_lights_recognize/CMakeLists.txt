#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)

set(TOOLS_CHAIN_DIR /usr/local/linaro-aarch64-2020.09-gcc10.2-linux5.4) # 交叉工具链所在目录
set(THIRD_DIR /home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party)#第三方库目录
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_FIND_ROOT_PATH ${TOOLS_CHAIN_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_C_COMPILER ${TOOLS_CHAIN_DIR}/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_CHAIN_DIR}/bin/aarch64-linux-gnu-g++)

set(SDK_DIR /home/lpj/Downloads/unisinsight_amba_sdk_1.0.3)#sdk目录

#2.project name
project(cnn_lights_recognize)

#amba_sdk
# include_directories(${PROJECT_SOURCE_DIR}/include)

# include_directories(${THIRD_DIR}/amba_sdk/include)
# include_directories(${THIRD_DIR}/amba_sdk/include/7X22)
include_directories(${SDK_DIR}/include)
include_directories(${SDK_DIR}/include/7X22)

# link_directories(${THIRD_DIR}/amba_sdk/lib/7X22)
link_directories(${SDK_DIR}/lib/7X22)

# opencv
# include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/opencv/include)
# link_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/opencv/lib)

include_directories(${SDK_DIR}/lib/opencv/include)
link_directories(${SDK_DIR}/lib/opencv/lib)

#tbb
# include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/tbb/include)
# link_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/tbb/lib)

include_directories(${SDK_DIR}/lib/tbb/include)
link_directories(${SDK_DIR}/lib/tbb/lib)

#libpng
# include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/libpng/include)
# link_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/libpng/lib)

include_directories(${SDK_DIR}/lib/libpng/include)
link_directories(${SDK_DIR}/lib/libpng/lib)

#libjpeg
# include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/libjpeg-turbo/include)
# link_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/libjpeg-turbo/lib)

include_directories(${SDK_DIR}/lib/libjpeg-turbo/include)
link_directories(${SDK_DIR}/lib/libjpeg-turbo/lib)

#zlib
# include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/zlib/include)
# link_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/zlib/lib)

include_directories(${SDK_DIR}/lib/zlib/include)
link_directories(${SDK_DIR}/lib/zlib/lib)

#include MOT headers
include_directories(${PROJECT_SOURCE_DIR}/)
include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/traffic_lights_classifier)
include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/common)
include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/amba_inference)
include_directories(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/json/include)

#collect image acquisition files
# find_package(OpenCV REQUIRED)



# file(GLOB SRCS
#     "${PROJECT_SOURCE_DIR}/*.cpp"
# 	)

file(GLOB AMBASRCS
    "/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/amba_inference/*.cpp"
)

file(GLOB TRAFFICSRCS
    "/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/traffic_lights_classifier/*.cpp"
)

aux_source_directory(/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/3rd_party/json/src SRC_DIR)
add_executable(test_traffic_lights_classifier ${SRC_DIR})

target_sources(test_traffic_lights_classifier
        PRIVATE
        "/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/test_traffic_lights_classifier.cpp"
        # "${PROJECT_SOURCE_DIR}/demo.cpp"
        ${AMBASRCS}
        ${TRAFFICSRCS}
        )

# link libraries
target_link_libraries(test_traffic_lights_classifier
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_videoio
                      nnctrl
                      cavalry_mem
                      vproc
                      tbb
                      png
                      jpeg
                      z)


