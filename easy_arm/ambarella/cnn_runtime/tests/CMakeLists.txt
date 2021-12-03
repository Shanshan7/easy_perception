PROJECT(tests)

#include amba_arm directories
include_directories(${amba_arm_BINARY_DIR})
include_directories(${amba_arm_SOURCE_DIR})

#collect source files
file(GLOB UTILITY_SRCS
    "${amba_arm_SOURCE_DIR}/utility/*.cpp"
)

add_executable(textnet_test textnet_test.cpp ${UTILITY_SRCS})
target_link_libraries(textnet_test
                      cnn_runtime 
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_highgui 
                      jpeg
                      png
                      z
                      nnctrl
                      vproc
                      cavalry_mem)

add_executable(classnet_test classnet_test.cpp ${UTILITY_SRCS})
target_link_libraries(classnet_test
                      cnn_runtime
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_highgui 
                      jpeg
                      png
                      z
                      nnctrl
                      vproc
                      cavalry_mem)

add_executable(denet_test denet_test.cpp ${UTILITY_SRCS})
target_link_libraries(denet_test
                      cnn_runtime 
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_highgui
                      jpeg
                      png
                      z
                      nnctrl
                      vproc
                      cavalry_mem)

add_executable(segnet_test segnet_test.cpp ${UTILITY_SRCS})
target_link_libraries(segnet_test
                      cnn_runtime
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_highgui 
                      jpeg
                      png
                      z
                      nnctrl
                      vproc
                      cavalry_mem)

add_executable(posenet_test posenet_test.cpp ${UTILITY_SRCS})
target_link_libraries(posenet_test
                      cnn_runtime 
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_highgui
                      opencv_flann
                      opencv_features2d
                      opencv_videoio
                      jpeg
                      png
                      z
                      nnctrl
                      vproc
                      cavalry_mem
                      utils
                      cvlib
                      PhaseData_20190919_v1.4f
                      IOdevice)

add_executable(one_class_test one_class_test.cpp ${UTILITY_SRCS})
target_link_libraries(one_class_test
                      cnn_runtime
                      opencv_core
                      opencv_imgcodecs
                      opencv_imgproc
                      opencv_highgui 
                      opencv_ml
                      jpeg
                      png
                      z
                      nnctrl
                      vproc
                      cavalry_mem)