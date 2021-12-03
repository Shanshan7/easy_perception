project(thread)

#include easy_ai directories
include_directories(${easy_ai_BINARY_DIR})
include_directories(${easy_ai_SOURCE_DIR})

#collect source files
file(GLOB THREAD_SRCS 
    "${PROJECT_SOURCE_DIR}/*.cc"
	)

set(TEST_SRCS ${THREAD_SRCS})
list(FILTER TEST_SRCS INCLUDE REGEX .*test[.]cc )
list(FILTER THREAD_SRCS EXCLUDE REGEX .*test[.]cc )

#build thread library
add_library(thread SHARED ${THREAD_SRCS})
target_link_libraries(thread
    cyber
    gflags
    glog
	${Protobuf_LIBRARIES}
)

#build tests
FOREACH(TEST_FILE ${TEST_SRCS})  
	string( REGEX MATCH "[A-Za-z0-9_]*[.]cc" FILE_NAME ${TEST_FILE} ) 
	string( REGEX MATCH "[A-Za-z0-9_]*" TARGET_NAME ${FILE_NAME})
	list(APPEND TEST_TARGETS ${TARGET_NAME})
	add_executable(${TARGET_NAME} ${TEST_FILE})
	target_link_libraries(${TARGET_NAME} thread gtest gmock_main)
	target_compile_options(${TARGET_NAME} PRIVATE -Og)
ENDFOREACH(TEST_FILE)

install(DIRECTORY "${PROJECT_SOURCE_DIR}" DESTINATION include ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
install(FILES "${easy_ai_BINARY_DIR}/libthread.so" DESTINATION lib ${CMAKE_INSTALL_LIBDIR})