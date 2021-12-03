PROJECT(cnn_runtime)

#include amba_arm directories
include_directories(${amba_arm_BINARY_DIR})
include_directories(${amba_arm_SOURCE_DIR})

#collect source files
file(GLOB CNN_RUNTIME_SRCS 
    "${PROJECT_SOURCE_DIR}/cnn_common/*.cpp"
	"${PROJECT_SOURCE_DIR}/classify/*.cpp"
	"${PROJECT_SOURCE_DIR}/det2d/*.cpp"
	"${PROJECT_SOURCE_DIR}/pose/*.cpp"
	"${PROJECT_SOURCE_DIR}/segment/*.cpp"
	"${PROJECT_SOURCE_DIR}/rec_text/*.cpp"
    "${PROJECT_SOURCE_DIR}/one_class/*.cpp"
	)

#build cnn_runtime library
add_library(cnn_runtime SHARED ${CNN_RUNTIME_SRCS})
target_link_libraries(cnn_runtime
    opencv_core
    opencv_imgcodecs
    opencv_imgproc
    opencv_highgui 
    opencv_ml
    jpeg
    png
    z
    nnctrl
    cavalry_mem
    vproc          
)

install(FILES cnn_common/cnn_data_structure.h DESTINATION include/cnn_runtime/cnn_common ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES classify/classnet.h DESTINATION include/cnn_runtime/classify ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES det2d/denet.h DESTINATION include/cnn_runtime/det2d ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES segment/segnet.h DESTINATION include/cnn_runtime/segment ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES pose/posenet.h DESTINATION include/cnn_runtime/pose ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES rec_text/textnet.h DESTINATION include/cnn_runtime/rec_text ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES one_class/one_class_net.h DESTINATION include/cnn_runtime/one_class ${CMAKE_INSTALL_INCLUDEDIR})

#install(DIRECTORY "${amba_arm_SOURCE_DIR}/cnn_runtime/classify" DESTINATION include/cnn_runtime ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY "${amba_arm_SOURCE_DIR}/cnn_runtime/det2d" DESTINATION include/cnn_runtime ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY "${amba_arm_SOURCE_DIR}/cnn_runtime/segment" DESTINATION include/cnn_runtime ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY "${amba_arm_SOURCE_DIR}/cnn_runtime/pose" DESTINATION include/cnn_runtime ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY "${amba_arm_SOURCE_DIR}/cnn_runtime/rec_text" DESTINATION include/cnn_runtime ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")

install(FILES "${amba_arm_BINARY_DIR}/libcnn_runtime.so" DESTINATION lib ${CMAKE_INSTALL_LIBDIR})
