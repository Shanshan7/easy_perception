project(test_imgproc)

#include ai_algorithm directories
include_directories(${ai_algorithm_BINARY_DIR})
include_directories(${ai_algorithm_SOURCE_DIR})

#collect source files
file(GLOB TEST_IMGPROC_SRCS 
    "${PROJECT_SOURCE_DIR}/*.cpp"
)

add_executable(test_imgproc ${TEST_IMGPROC_SRCS})
target_link_libraries(test_imgproc imgproc_c opencv_core opencv_videoio opencv_imgcodecs opencv_imgproc opencv_highgui)
