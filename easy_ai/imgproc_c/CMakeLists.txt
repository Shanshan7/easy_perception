project(imgproc_c LANGUAGES C)

#include easy_ai directories
include_directories(${easy_ai_BINARY_DIR})
include_directories(${easy_ai_SOURCE_DIR})

#collect source files
file(GLOB IMGPROC_SRCS 
    "${PROJECT_SOURCE_DIR}/common/*.c"
	"${PROJECT_SOURCE_DIR}/base/*.c"
	"${PROJECT_SOURCE_DIR}/cvtcolor/*.c"
	"${PROJECT_SOURCE_DIR}/image_resize/*.c"
	"${PROJECT_SOURCE_DIR}/filtering/*.c"
	"${PROJECT_SOURCE_DIR}/features2d/*.c"
	"${PROJECT_SOURCE_DIR}/optical_flow/*.c"
	"${PROJECT_SOURCE_DIR}/clustering/*.c"
	"${PROJECT_SOURCE_DIR}/tracking/*.c"
	)

#build imgproc_c library
add_library(imgproc_c SHARED ${IMGPROC_SRCS})

install(DIRECTORY "${easy_ai_SOURCE_DIR}/imgproc_c" DESTINATION include ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
install(FILES "${easy_ai_BINARY_DIR}/libimgproc_c.so" DESTINATION lib ${CMAKE_INSTALL_LIBDIR})