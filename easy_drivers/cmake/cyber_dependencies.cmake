
if(NOT DEFINED ENV{CYBER_PATH})
    message(FATAL_ERROR "not defined environment variable:CYBER_PATH")  
endif()
set(CYBER_DIR $ENV{CYBER_PATH})

set(Protobuf_PREFIX_PATH
    "${CYBER_DIR}/third_party/protobuf/include"
    "${CYBER_DIR}/third_party/protobuf/lib"
    "${CYBER_DIR}/third_party/protobuf/bin"
)
list(APPEND CMAKE_PREFIX_PATH "${Protobuf_PREFIX_PATH}")
find_package(Protobuf REQUIRED NOCONFIG)
message(STATUS "Protobuf version: ${Protobuf_VERSION}")
message(STATUS "Protobuf: ${Protobuf_INCLUDE_DIRS}, ${Protobuf_LIBRARIES}")
include_directories(${Protobuf_INCLUDE_DIRS})
link_directories(${Protobuf_LIBRARIES})

include_directories(${CYBER_DIR}/third_party/glog/include)
link_directories(${CYBER_DIR}/third_party/glog/lib)

include_directories(${CYBER_DIR}/third_party/gflags/include)
link_directories(${CYBER_DIR}/third_party/gflags/lib)

include_directories(${CYBER_DIR}/third_party/fastrtps/include)
link_directories(${CYBER_DIR}/third_party/fastrtps/lib)

#cyber
include_directories(${CYBER_DIR}/include)
link_directories(${CYBER_DIR}/lib)