project(cyber_tools)

#include cyberRT_project directories
include_directories(${cyberRT_project_BINARY_DIR})
include_directories(${cyberRT_project_SOURCE_DIR})

#build cyber_monitor
add_subdirectory(cyber_monitor)

#build cyber_recorder
add_subdirectory(cyber_recorder)

install(DIRECTORY cyber_launch DESTINATION ${CMAKE_INSTALL_BINDIR}/cyber/tools)
install(DIRECTORY cyber_channel DESTINATION ${CMAKE_INSTALL_BINDIR}/cyber/tools)
#install cyber_tools_auto_complete.bash
install(FILES cyber_tools_auto_complete.bash DESTINATION ${CMAKE_INSTALL_BINDIR}/cyber/tools)
