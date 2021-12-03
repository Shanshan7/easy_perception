project(mainboard)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})

file(GLOB CYBER_MAINBOARD_SRCS "${PROJECT_SOURCE_DIR}/*.cc")
add_executable(mainboard ${CYBER_MAINBOARD_SRCS})
target_link_libraries(mainboard cyber gflags glog)
target_compile_options(mainboard PRIVATE -Og)

install(TARGETS mainboard RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
