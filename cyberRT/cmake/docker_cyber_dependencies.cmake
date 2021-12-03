set(CyberRT_Depend_DIR /usr/local/cyberRT)

set(fastrtps_DIR ${CyberRT_Depend_DIR}/third_party/fastrtps/lib/fastrtps/cmake)
find_package(fastrtps REQUIRED CONFIG)
message(STATUS "Found fastrtps ${fastrtps_VERSION}")
message(STATUS "${fastrtps_LIB_DIR}, ${fastrtps_INCLUDE_DIR}")
include_directories(${fastrtps_INCLUDE_DIR})
link_directories(${fastrtps_LIB_DIR})

set(fastcdr_DIR ${CyberRT_Depend_DIR}/third_party/fastcdr/lib/fastcdr/cmake)
find_package(fastcdr REQUIRED CONFIG)
message(STATUS "Found fastcdr ${fastcdr_VERSION}")
message(STATUS "${fastcdr_LIB_DIR}, ${fastcdr_INCLUDE_DIR}")
include_directories(${fastcdr_INCLUDE_DIR})
link_directories(${fastcdr_LIB_DIR})

#set(Protobuf_ROOT ${CyberRT_Depend_DIR}/third_party/protobuf)
#set(Protobuf_DIR ${CyberRT_Depend_DIR}/third_party/protobuf/lib/cmake/protobuf)
#option(protobuf_VERBOSE "Enable for verbose output" ON)
#option(protobuf_MODULE_COMPATIBLE "CMake build-in FindProtobuf.cmake module compatible" ON)
set(Protobuf_PREFIX_PATH
    "${CyberRT_Depend_DIR}/third_party/protobuf/include"
    "${CyberRT_Depend_DIR}/third_party/protobuf/lib"
    "${CyberRT_Depend_DIR}/third_party/protobuf/bin"
)
list(APPEND CMAKE_PREFIX_PATH "${Protobuf_PREFIX_PATH}")
find_package(Protobuf REQUIRED NOCONFIG)
message(STATUS "version: ${Protobuf_VERSION}")
message(STATUS "Protobuf: ${Protobuf_INCLUDE_DIRS}, ${Protobuf_LIBRARIES}")
include_directories(${Protobuf_INCLUDE_DIRS})
link_directories(${Protobuf_LIBRARIES})

#set(tinyxml2_DIR /home/allen/aarch64-depends/tinyxml2/lib/cmake/tinyxml2)
#set(tinyxml2_ROOT /home/allen/aarch64-depends/tinyxml2/lib/cmake/tinyxml2)
#find_package(tinyxml2 REQUIRED CONFIG)
#message(STATUS "###tinyxml2: ${tinyxml2_LIBRARIES}")
#include_directories(/home/allen/aarch64-depends/tinyxml2/include)
#link_directories(/home/allen/aarch64-depends/tinyxml2/lib)

set(Poco_DIR ${CyberRT_Depend_DIR}/third_party/Poco/lib/cmake/Poco)
find_package(Poco REQUIRED COMPONENTS Foundation CONFIG)
message(STATUS "version: ${Poco_VERSION}")
message(STATUS "Found Poco: ${Poco_LIBRARIES}")

#set(gflags_DIR ${CyberRT_Depend_DIR}/third_party/gflags/lib/cmake/gflags)
#find_package(gflags REQUIRED CONFIG)
#message(STATUS "version: ${gflags_VERSION}")
#message(STATUS "Found gflags: ${gflags_LIBRARIES}")
include_directories(${CyberRT_Depend_DIR}/third_party/gflags/include)
link_directories(${CyberRT_Depend_DIR}/third_party/gflags/lib)


#set(glog_DIR ${CyberRT_Depend_DIR}/third_party/glog/lib/cmake/glog)
#find_package(glog REQUIRED CONFIG)
#message(STATUS "version: ${glog_VERSION}")
#message(STATUS "Found glog: ${glog_LIBRARIES}")
include_directories(${CyberRT_Depend_DIR}/third_party/glog/include)
link_directories(${CyberRT_Depend_DIR}/third_party/glog/lib)

include_directories(${CyberRT_Depend_DIR}/third_party/gtest/include)
link_directories(${CyberRT_Depend_DIR}/third_party/gtest/lib)
