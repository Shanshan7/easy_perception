PROJECT(rec_text)

#include amba_arm directories
include_directories(${amba_arm_BINARY_DIR})
include_directories(${amba_arm_SOURCE_DIR})

#collect source files
file(GLOB CNN_COMMON_SRCS
    "${amba_arm_SOURCE_DIR}/cnn_runtime/cnn_common/*.cpp"
)

add_library(textnet SHARED textnet.cpp ${CNN_COMMON_SRCS})
