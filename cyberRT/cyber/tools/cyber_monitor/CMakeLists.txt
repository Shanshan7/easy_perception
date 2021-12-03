project(cyber_monitor)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})

file(GLOB CYBER_MONITOR_SRCS "${PROJECT_SOURCE_DIR}/*.cc")
add_executable(cyber_monitor ${CYBER_MONITOR_SRCS})
target_link_libraries(cyber_monitor cyber gflags glog pthread ncurses)

install(TARGETS cyber_monitor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/cyber/tools/cyber_monitor)
