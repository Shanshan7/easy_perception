project(cyber)
enable_language(ASM)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
	set(SWAP_SRCS "${PROJECT_SOURCE_DIR}/croutine/detail/swap_x86_64.S")
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
	set(SWAP_SRCS "${PROJECT_SOURCE_DIR}/croutine/detail/swap_aarch64.S")
else()
	message(FATAL_ERROR "Can not set swap source file, please check")
	return()
endif()

#collect source files
file(GLOB CYBER_SRCS 
	"${PROJECT_SOURCE_DIR}/base/*.cc"
	"${PROJECT_SOURCE_DIR}/blocker/*.cc"
	"${PROJECT_SOURCE_DIR}/class_loader/*.cc"
	"${PROJECT_SOURCE_DIR}/class_loader/*/*.cc"
	"${PROJECT_SOURCE_DIR}/common/*.cc" 
	"${PROJECT_SOURCE_DIR}/component/*.cc"
	"${PROJECT_SOURCE_DIR}/croutine/*.cc"
	"${PROJECT_SOURCE_DIR}/croutine/*/*.cc"
	"${SWAP_SRCS}"
	"${PROJECT_SOURCE_DIR}/data/*.cc"
	"${PROJECT_SOURCE_DIR}/event/*.cc"
	"${PROJECT_SOURCE_DIR}/io/*.cc"
	"${PROJECT_SOURCE_DIR}/logger/*.cc"
	"${PROJECT_SOURCE_DIR}/message/*.cc"
	"${PROJECT_SOURCE_DIR}/node/*.cc"
	"${PROJECT_SOURCE_DIR}/parameter/*.cc"
	"${PROJECT_SOURCE_DIR}/record/*.cc"
	"${PROJECT_SOURCE_DIR}/record/*/*.cc"
	"${PROJECT_SOURCE_DIR}/scheduler/*.cc"
	"${PROJECT_SOURCE_DIR}/scheduler/*/*.cc"
	"${PROJECT_SOURCE_DIR}/service/*.cc"
	"${PROJECT_SOURCE_DIR}/service_discovery/*.cc"
	"${PROJECT_SOURCE_DIR}/service_discovery/*/*.cc"
	"${PROJECT_SOURCE_DIR}/sysmo/*.cc"
	"${PROJECT_SOURCE_DIR}/task/*.cc" 
	"${PROJECT_SOURCE_DIR}/time/*.cc"
	"${PROJECT_SOURCE_DIR}/timer/*.cc"	
	"${PROJECT_SOURCE_DIR}/transport/*.cc"
	"${PROJECT_SOURCE_DIR}/transport/*/*.cc"
	"${PROJECT_SOURCE_DIR}/*.cc"
	)

set(CYBER_TEST_SRCS ${CYBER_SRCS})
list(FILTER CYBER_TEST_SRCS INCLUDE REGEX .*test[.]cc )
list(FILTER CYBER_SRCS EXCLUDE REGEX .*test[.]cc )

#build cyber library
add_library(cyber SHARED ${CYBER_SRCS})
target_link_libraries(cyber cyber_proto fastrtps fastcdr glog ${Poco_LIBRARIES} atomic uuid)

#build tests
FOREACH(TEST_FILE ${CYBER_TEST_SRCS})  
	string( REGEX MATCH "[A-Za-z0-9_]*[.]cc" FILE_NAME ${TEST_FILE} ) 
	string( REGEX MATCH "[A-Za-z0-9_]*" TARGET_NAME ${FILE_NAME})
	list(APPEND TEST_TARGETS ${TARGET_NAME})
	add_executable(${TARGET_NAME} ${TEST_FILE})
	target_link_libraries(${TARGET_NAME} cyber gflags glog gtest gmock_main)
	target_compile_options(${TARGET_NAME} PRIVATE -Og)
ENDFOREACH(TEST_FILE)

#install
install(TARGETS cyber LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS ${TEST_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/cyber/test)