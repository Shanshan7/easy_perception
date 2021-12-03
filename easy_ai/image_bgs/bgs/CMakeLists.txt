project(bgs)

#include easy_ai directories
include_directories(${easy_ai_BINARY_DIR})
include_directories(${easy_ai_SOURCE_DIR})

#collect source files
file(GLOB_RECURSE BGS_SRCS ${PROJECT_SOURCE_DIR}/*.cpp)

#build BGS_SRCS library
add_library(image_bgs SHARED ${BGS_SRCS})
target_link_libraries(image_bgs
    opencv_core
    opencv_imgcodecs 
    opencv_imgproc
    opencv_video
    opencv_highgui
)

install(DIRECTORY "${PROJECT_SOURCE_DIR}" DESTINATION include ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
install(FILES "${easy_ai_BINARY_DIR}/libimage_bgs.so" DESTINATION lib ${CMAKE_INSTALL_LIBDIR})