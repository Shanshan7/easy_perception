project(ldws_detector)

add_compile_options(-fopenmp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

#include easy_ai directories
include_directories(${easy_ai_BINARY_DIR})
include_directories(${easy_ai_SOURCE_DIR})

#collect source files
file(GLOB LDWS_SRCS 
  "${PROJECT_SOURCE_DIR}/ldws_detector/*.cpp"
)

#build ldws_detector library
add_library(ldws_detector SHARED ${LDWS_SRCS})

#collect source files
file(GLOB TEST_LDWS_SRCS 
    "${PROJECT_SOURCE_DIR}/tools/*.cpp"
)

add_executable(camera_ldws_detector ${TEST_LDWS_SRCS})
target_link_libraries(camera_ldws_detector ldws_detector opencv_core opencv_videoio opencv_imgcodecs opencv_imgproc opencv_highgui)

install(FILES ldws_detector/LDWS_Interface.h DESTINATION include/ldws_detector ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${easy_ai_BINARY_DIR}/libldws_detector.so" DESTINATION lib ${CMAKE_INSTALL_LIBDIR})
