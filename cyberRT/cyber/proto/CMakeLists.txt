project(cyber_proto)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})
message(STATUS "cyber_SOURCE_DIR: " ${cyberRT_project_SOURCE_DIR})
message(STATUS "cyber_BINARY_DIR: " ${cyberRT_project_BINARY_DIR})

file(GLOB CYBER_PROTO_SRCS *.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CYBER_PROTO_SRCS})

add_library(cyber_proto SHARED ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(cyber_proto ${Protobuf_LIBRARIES})
message(STATUS "protobuf_library: " ${Protobuf_LIBRARIES})

install(TARGETS cyber_proto LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
