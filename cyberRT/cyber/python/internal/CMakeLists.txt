project(cyber_py_wrapper)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})

find_package(Python3 COMPONENTS Development)
include_directories(${Python3_INCLUDE_DIRS})

#build _cyber_wrapper.so
add_library(_cyber_wrapper SHARED "py_cyber.cc")
target_link_libraries(_cyber_wrapper cyber)
install(CODE "file(RENAME ${cyberRT_project_BINARY_DIR}/lib_cyber_wrapper.so 
        ${cyberRT_project_BINARY_DIR}/_cyber_wrapper.so)")
install(FILES "${cyberRT_project_BINARY_DIR}/_cyber_wrapper.so" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cyber/python/internal)

#build _cyber_record_wrapper.so
add_library(_cyber_record_wrapper SHARED "py_record.cc")
target_link_libraries(_cyber_record_wrapper cyber)
install(CODE "file(RENAME ${cyberRT_project_BINARY_DIR}/lib_cyber_record_wrapper.so 
        ${cyberRT_project_BINARY_DIR}/_cyber_record_wrapper.so)")
install(FILES "${cyberRT_project_BINARY_DIR}/_cyber_record_wrapper.so" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cyber/python/internal)

#build _cyber_time_wrapper.so
add_library(_cyber_time_wrapper SHARED "py_time.cc")
target_link_libraries(_cyber_time_wrapper cyber)
install(CODE "file(RENAME ${cyberRT_project_BINARY_DIR}/lib_cyber_time_wrapper.so 
        ${cyberRT_project_BINARY_DIR}/_cyber_time_wrapper.so)")
install(FILES "${cyberRT_project_BINARY_DIR}/_cyber_time_wrapper.so" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cyber/python/internal)

#build _cyber_timer_wrapper.so
add_library(_cyber_timer_wrapper SHARED "py_timer.cc")
target_link_libraries(_cyber_timer_wrapper cyber)
install(CODE "file(RENAME ${cyberRT_project_BINARY_DIR}/lib_cyber_timer_wrapper.so 
        ${cyberRT_project_BINARY_DIR}/_cyber_timer_wrapper.so)")
install(FILES "${cyberRT_project_BINARY_DIR}/_cyber_timer_wrapper.so" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cyber/python/internal)

#build _cyber_parameter_wrapper.so
add_library(_cyber_parameter_wrapper SHARED "py_parameter.cc")
target_link_libraries(_cyber_parameter_wrapper cyber)
install(CODE "file(RENAME ${cyberRT_project_BINARY_DIR}/lib_cyber_parameter_wrapper.so 
        ${cyberRT_project_BINARY_DIR}/_cyber_parameter_wrapper.so)")
install(FILES "${cyberRT_project_BINARY_DIR}/_cyber_parameter_wrapper.so" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cyber/python/internal)


list(APPEND TEST_TARGETS py_cyber_test)
add_executable(py_cyber_test py_cyber_test.cc)
target_link_libraries(py_cyber_test _cyber_wrapper cyber gflags glog gtest gmock_main)
target_compile_options(py_cyber_test PRIVATE -Og)

list(APPEND TEST_TARGETS py_record_test)
add_executable(py_record_test py_record_test.cc)
target_link_libraries(py_record_test _cyber_record_wrapper cyber gflags glog gtest gmock_main)
target_compile_options(py_record_test PRIVATE -Og)

install(TARGETS ${TEST_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/cyber/python)