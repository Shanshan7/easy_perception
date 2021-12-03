project(image_capture)

#include drivers directories
include_directories(${easy_drivers_BINARY_DIR})
include_directories(${easy_drivers_SOURCE_DIR})

#set( CMAKE_BUILD_TYPE "Debug" )

add_executable(camera_image_capture ${PROJECT_SOURCE_DIR}/image_capture.cc)
target_link_libraries(camera_image_capture 
    opencv_core 
    opencv_imgcodecs 
    opencv_imgproc
    opencv_highgui
    opencv_videoio)





