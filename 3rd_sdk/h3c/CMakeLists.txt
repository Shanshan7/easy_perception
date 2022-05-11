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
#include_directories(${PROJECT_SOURCE_DIR}/)
include_directories(${PROJECT_SOURCE_DIR}/NetSDK_V2)
include_directories(${PROJECT_SOURCE_DIR}/json/include)

#collect source files
file(GLOB H3C_PROCESS_SRCS
    "${PROJECT_SOURCE_DIR}/h3cprocess.cpp"
    "${PROJECT_SOURCE_DIR}/json/src/*.cpp"
	)

#link cal_traj.so
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
     link_directories(${PROJECT_SOURCE_DIR}/NetSDK_V2/Lib/win64)
else(CMAKE_SYSTEM_NAME MATCHES "Linux")
    link_directories(${PROJECT_SOURCE_DIR}/NetSDK_V2/Lib/linux64)   
endif()


    
add_executable(h3c_process "")

target_sources(h3c_process
        PRIVATE
        "${PROJECT_SOURCE_DIR}/h3c_json.cpp"
        ${H3C_PROCESS_SRCS}
        )

# link libraries
target_link_libraries(h3c_process 
                      idm_alarm
                      idm_live
                      idm_netsdk
                      idm_pdsp
                      idm_transfile
                      idm_vod
                      idm_voicetalk)