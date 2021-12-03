project(ai_utility)

#include easy_ai directories
include_directories(${easy_ai_BINARY_DIR})
include_directories(${easy_ai_SOURCE_DIR})

add_subdirectory(registerer)

add_subdirectory(thread)