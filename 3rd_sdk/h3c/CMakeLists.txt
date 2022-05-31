#1.cmake verson
cmake_minimum_required(VERSION 3.5.0)
#CONFIGURE_COMMAND ""
#BUILD_COMMAND ""

#2.project name
project(SSC_json)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE Release)

#add_definitions(-DIS_SHOW)
#add_definitions(-DIS_SEND_DATA)

#include MOT headers
include_directories(${PROJECT_SOURCE_DIR}/)
include_directories(${PROJECT_SOURCE_DIR}/NetSDK_V2)
include_directories(${PROJECT_SOURCE_DIR}/json/include)

#collect source files
file(GLOB H3C_PROCESS_SRCS
    "${PROJECT_SOURCE_DIR}/h3cprocess.cpp"
    "${PROJECT_SOURCE_DIR}/utils.cpp"
    "${PROJECT_SOURCE_DIR}/outputCsv.cpp"
    "${PROJECT_SOURCE_DIR}/json/src/*.cpp"
	)

#link cal_traj.so
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
     link_directories(${PROJECT_SOURCE_DIR}/NetSDK_V2/Lib/win64)
else(CMAKE_SYSTEM_NAME MATCHES "Linux")
    link_directories(${PROJECT_SOURCE_DIR}/NetSDK_V2/Lib/linux64)   
endif()
    
add_executable(test_web "")

target_sources(test_web
        PRIVATE
        "${PROJECT_SOURCE_DIR}/test_socket.cpp"
        ${H3C_PROCESS_SRCS}
        )

# link libraries
target_link_libraries(test_web 
                      idm_alarm
                      idm_live
                      idm_netsdk
                      idm_pdsp
                      idm_transfile
                      idm_vod
                      idm_voicetalk
                      libmysql
                      wsock32)