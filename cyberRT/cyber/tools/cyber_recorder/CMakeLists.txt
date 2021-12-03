project(cyber_recorder)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})

file(GLOB CYBER_RECORDER_SRCS 
    "${PROJECT_SOURCE_DIR}/*.cc" 
    "${PROJECT_SOURCE_DIR}/player/*.cc"
)
add_executable(cyber_recorder ${CYBER_RECORDER_SRCS})
target_link_libraries(cyber_recorder cyber gflags glog)
install(TARGETS cyber_recorder RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/cyber/tools/cyber_recorder)