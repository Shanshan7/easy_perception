project(registerer)

#include easy_ai directories
include_directories(${easy_ai_BINARY_DIR})
include_directories(${easy_ai_SOURCE_DIR})

#collect source files
file(GLOB REGISTER_SRCS 
    "${PROJECT_SOURCE_DIR}/*.cc"
	)

set(TEST_SRCS ${REGISTER_SRCS})
list(FILTER TEST_SRCS INCLUDE REGEX .*test[.]cc )
list(FILTER REGISTER_SRCS EXCLUDE REGEX .*test[.]cc )

#build registerer library
add_library(registerer SHARED ${REGISTER_SRCS})
target_link_libraries(registerer
    cyber
    gflags
    glog
)

#build tests
FOREACH(TEST_FILE ${TEST_SRCS})  
	string( REGEX MATCH "[A-Za-z0-9_]*[.]cc" FILE_NAME ${TEST_FILE} ) 
	string( REGEX MATCH "[A-Za-z0-9_]*" TARGET_NAME ${FILE_NAME})
	list(APPEND TEST_TARGETS ${TARGET_NAME})
	add_executable(${TARGET_NAME} ${TEST_FILE})
	target_link_libraries(${TARGET_NAME} registerer gtest gmock_main)
	target_compile_options(${TARGET_NAME} PRIVATE -Og)
ENDFOREACH(TEST_FILE)

install(DIRECTORY "${PROJECT_SOURCE_DIR}" DESTINATION include ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
install(FILES "${easy_ai_BINARY_DIR}/libregisterer.so" DESTINATION lib ${CMAKE_INSTALL_LIBDIR})