project(image_bgs)

#include easy_ai directories
include_directories(${easy_ai_BINARY_DIR})
include_directories(${easy_ai_SOURCE_DIR})

add_subdirectory(bgs)

#collect source files
file(GLOB DETECTION_SRCS
    "${PROJECT_SOURCE_DIR}/tools/*.cpp"
	)
list(REMOVE_ITEM DETECTION_SRCS ${PROJECT_SOURCE_DIR}/tools/moving_object_detection.cpp)

add_executable(moving_object_detection ${PROJECT_SOURCE_DIR}/tools/moving_object_detection.cpp ${DETECTION_SRCS})
target_link_libraries(moving_object_detection
    image_bgs
    opencv_core
    opencv_imgcodecs 
    opencv_imgproc
    opencv_video
    opencv_videoio
    opencv_highgui
)